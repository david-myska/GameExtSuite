#include "ge_test.h"

#include <set>
#include <utility>

#include "game_enhancer/achis/achievement.h"
#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"
#include "pma/logging/console_logger.h"

namespace Raw
{

#pragma pack(push, 1)

    struct Stat
    {
        // clang-format off
                                // sizeof = 0x8
    uint32_t m_statId = {};
    int32_t m_value      = {};
        // clang-format on
    };

    struct Stats
    {
        // clang-format off
                                // sizeof = 0x8
    Stat* m_pStats     = {};
    uint16_t m_count      = {};
    uint16_t m_sizeInBits = {};
        // clang-format on
    };

    struct StatList
    {
        // clang-format off
                                            //  sizeof = 0x40
    uint32_t m_pMemPool    = {};            //  +000 - pMemPool - always NULL, as for other structures
    uint32_t m_pUnit       = {};            //  +004 - pUnit - the unit to which the list is attached
    uint32_t m_ownerType   = {};            //  +008 - eOwnerType - the unit type of whatever unit created the statlist (spell caster etc)
    uint32_t m_ownerGUID   = {};            //  +00C - OwnerGUID - the global unique identifier of whatever unit created the statlist
    uint32_t m_listFlags   = {};            //  +010 - ListFlags (32 flags)
    uint32_t m_stateNo     = {};            //  +014 - stateNo - Id of the state this statlist is linked to (for buffs, curses etc)
    uint32_t m_expireFrame = {};            //  +018 - ExpireFrame - the frame at which the state expires (end of duration for skill)
    uint32_t m_skillNo     = {};            //  +01C - skillNo - Id of the skill that created the stat list
    uint32_t m_skillLvl    = {};            //  +020 - sLvl - level of the skill that created the stat list
    Stats m_stats          = {};            //  +024 - Stats structure (inline)
                                            //    +000 - pStat[arrSize] - dynamic array of stat structures
                                            //      +000 - hiStatId (param)
                                            //      +002 - loStatId (statNo from ItemStatCost.txt)
                                            //      +004 - value
                                            //    +004 - statCount (size of the array)
                                            //    +008 - sizeInBits
    uint32_t m_pPrevList              = {}; //  +02C - pPrevList - previous list on this unit
    uint32_t m_pNextList              = {}; //  +030 - pNextList - next list on this unit
    uint32_t m_pPrevious              = {}; //  +034 - pPrevious - previous list overall
    uint32_t m_functionExpireCallback = {}; //  +038 - fpStatExpires(); - function to call when the list is removed (void * __fastcall)(pUnit,stateNo,BOOL);
    uint32_t m_pNext                  = {}; //  +03C - pNext - next list overall
                                // clang-format on
    };

    struct StatListEx
    {
        // clang-format off
                                      //  sizeof = 0x64
    uint8_t m_pMemPool[8]       = {}; //  +000 - pMemPool - always NULL
    uint32_t m_ownerType        = {}; //  +008 - eOwnerType
    uint32_t m_ownerGUID        = {}; //  +00C - OwnerGUID
    uint8_t m_listFlags[20]     = {}; //  +010 - ListFlags
    Stats m_baseStats           = {}; //  +024 - BaseStats structure (inline, see under pStatList for details)
    uint8_t m_unknown1[4]       = {};
    uint32_t m_pLastList        = {}; //  +02C - pLastList - pointer to the last pStatList of the StatListEx owner (aka item owner in case list ex belongs to item)
    uint32_t m_pLastListEx      = {}; //  +034 - pStatListEx - pointer to owner StatListEx (if this one is owned by a item, this points to the item owners list)
    uint32_t m_pNextListEx      = {}; //  +038 - pNextListEx - next StatListEx
    uint32_t m_pMyLastList      = {}; //  +03C - pMyLastList (statlist)
    uint32_t m_pMyStats         = {}; //  +040 - pMyStats (statlist)
    uint32_t m_pOwner           = {}; //  +044 - pUnit (list owner)
    Stats m_fullStats           = {}; //  +048 - FullStats (inline stats struct, see below)
    Stats m_modStats            = {}; //  +050 - ModStats (inline stats struct, see below)
    uint32_t m_pStatFlags       = {}; //  +058 - StatFlags[] (pointer to array)
    uint32_t m_functionCallback = {}; //  +05C - fCallback (function to call by SetStat, AddStat when a fcallback stat changes)
    uint32_t m_pGame            = {}; //  +060 - pGame (on server)
                                // clang-format on
    };

#pragma pack(pop)

#pragma pack(push, 1)

    struct Inventory
    {
        // clang-format off
                                        //  sizeof = 0x40
    uint32_t m_invStamp           = {}; //  +000 - dwInvStamp - always, 0x1020304, used to verify the inventory is valid
    uint32_t m_pMemPool           = {}; //  +004 - pMemPool - always NULL, not used
    uint32_t m_pOwnerUnit         = {}; //  +008 - pOwnerUnit - pUnit of the inventory owner
    uint32_t m_pFirstItem         = {}; //  +00C - pFirstItem - pUnit of the first item in the inventory
    uint32_t m_pInvInfo           = {}; //  +014 - pInvInfo - list of pointers to equipped gear
    uint32_t m_invInfoCount       = {}; //  +018 - nInvInfo - count for above
    uint32_t m_weaponGUID         = {}; //  +01C - WeaponGUID
    uint32_t m_pInvOwnerItem      = {}; //  +020 - pInvOwnerItem - points to self on items that aren't placed into sockets
    uint32_t m_ownerGUID          = {}; //  +024 - OwnerGUID - GUID of the inventory owner
    uint32_t m_filledSocketsCount = {}; //  +028 - nFilledSockets
    uint32_t m_pFirstCorpse       = {}; //  +034 - pFirstCorpse - ptr to first corpse structure
    uint32_t m_nextCorpseGUID     = {}; //  +03C - NextCorpseGUID
                                         // clang-format on
    };

    struct StaticPath
    {
        // clang-format off
                                      //  sizeof = 0x20
    uint32_t m_outerWorld      = {};  //  +000 - is non-zero when dropped
    uint16_t m_unknown2        = {};  //  +004 - is set when dropped
    uint16_t m_unknown3        = {};  //  +006 - is set when dropped, always seems to be -1
    uint16_t m_unknown4        = {};  //  +008 - is set when dropped
    uint16_t m_unknown5        = {};  //  +00A - is set when dropped, always seems to be 1
    uint16_t m_xPos            = {};  //  +00C - x position in the world/in the inventory/when equipped
    uint16_t m_unknown6        = {};  //  +00E - always zero
    uint16_t m_yPos            = {};  //  +010 - y position in the world/in the inventory/(when equipped is 0)
    uint16_t m_unknown7        = {};  //  +012 - always zero
    uint8_t  m_unknown8[12]    = {};  //  +014
                                      // clang-format on
    };

    struct DynamicPath
    {
        // clang-format off
                                      //  sizeof = 220
    uint16_t m_unknown1        = {};  //  +000
    uint16_t m_xPos            = {};  //  +002 - current X position
    uint16_t m_unknown2        = {};  //  +004
    uint16_t m_yPos            = {};  //  +006 - current Y position
    uint16_t m_unknown3        = {};  //  +008
    uint16_t m_unknown4        = {};  //  +00A - always seems to be -1
    uint16_t m_unknown5        = {};  //  +00C
    uint16_t m_unknown6        = {};  //  +00E - always seems to be 1
    uint16_t m_xTargetPos      = {};  //  +010 - final destination of the unit on the x-axis
    uint16_t m_yTargetPos      = {};  //  +012 - final destination of the unit on the y-axis
    uint16_t m_xTargetPos2     = {};  //  +014 - same
    uint16_t m_yTargetPos2     = {};  //  +016 - same
    uint16_t m_xTargetPos3     = {};  //  +018 - same
    uint16_t m_yTargetPos3     = {};  //  +01A - same
    uint16_t m_unknown7        = {};  //  +01C
    uint16_t m_unknown8        = {};  //  +01E
    uint32_t m_unknown9        = {};  //  +020
    uint32_t m_unknown10       = {};  //  +024
    uint16_t m_activePathNodes = {};  //  +028 - when colliding more are active to find a way, when
                                      //  moving without collision, only 1 is active
    uint8_t m_unknown11[114]    = {}; //  +02A
    uint16_t m_pathNodes[16][2] = {}; //  +09C - pathNode is struct of {xPos, yPos}
                                           // clang-format on
    };

    struct ItemData
    {
        using Path = StaticPath;
        // clang-format off
                                         //  sizeof = 0x74
    uint32_t m_quality = {};             //  +000 - qualityNo;
                                         //      0 - QUALITY_INVALID
                                         //      1 - QUALITY_LOW
                                         //      2 - QUALITY_NORMem
                                         //      3 - QUALITY_SUPERIOR
                                         //      4 - QUALITY_MAGIC
                                         //      5 - QUALITY_SET
                                         //      6 - QUALITY_RARE
                                         //      7 - QUALITY_UNIQUE
                                         //      8 - QUALITY_CRAFTED
                                         //      9 - QUALITY_TAMPERED
    uint32_t m_seed[2] = {};             //  +004 - ItemSeed
                                         //     +000 - LoSeed
                                         //     +004 - HiSeed
    uint32_t m_ownerGUID    = {};        //  +00C - OwnerGUID (-1 when not owned by player, otherwise equal to player GUID [IIRC])
    uint32_t m_spawnSeed    = {};        //  +010 - FingerPrint - this is the initial spawning seed
    uint32_t m_commandFlags = {};        //  +014 - CommandFlags - I've not seen how this is used myself yet
    uint32_t m_itemFlags[3] = {};        //  +018 - ItemFlags - for more flags look at the original topic by Kingpin
                                         //     0x00000010 - ITEMFLAG_IDENTIFIED
                                         //     0x00000800 - ITEMFLAG_SOCKETED
                                         //     0x00008000 - ITEMFLAG_NAMED (for ears, personalized items etc)
                                         //     0x00020000 - ITEMFLAG_INEXPENSIVE (always costs 1 for repair / sell)
                                         //     0x00200000 - ITEMFLAG_COMPACTSAVE
                                         //     0x00400000 - ITEMFLAG_ETHEREAL
                                         //     0x04000000 - ITEMFLAG_RUNEWORD
    uint32_t m_actionStamp = {};         //  +024 - ActionStamp - seams to be changed every time an action is done with the item
    uint32_t m_fileIndex   = {};         //  +028 - FileIndex - index from data files
                                         //      UniqueItems.txt, SetItems.txt, QualityItems.txt, LowQualityItems.txt (etc)
    uint32_t m_itemLvl        = {};      //  +02C - iLvl
    uint16_t m_itemFormat     = {};      //  +030 - ItemFormat - read from pGame -> ItemFormat (word) on creation
    uint16_t m_rarePrefix     = {};      //  +032 - RarePrefix (word)
    uint16_t m_rareSuffix     = {};      //  +034 - RareSuffix (word)
    uint16_t m_autoPrefix     = {};      //  +036 - AutoPrefix (word)
    uint16_t m_magicPrefix[3] = {};      //  +038 - MagicPrefix[3] (words)
    uint16_t m_magicSuffix[3] = {};      //  +03E - MagicSuffix[3] (words)
    uint8_t m_bodyLoc         = {};      //  +044 - BodyLoc (byte) - Id from BodyLocs.txt, note this field isn't always cleared, use D2Common.#11003 instead of checking this
    uint8_t m_invPage         = {};      //  +045 - InvPage (byte) - set to -1 when equipped
                                         //      0 = INVPAGE_INVENTORY
                                         //      3 = INVPAGE_HORADRIC_CUBE
                                         //      4 = INVPAGE_STASH
    uint8_t m_earLvl               = {}; //  +048 - EarLevel (byte)
    uint8_t m_invGfxIdx            = {}; //  +049 - InvGfxIdx (byte) - for itemtypes with VarInvGfx
    uint16_t m_playerName[16]      = {}; //  +04A - szPlayerName[16] - used for Ears and Personalized items
    uint32_t m_pNodeOwnerInventory = {}; //  +05C - pNodeOwnerInventory - for socketed items this points to the inventory of the parent item
    uint32_t m_pNextSocketedItem   = {}; //  +064 - pNextSocketedItem - item filling the next socket, if pNodeOwnerInventory is set
    uint8_t m_nodePosition         = {}; //  +068 - nNodePosition
    uint8_t m_nodePosition2        = {}; //  +069 - nNodePositionOther
    uint8_t m_unknown[4]           = {};
        // clang-format on
    };

    struct MonsterData
    {
        using Path = DynamicPath;
        // clang-format off
                                        //  sizeof = 0x60
    uint32_t m_pMonStats     = {};      //  +000 - pMonStats - record in monstats.txt
    uint8_t m_components[16] = {};      //  +004 - Components[16] - bytes holding the component Ids for each component;
                                        //      Order: HD, TR, LG, RA, LA, RH, LH, SH, S1, S2, S3, S4, S5, S6, S7, S8
    uint16_t m_nameSeed = {};           //  +014 - NameSeed
    uint8_t m_typeFlags = {};           //  +016 - TypeFlags
                                        //  	0x00000001 - MONTYPE_OTHER (set for some champs, uniques)
                                        //      0x00000002 - MONTYPE_SUPERUNIQUE
                                        //      0x00000004 - MONTYPE_CHAMPION
                                        //      0x00000008 - MONTYPE_UNIQUE
                                        //      0x00000010 - MONTYPE_MINION
                                        //      0x00000020 - MONTYPE_POSSESSED
                                        //      0x00000040 - MONTYPE_GHOSTLY
                                        //      0x00000080 - MONTYPE_MULTISHOT
    uint8_t m_lastMode            = {}; //  +017 - eLastMode
    uint32_t m_duriel             = {}; //  +018 - dwDuriel - set only for duriel
    uint8_t m_monUModList[9]      = {}; //  +01C - MonUModList[9] - nine bytes holding the Ids for each MonUMod assigned to the unit
    uint8_t m_unknown1[1]         = {};
    uint16_t m_bossNo             = {}; //  +026 - bossNo - hcIdx from superuniques.txt for superuniques (word)
    uint32_t m_pAiGeneral         = {}; //  +028 - pAiGeneral
    uint32_t m_pMonNameOrAiParams = {}; //  - server side -
                                        //      +02C - pAiParams
                                        //  - client side -
                                        //      +02C - szMonName (ptr to wchar_t string, 300 chars long)
    uint8_t m_someAI[16]    = {};       //  +030 - this holds a third monster ai structure I didn't analyse yet
    uint16_t m_pet          = {};       //  +040 - dwNecroPet - set for necro pets
    uint8_t m_unknown2[14]  = {};
    uint32_t m_pVision      = {}; //  +050 - pVision - this may be polymorphic, the way this is used seams to depend on the monster type, used in LOS evaluation
    uint32_t m_aiState      = {}; //  +054 - AiState - this is used to tell monsters what special state has been set, this tells them they just got attacked etc
    uint32_t m_regionNo     = {}; //  +058 - lvlNo - the Id from levels.txt of the level they got spawned in (used to access pGame -> pMonsterRegion[...])
    uint8_t m_summonerFlags = {}; //  +05C - SummonerFlags - byte used only by the summoner
    uint8_t m_unknown3[3]   = {};
        // clang-format on
    };

    struct PlayerData
    {
        using Path = DynamicPath;
        // Not well explored/documented
        // clang-format off
    uint8_t m_name[0x10]                = {}; //+00	Player Name
    uint32_t m_pQuest[3]              = {}; //+10	Quest Pointers for each difficulty
    uint8_t m_unknown1[0x18]          = {}; //+1C		//before : 0x14
    uint32_t m_pArenaUnit             = {}; //+34	ptArena for the Unit
    uint8_t m_unknown2[0x4]           = {}; //+38		//before : 0x7
    uint16_t m_MPSourcePortalUniqueID = {}; //+3C	Source Portal Unique_ID
    uint8_t m_unknown3[0x2]           = {}; //+3E
    uint16_t m_MPDestPortalUniqueID   = {}; //+40	Destination Portal Unique_ID
    uint8_t m_unknown4[0x06]          = {}; //+42
    uint8_t m_townPortalId            = {}; //+48	Object UniqueID for TownPortals
    uint8_t m_unknown5[0x53]          = {}; //+49
    uint32_t m_pNetClient             = {}; //+9C	ptClient
                                     // clang-format on
    };

    struct NoData
    {
        using Path = void;
    };

    template <typename UnitType = NoData>
    struct UnitData
    {
        // clang-format off
                                                    //  sizeof = 0xF4
    uint32_t m_unitType = {};                       //  +000 - eType
                                                    //          0 = PLAYER
                                                    //          1 = MONSTER
                                                    //          2 = OBJECT
                                                    //          3 = MISSILE
                                                    //          4 = ITEM
                                                    //          5 = VIS_TILE
    uint32_t m_unitClass = {};                      //  +004 - eClass
                                                    //          unitclass,thehcIdxfromthesefiles
                                                    //          PlrClass.txt
                                                    //          MonStats.txt
                                                    //          Objects.txt
                                                    //          Missiles.tx
                                                    //          Items.txt(Weapons+Armor+Misc)
                                                    //          LvlWarps.txt
    uint32_t m_pMemPool = {};                       //  +008 - pMemPool - not used, always NULL (pGame+1C is always NULL)
    uint32_t m_GUID     = {};                       //  +00C - GUID - global unique identifier that links a server side unit to a client side unit
    uint32_t m_unitMode = {};                       //  +010 - eMode
                                                    //          modeindexfrom
                                                    //          PlrMode.txt
                                                    //          MonMode.txt
                                                    //          ObjMode.txt
                                                    //          Missiles, Items and VisTiles have their own modes (for missiles this holds the collision type)
                                                    //          eItemModes
                                                    //            3=onthefloor
    UnitType* m_pUnitData = {};                      //  +014 - pUnitData (union of 5 classes)
                                                    //            pPlayerData
                                                    //            pMonsterData
                                                    //            pObjectData
                                                    //            pMissileData
                                                    //            pItemData
                                                    //            Note, there is no pTileData
    uint32_t m_actNo    = {};                       //  +018 - actNo (byte)
    uint32_t m_pDrlgAct = {};                       //  +01C - pDrlgAct
    uint32_t m_seed[2]  = {};                       //  +020 - seed (inline structure)
                                                    //    +000 - loSeed
                                                    //    +004 - hiSeed
    uint32_t m_initialSeed = {};                    //  +028 - dwInitSeed
    UnitType::Path* m_pPath       = {};                    //  +02C - pPath (union of 2 classes)
                                                    //            pStaticPath (Objects, VisTiles, Items)
                                                    //            pDynamicPath (Players, Monsters, Missiles)
    uint32_t m_pSkillSeq                   = {};    //  +030 - pSeqMode (holds a pointer to skill sequence)
    uint32_t m_skillSeqFrameCount          = {};    //  +034 - nSeqFrameCount (frame * 256, count for sequence)
    uint32_t m_remainingSkillSeqFrameCount = {};    //  +038 - nSeqFrame (frame * 256 remaining for sequence)
    uint32_t m_animSpeed                   = {};    //  +03C - AnimSpeed (32Bit)
    uint32_t m_animSeqMode                 = {};    //  +040 - eSeqMode (holds current anim mode of sequence)
    uint32_t m_animCurrentFrame            = {};    //  +044 - CurrentFrame (frame * 256 remaining for animation)
    uint32_t m_animFrameCount              = {};    //  +048 - FrameCount (frame * 256)
    uint16_t m_animSpeedShort              = {};    //  +04C - AnimSpeed (16Bit) (word)
    uint16_t m_actionFrame                 = {};    //  +04E - bActionFrame (1 byte) - exact purpose not known to me
    uint32_t m_pAnimData                   = {};    //  +050 - pAnimData - ptr to the record for current *.COF file in AnimData.d2
    uint32_t m_pGfxData                    = {};    //  +054 - pGfxData
                                                    //    +030 - pGfxDrawOffsets
                                                    //      +00 - DrawFlags
                                                    //      +04 - x_pos
                                                    //      +08 - y_pos
                                                    //      +0C - z_pos
                                                    //      +10 - x_offset
                                                    //      +14 - y_offset
                                                    //      +18 - z_offset
                                                    //      +1C - x_offset_2
                                                    //      +20 - y_offset_2
                                                    //      +24 - z_offset_2
                                                    //      +28 - x_offset_3
                                                    //      +2C - y_offset_3
                                                    //      +30 - z_offset_3
                                                    //    +038 - nPalShiftIndex
    uint32_t m_pGfxData2                      = {}; //  +058 - pGfxData (another copy of pGfxData - didn't check what the second is used for)
    StatListEx* m_pStatListEx                    = {}; //  +05C - pStatListEx
    Inventory* m_pInventory                     = {}; //  +060 - pInventory
    uint32_t m_pLightMapOrInteractGUID        = {};
    uint32_t m_startLightRadiusOrInteractType = {};
    uint16_t m_pl2ShiftIndexOrIsInteracting   = {};
    //  - serverside -
    //      +064 - InteractGUID - global unique identifier of the other unit participating in interaction
    //      +068 - eInteractType - the unit type of the other unit participating in interaction
    //      +06C - bInteracting - boolean set to true when interatcing
    //  - clientside -
    //      +064 - pLightMap
    //      +068 - dwStartLightRadius
    //      +06C - nPl2ShiftIndex
    uint16_t m_updateType         = {}; //  +06E - UpdateType (word)
    uint32_t m_pUpdateUnit        = {}; //  +070 - pUpdateUnit - this is a pointer to self, used when updating the unit
    uint32_t m_pQuestRecord       = {}; //  +074 - pQuestRecord - for quest monsters (etc)
    uint32_t m_sparkyChest        = {}; //  +078 - bSparkyChest - boolean used only for sparky chests, to get the extra drop (byte)
    uint32_t m_pTimerArgs         = {}; //  +07C - pTimerArgs - arguments to pass to timer
    uint32_t m_soundSyncOrGamePtr = {};
    //  - serverside -
    //      +080 - pGame
    //  - clientside -
    //      +080 - dwSoundSync - used by summons and ambient stuff
    uint8_t m_unknown1[12]                = {};
    uint32_t m_pEvent                     = {}; //  +090 - pEvent - this is a queue of events to execute (chance to cast skills for example)
    uint32_t m_ownerType                  = {}; //  +094 - eOwnerType - unit type of missile or minion owner (also used by portals)
    uint32_t m_ownerGUID                  = {}; //  +098 - OwnerGUID - global unique identifier of minion or missile owner (also used by portals)
    uint8_t unknown2[8]                   = {};
    uint32_t m_pHoverTextControl          = {}; //  +0A4 - pHoverText - hovering text controller (such as the shrine message)
    uint32_t m_pSkillsControl             = {}; //  +0A8 - pSkills - controller holding a list of all skills the unit has (pointers to pSkill)
    uint32_t m_pCombat                    = {}; //  +0AC - pCombat - a queue of attacks to execute
    uint32_t m_lastHitClass               = {}; //  +0B0 - dwHitClass - the hitclass for the last hit suffered
    uint8_t m_unknown2[4]                 = {};
    uint32_t m_dropCode                   = {}; //  +0B8 - DropCode - used only by book shelves in vanilla IIRC, item code of what to drop
    uint8_t m_unknown3[8]                 = {};
    uint8_t m_unitFlags[12]               = {}; //  +0C4 - UnitFlags - UnitFlags (qword)
    uint32_t m_nodeIndex                  = {}; //  +0D0 - Node Index - originally said to be client Id, but this is used together with pGame -> pNodeList IIRC
    uint32_t m_tickCount                  = {}; //  +0D4 - GetTickCount - used only by client units for overlays
    uint32_t m_pParticleStreamOrTickCount = {};
    //  - serverside -
    //      +0D8 - GetTickCount - used only by doors
    //  - clientside -
    //      +0D8 - pParticleStream
    uint32_t m_pTimer          = {}; //  +0DC - pTimer - a queue of timers assigned to this unit
    uint8_t m_unknown4[4]     = {};
    UnitData<UnitType>* m_pPrevUnit       = {}; //  +0E4 - pPrevUnit - previous unit in the unit-type list (the last unit is linked to pGame -> pUnitList[eType][GUID&127]
    uint32_t m_pPrevUnitInRoom = {}; //  +0E8 - pPrevUnitInRoom - the previous unit in the current room
    uint32_t m_pMsgFirst       = {}; //  +0EC - pMsgFirst
    uint32_t m_pMsgLast        = {}; //  +0F0 - pMsgLast
                                   // clang-format on
    };

    struct Game
    {
        // clang-format off
                                          //  sizeof = 0x1DF4
    uint8_t m_unknown1[0x18]        = {};
    uint32_t m_pCriticalSection     = {}; //  +0018 - pCriticalSection (see MSDN please)
    uint8_t m_memPool[0x6A - 0x1C]  = {}; //  +001C - pMemPool - not used, always NULL
    uint8_t m_gameType1             = {}; //  +006A - GameType - whenever this is single player (etc)
    uint8_t m_unknown2[0x6D - 0x6B] = {};
    uint8_t m_difficultyLevel       = {}; //  +006D - DifficultyLevel -> 0 = NORMAL, 1 = NIGHTMARE, 2 = HELL
    uint8_t m_unknown3[0x70 - 0x6E] = {};
    uint32_t m_expansion            = {}; //  +0070 - bExpansion -> 0 = CLASSIC, 1 = EXPANSION
    uint32_t m_gameType2            = {}; //  +0074 - GameType - similar to +06A, this seams to be more related to ladder vs. non ladder, (single player is ladder)
    uint32_t m_itemFormat           = {}; //  +0078 - ItemFormat (word) -> 0 = LEGACY, 1 = CLASSIC, 101 = EXPANSION
    uint32_t m_initSeed1            = {}; //  +007C - InitSeed
    uint32_t m_objSeed              = {}; //  +0080 - ObjSeed - seed used for object spawning
    uint32_t m_initSeed2            = {}; //  +0084 - InitSeed (another instance, dunno why)
    uint32_t m_pClientList          = {}; //  +0088 - pClientList - (pClient structure of last player that entered the game)
    uint32_t m_clientCount          = {}; //  +008C - nClients
    uint32_t m_unitCounters[6]      = {}; //  +0090 - nUnits[eType] - array of 6 counters, one for each unit type, this is the next GUID used too
    uint32_t m_gameFrame            = {}; //  +00A8 - GameFrame - the current frame of the game, used for timers (buff duration etc)
    uint8_t m_unknown4[0xB8 - 0xAC] = {};
    uint32_t m_pTimerQueue          = {};  //  +00B8 - pTimerQueue - a queue of all current active and inactive timers
    uint32_t m_pDrlgAct[5]          = {};  //  +00BC - pDrlgAct[5]
    uint32_t m_gameSeed[2]          = {};  //  +00D0 - GameSeed (inlined structure) -> +000 - loSeed, +004 - hiSeed
    uint32_t m_pDrlgRoomList[5]     = {};  //  +00D8 - pDrlgRoomList[5]
    uint32_t m_monSeed              = {};  //  +00EC - MonSeed - seed used for monster spawning
    uint32_t m_pMonsterRegion[1024] = {};  //  +00F0 - pMonsterRegion[1024] - one pointer for each of the 1024 possible levels
    uint32_t m_pObjectControl       = {};  //  +10F0 - pObjectControl - a controller holding all object region structs
    uint32_t m_questControl         = {};  //  +10F4 - pQuestControl - a controller holding all quest info
    uint32_t m_pUnitNodes[10]       = {};  //  +10F8 - pUnitNodes[10] - ten lists of unit node lists, this is used by the AI target seeking code (and other stuff)
    UnitData<PlayerData>* m_pPlayerList[128]    = {};  //  +1120 - pUnitList[eType][128] - 5 lists of 128 lists of units (see pUnit documentation)
    UnitData<MonsterData>* m_pMonsterList[128]    = {};
    UnitData<>* m_pObjectList[128]    = {};
    UnitData<ItemData>* m_pItemList[128]    = {};
    UnitData<>* m_pMissileList[128]    = {};
    UnitData<>* m_pTileList[128]    = {};
                                           //  -> second index is GUID & 127, BEWARE: since ever, missiles are array #4 and items are array #3 (so type3=index4 and type4=index3)
    uint32_t m_uniqueFlags[128] = {};      //  +1B24 - UniqueFlags[128] - 128 DWORDS worth of flags that control if a unique item got spawned [room for 4096]
    uint32_t m_pNpcControl      = {};      //  +1D24 - pNpcControl - a controller holding all npc info (like store inventories, merc list)
    uint32_t m_pArenaControl    = {};      //  +1D28 - pArenaControl - a controller for arena stuff, functional and also used in game init
                                           //  -> +00 - nAlternateStartTown, +08 - dwGameType (when type = 2, use nAlternateStartTown instead of norMem town level Id for game start)
    uint32_t m_pPartyControl         = {}; //  +1D2C - pPartyControl - a controller for all party related stuff
    uint8_t m_bossFlags[64]          = {}; //  +1D30 - BossFlags[64] - 64 bytes for handling 512 super unique monsters (if they were spawned etc)
    uint32_t m_monModeData[17]       = {}; //  +1D70 - MonModeData[17] - related to monsters changing mode
    uint32_t m_counterMonModeData[4] = {}; //  +1DB4 - nMonModeData - counter related to the above
    uint32_t m_syncTimers[9]         = {}; //  +1DC4 - nSyncTimer - used to sync events
    uint32_t m_bUberBaal             = {}; //  +1DE8 - bUberBaal - killed uber baal
    uint32_t m_bUberDiablo           = {}; //  +1DEC - bUberDiablo - killed uber diablo
    uint32_t m_bUberMephisto         = {}; //  +1DF0 - bUberMephisto - killed uber mephisto
                                        // clang-format on
    };

#pragma pack(pop)

}  // namespace Raw

namespace D2Data
{
    enum class Difficulty
    {
        Normal,
        Nightmare,
        Hell
    };

    enum class GameType
    {
        Unknown
    };

    enum class StatType
    {
        // TODO
    };

    enum class ItemSlot
    {
        // TODO
    };

    struct Stats
    {
        Stats(const Raw::StatListEx* raw)
        {
            auto& stats = raw->m_baseStats;
            for (uint32_t i = 0; i < stats.m_count; ++i)
            {
                m_stats[static_cast<StatType>(stats.m_pStats->m_statId)] = stats.m_pStats->m_value;
            }
            // TODO fix hp and mana values
        }

        bool Has(StatType aStat) const { return m_stats.contains(aStat); }

        bool GetValue(StatType aStat) const { return m_stats.at(aStat); }

    private:
        std::unordered_map<StatType, int32_t> m_stats;
    };

    struct Position
    {
        const uint16_t x;
        const uint16_t y;
    };

    struct Unit
    {
        Unit(const Raw::StatListEx* aStatList, uint16_t x, uint16_t y)
            : m_stats(aStatList)
            , m_pos({x, y})
        {
        }

        const Stats m_stats;
        const Position m_pos;
    };

    struct Item : public Unit
    {
        Item(const Raw::UnitData<Raw::ItemData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos)
        {
        }

        using Raw = Raw::ItemData;
    };

    struct Player : public Unit
    {
        Player(const Raw::UnitData<Raw::PlayerData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos)
        {
        }

        using Raw = Raw::PlayerData;
    };

    struct Monster : public Unit
    {
        Monster(const Raw::UnitData<Raw::MonsterData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos)
        {
        }

        using Raw = Raw::MonsterData;

        bool IsDead() const { return false; }
    };

    template <typename UnitType>
    struct Units
    {
        Units(const Raw::UnitData<typename UnitType::Raw>* const aRaw[128])
            : m_units(InitializeUnits(aRaw))
        {
        }

        const std::set<const std::unique_ptr<UnitType>>& GetAll() const { return m_units; }

    protected:
        static auto InitializeUnits(const Raw::UnitData<typename UnitType::Raw>* const aRaw[128])
        {
            std::set<std::unique_ptr<UnitType>> result;
            for (uint32_t i = 0; i < 128; ++i)
            {
                IterateThroughUnits(aRaw[i], result);
            }
            return result;
        }

        static void IterateThroughUnits(const Raw::UnitData<typename UnitType::Raw>* aUnit,
                                        std::set<std::unique_ptr<UnitType>>& aOutput)
        {
            if (!aUnit)
            {
                return;
            }
            aOutput.insert(std::make_unique<UnitType>(aUnit));
            IterateThroughUnits(aUnit->m_pPrevUnit, aOutput);
        }

        const std::set<std::unique_ptr<UnitType>> m_units;
    };

    struct Players : public Units<Player>
    {
        Players(const Raw::UnitData<Raw::PlayerData>* const aRaw[128])
            : Units(aRaw)
        {
        }

        const Player& GetLocal() const
        {
            // TODO somehow decide which is local
            return **m_units.begin();
        }
    };

    struct Monsters : public Units<Monster>
    {
        Monsters(const Raw::UnitData<Raw::MonsterData>* const aRaw[128])
            : Units(aRaw)
        {
            for (const auto& mon : m_units)
            {
                if (mon->IsDead())
                {
                    m_dead.insert(mon.get());
                }
                else
                {
                    m_alive.insert(mon.get());
                }
            }
        }

        const std::set<const Monster*>& GetAlive() const { return m_alive; }

        const std::set<const Monster*>& GetDead() const { return m_dead; }

    private:
        std::set<const Monster*> m_alive;
        std::set<const Monster*> m_dead;
    };

    struct Items : public Units<Item>
    {
        Items(const Raw::UnitData<Raw::ItemData>* const aRaw[128])
            : Units(aRaw)
        {
            for (const auto& item : m_units)
            {
                SortOutItem(item.get());
            }
        }

        const std::set<const Item*>& GetEquipped() const { return m_equipped; }

        const std::set<const Item*>& GetDropped() const { return m_dropped; }

        const std::set<const Item*>& GetInInventory() const { return m_inInventory; }

        const std::set<const Item*>& GetInCube() const { return m_inCube; }

        const std::set<const Item*>& GetInStash() const { return m_inStash; }

        std::optional<const Item*> GetInHand() const { return m_inHand; }

        std::optional<const Item*> GetEquipped(ItemSlot aSlot) const { return {}; }

    private:
        void SortOutItem(const Item* item)
        {
            // TODO
        }

        std::set<const Item*> m_equipped;
        std::set<const Item*> m_dropped;
        std::set<const Item*> m_inInventory;
        std::set<const Item*> m_inCube;
        std::set<const Item*> m_inStash;
        std::optional<const Item*> m_inHand;
    };

    template <typename T>
    std::set<T> Union(const std::set<T>& l, const std::set<T>& r)
    {
        std::set<T> result;
        std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
        return result;
    }

    template <typename T>
    std::set<T> operator+(const std::set<T>& l, const std::set<T>& r)
    {
        return Union(l, r);
    }

    template <typename T>
    std::set<T> Intersection(const std::set<T>& l, const std::set<T>& r)
    {
        std::set<T> result;
        std::set_intersection(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
        return result;
    }

    template <typename T>
    std::set<T> operator^(const std::set<T>& l, const std::set<T>& r)
    {
        return Intersection(l, r);
    }

    template <typename T>
    std::set<T> Difference(const std::set<T>& l, const std::set<T>& r)
    {
        std::set<T> result;
        std::set_difference(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
        return result;
    }

    template <typename T>
    std::set<T> operator-(const std::set<T>& l, const std::set<T>& r)
    {
        return Difference(l, r);
    }

    struct DataAccess
    {
        DataAccess(std::shared_ptr<GE::DataAccessor> aDataAccess)
            : m_dataAccess(std::move(aDataAccess))
            , m_difficulty(static_cast<D2Data::Difficulty>(m_dataAccess->Get<Raw::Game>("Game")->m_difficultyLevel))
            , m_gameType(D2Data::GameType{})  // TODO
        {
            const size_t frames = m_dataAccess->GetNumberOfFrames();
            for (size_t i = 1; i <= frames; ++i)
            {
                m_frames.push_back(std::make_unique<FrameData>(*m_dataAccess, frames - i));
            }
        }

        void AdvanceFrame()
        {
            std::rotate(m_frames.rbegin(), m_frames.rbegin() + 1, m_frames.rend());
            m_frames.front() = std::make_unique<FrameData>(*m_dataAccess);
        }

        // Frame independent
        D2Data::Difficulty GetDifficulty() const { return m_difficulty; }

        D2Data::GameType GetGameType() const { return m_gameType; }

        // Frame dependent

    private:
        std::shared_ptr<GE::DataAccessor> m_dataAccess;

        const D2Data::Difficulty m_difficulty;
        const D2Data::GameType m_gameType;

        struct FrameData
        {
            FrameData(const GE::DataAccessor& aDataAccess, size_t aFrame = 0)
                : m_players(aDataAccess.Get<Raw::Game>("Game", aFrame)->m_pPlayerList)
                , m_monsters(aDataAccess.Get<Raw::Game>("Game", aFrame)->m_pMonsterList)
                , m_items(aDataAccess.Get<Raw::Game>("Game", aFrame)->m_pItemList)
            {
            }

            Players m_players;
            Monsters m_monsters;
            Items m_items;
        };

        std::vector<std::unique_ptr<FrameData>> m_frames;
    };

    struct SharedData
    {
        SharedData(std::shared_ptr<D2Data::DataAccess> aDataAccess)
            : m_dataAccess(std::move(aDataAccess))
        {
            Update();
        }

        void Update() noexcept
        {
            // TODO all the fucking updates
        }

    private:
        std::shared_ptr<D2Data::DataAccess> m_dataAccess;

        // Items
        std::set<Item*> m_droppedItems;
        std::set<Item*> m_pickedItems;
        std::set<Item*> m_equippedItems;
        std::set<Item*> m_unequippedItems;

        // Monsters
        std::set<Monster*> m_newMonsters;
        std::set<Monster*> m_deadMonsters;
        std::set<Monster*> m_outMonsters;
        std::set<Monster*> m_inMonsters;
    };
}

using TestAchiBuilder = GE::AchievementBuilder<std::string, GE::None, D2Data::SharedData, D2Data::DataAccess>;
using TestAchievement = decltype(std::declval<TestAchiBuilder>().Build());

void RegisterLayouts(GE::MemoryProcessor& aMemoryProcessor)
{
    auto dynPathLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::DynamicPath)).Build();
    auto gameLayout = GE::LayoutBuilder::MakeAbsolute()
                          ->SetTotalSize(sizeof(Raw::Game))
                          .AddPointerOffsets(0x1120, "UnitData", 128)
                          .AddPointerOffsets(0x1120 + 1 * 128 * 4, "UnitData", 128)
                          .AddPointerOffsets(0x1120 + 3 * 128 * 4, "UnitData", 128)
                          .Build();
    auto inventoryLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::Inventory)).Build();
    auto itemLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::ItemData)).Build();
    auto monsterLayout =
        GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::MonsterData)).AddPointerOffsets(0x2C, 300).Build();
    auto playerDataLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::PlayerData)).Build();
    auto staticPathLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::StaticPath)).Build();
    auto statlistLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::StatList)).Build();
    auto statlistExLayout = GE::LayoutBuilder::MakeAbsolute()
                                ->SetTotalSize(sizeof(Raw::StatListEx))
                                .AddPointerOffsets<Raw::StatListEx>(0x24,
                                                                    [](Raw::StatListEx* aStatList) {
                                                                        return aStatList->m_baseStats.m_count * sizeof(Raw::Stat);
                                                                    })
                                .Build();
    auto unitLayout = GE::LayoutBuilder::MakeAbsolute()
                          ->SetTotalSize(sizeof(Raw::UnitData<>))
                          .AddPointerOffsets<Raw::UnitData<>>(0x14,
                                                              [](Raw::UnitData<>* aUnit) {
                                                                  if (aUnit->m_unitType == 0)
                                                                  {
                                                                      return "PlayerData";
                                                                  }
                                                                  else if (aUnit->m_unitType == 1)
                                                                  {
                                                                      return "MonsterData";
                                                                  }
                                                                  else if (aUnit->m_unitType == 4)
                                                                  {
                                                                      return "ItemData";
                                                                  }
                                                                  throw std::runtime_error("Unknown unit type");
                                                              })
                          .AddPointerOffsets<Raw::UnitData<>>(0x2C,
                                                              [](Raw::UnitData<>* aUnit) {
                                                                  if (aUnit->m_unitType == 0)
                                                                  {
                                                                      return "DynamicPath";
                                                                  }
                                                                  else if (aUnit->m_unitType == 1)
                                                                  {
                                                                      return "DynamicPath";
                                                                  }
                                                                  else if (aUnit->m_unitType == 4)
                                                                  {
                                                                      return "StaticPath";
                                                                  }
                                                                  throw std::runtime_error("Unknown unit type");
                                                              })
                          .AddPointerOffsets(0x5C, "StatListEx")
                          .AddPointerOffsets(0x60, "Inventory")
                          .AddPointerOffsets(0xE4, "UnitData")
                          .Build();

    aMemoryProcessor.RegisterLayout("DynamicPath", dynPathLayout);
    aMemoryProcessor.RegisterLayout("Game", gameLayout);
    aMemoryProcessor.RegisterLayout("Inventory", inventoryLayout);
    aMemoryProcessor.RegisterLayout("ItemData", itemLayout);
    aMemoryProcessor.RegisterLayout("MonsterData", monsterLayout);
    aMemoryProcessor.RegisterLayout("PlayerData", playerDataLayout);
    aMemoryProcessor.RegisterLayout("StaticPath", staticPathLayout);
    aMemoryProcessor.RegisterLayout("StatList", statlistLayout);
    aMemoryProcessor.RegisterLayout("StatListEx", statlistExLayout);
    aMemoryProcessor.RegisterLayout("UnitData", unitLayout);
}

std::string ToString(const GE::ConditionType aType)
{
    switch (aType)
    {
    case GE::ConditionType::Precondition:
        return "Precondition";
    case GE::ConditionType::Activator:
        return "Activator";
    case GE::ConditionType::Invariant:
        return "Invariant";
    case GE::ConditionType::Completer:
        return "Completer";
    case GE::ConditionType::Failer:
        return "Failer";
    case GE::ConditionType::Reseter:
        return "Reseter";
    default:
        return "Unknown";
    }
    return "Unknown";
}

void PrintAchievement(const TestAchievement& aAchievement)
{
    std::cout << "--- " << aAchievement->GetMetadata() << " ---" << std::endl;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GE::ConditionType::All); ++i)
    {
        auto cType = static_cast<GE::ConditionType>(i);
        std::cout << ToString(cType) << ": ";
        const auto& names = aAchievement->GetConditionNames(cType);
        const auto& results = aAchievement->GetConditionResults(cType);
        for (uint32_t j = 0; j < names.size(); ++j)
        {
            std::cout << names[j] << "[" << results[j] << "], ";
        }
        std::cout << std::endl;
    }
}

void PrintAchievements(const std::vector<TestAchievement>& aAchievements)
{
    for (const auto& a : aAchievements)
    {
        PrintAchievement(a);
        std::cout << "\n-----\n" << std::endl;
    }
}

std::vector<TestAchievement> GetAchievements()
{
    std::vector<TestAchievement> achis;
    achis.push_back(TestAchiBuilder("Name").Build());
    return achis;
}

TEST_F(GE_Tests, Test)
{
    // PMA::Setup::InjectLogger(std::make_unique<PMA::ConsoleLogger>(), PMA::LogLevel::Verbose);
    auto config = PMA::TargetProcess::Config{
        .windowInfo = PMA::TargetProcess::Config::WindowInfo{.windowTitle = "Diablo II"},
        .modules = {"D2Client.dll", "D2Common.dll", "D2Win.dll", "D2Lang.dll", "D2Sigma.dll", "D2Game.dll"},
        .pathPrefix = "C:\\games\\median-xl\\"
    };
    auto targetProcess = PMA::TargetProcess::Create(config);
    auto memoryProcessor = GE::MemoryProcessor::Create(std::move(targetProcess));

    RegisterLayouts(*memoryProcessor);

    memoryProcessor->AddStarterLayout("Game", [](PMA::MemoryAccessPtr aMemoryAccess) {
        size_t address = 0;
        EXPECT_NO_THROW(aMemoryAccess->Read("D2Client.dll", 0x12236C, reinterpret_cast<uint8_t*>(&address), sizeof(size_t)));
        return address;
    });
    memoryProcessor->Initialize();

    std::shared_ptr<D2Data::DataAccess> dataAccess;
    std::shared_ptr<D2Data::SharedData> sharedData;
    memoryProcessor->SetOnReadyCallback([&](std::shared_ptr<GE::DataAccessor> aDataAccess) {
        dataAccess = std::make_shared<D2Data::DataAccess>(aDataAccess);
        sharedData = std::make_shared<D2Data::SharedData>(dataAccess);
    });

    std::cout << "Creating achievements" << std::endl;
    auto achis = GetAchievements();

    memoryProcessor->SetUpdateCallback(1000, [&](const GE::DataAccessor&) {
        sharedData->Update();
        for (auto& a : achis)
        {
            a->Update(*dataAccess, *sharedData);
        }
        PrintAchievements(achis);
        if (std::all_of(achis.begin(), achis.end(), [&memoryProcessor](const auto& a) {
                return a->GetStatus() == GE::Status::Completed || a->GetStatus() == GE::Status::Failed;
            }))
        {
            std::cout << "All achievements completed" << std::endl;
            memoryProcessor->RequestStop();
        }
    });

    std::cout << "Starting memoryProcessor" << std::endl;
    memoryProcessor->Start();
    memoryProcessor->Wait();
}