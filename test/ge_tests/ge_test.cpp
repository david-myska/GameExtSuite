#include "ge_test.h"

#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"
#include "pma/logging/console_logger.h"

namespace Raw
{

#pragma pack(push, 1)

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

#pragma pack(pop)

#pragma pack(push, 1)

    struct PlayerData
    {
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

#pragma pack(pop)

#pragma pack(push, 1)

    template <typename UnitType = void>
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
    DynamicPath* m_pPath       = {};                    //  +02C - pPath (union of 2 classes)
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
    uint32_t m_pStatListEx                    = {}; //  +05C - pStatListEx
    uint32_t m_pInventory                     = {}; //  +060 - pInventory
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
    uint32_t m_pPrevUnit       = {}; //  +0E4 - pPrevUnit - previous unit in the unit-type list (the last unit is linked to pGame -> pUnitList[eType][GUID&127]
    uint32_t m_pPrevUnitInRoom = {}; //  +0E8 - pPrevUnitInRoom - the previous unit in the current room
    uint32_t m_pMsgFirst       = {}; //  +0EC - pMsgFirst
    uint32_t m_pMsgLast        = {}; //  +0F0 - pMsgLast
                                // clang-format on
    };

#pragma pack(pop)

#pragma pack(push, 1)

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
    UnitData<>* m_pUnitList[4][128]    = {};  //  +1120 - pUnitList[eType][128] - 5 lists of 128 lists of units (see pUnit documentation)
                                           //  -> second index is GUID & 127, BEWARE: since ever, missiles are array #4 and items are array #3 (so type3=index4 and type4=index3)
    uint32_t m_pTileList        = {};      //  +1B20 - pTileList - a list for all VisTile units
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

TEST_F(GE_Tests, Test)
{
    PMA::Setup::InjectLogger(std::make_unique<PMA::ConsoleLogger>(), PMA::LogLevel::Verbose);
    auto config = PMA::TargetProcess::Config{
        .windowInfo = PMA::TargetProcess::Config::WindowInfo{.windowTitle = "Diablo II"},
        .modules = {"D2Client.dll", "D2Common.dll", "D2Win.dll", "D2Lang.dll", "D2Sigma.dll", "D2Game.dll"},
        .pathPrefix = "C:\\games\\median-xl\\"
    };
    auto targetProcess = PMA::TargetProcess::Create(config);

    auto gameLayout =
        GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::Game)).AddPointerOffsets(0x1120, "Unit", 128).Build();
    auto unitLayout = GE::LayoutBuilder::MakeAbsolute()
                          ->SetTotalSize(sizeof(Raw::UnitData<>))
                          .AddPointerOffsets(0x14, "PlayerData")
                          .AddPointerOffsets(0x2C, "DynPath")
                          .Build();
    auto playerDataLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::PlayerData)).Build();
    auto dynPathLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::DynamicPath)).Build();
    auto memoryProcessor = GE::MemoryProcessor::Create(std::move(targetProcess));
    memoryProcessor->RegisterLayout("Game", gameLayout);
    memoryProcessor->RegisterLayout("Unit", unitLayout);
    memoryProcessor->RegisterLayout("PlayerData", playerDataLayout);
    memoryProcessor->RegisterLayout("DynPath", dynPathLayout);
    memoryProcessor->AddStarterLayout("Game", [](PMA::MemoryAccessPtr aMemoryAccess) {
        size_t address = 0;
        EXPECT_NO_THROW(aMemoryAccess->Read("D2Client.dll", 0x12236C, reinterpret_cast<uint8_t*>(&address), sizeof(size_t)));
        return address;
    });
    memoryProcessor->Initialize();
    memoryProcessor->SetUpdateCallback(1000, [](const GE::DataAccessor& aFrameAccessor) {
        auto frames = aFrameAccessor.Get2Frames<Raw::Game>("Game");
        auto game = std::get<0>(frames);
        std::cout << "D2 Counter: " << game->m_gameFrame << std::endl;
        auto ud = game->m_pPlayerList[1];
        std::cout << ud->m_pUnitData->m_name << ": <" << ud->m_pPath->m_xPos << ":" << ud->m_pPath->m_yPos << ">" << std::endl;
    });
    std::cout << "Starting memoryProcessor" << std::endl;
    memoryProcessor->Start();
    std::cout << "Sleeping for 60s" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
    std::cout << "Done sleeping" << std::endl;
}