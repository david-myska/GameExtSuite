#pragma once

#include <string>
#include <format>

namespace D2::Data
{
    enum class Zone : uint16_t
    {
        Invalid,
        // Act1
        Act1_Start,
        Act1_RogueEncampment = Act1_Start,
        Act1_BloodMoor,
        Act1_ColdPlains,
        Act1_StonyField,
        Act1_DarkWood,
        Act1_BlackMarsh,
        Act1_TamoeHighland,
        Act1_DenOfEvil,
        Act1_CaveLevel1,
        Act1_UndergroundPassage,
        Act1_HoleLevel1,
        Act1_PitLevel1,
        Act1_CaveLevel2,
        Act1_UndergroundPassageLevel2,
        Act1_HoleLevel2,
        Act1_PitLevel2,
        Act1_BurialGrounds,
        Act1_Crypt,
        Act1_Mausoleum,
        Act1_ForgottenTower,
        Act1_TowerCellarLevel1,
        Act1_TowerCellarLevel2,
        Act1_TowerCellarLevel3,
        Act1_TowerCellarLevel4,
        Act1_Bloodthrone,
        Act1_MonasteryGate,
        Act1_OuterCloister,
        Act1_Barracks,
        Act1_JailLevel1,
        Act1_JailLevel2,
        Act1_JailLevel3,
        Act1_InnerCloister,
        Act1_Cathedral,
        Act1_CatacombsLevel1,
        Act1_CatacombsLevel2,
        Act1_CatacombsLevel3,
        Act1_CatacombsLevel4,
        Act1_Tristram,
        Act1_End,
        // Act2
        Act2_Start,
        Act2_LutGholein = Act2_Start,
        Act2_RockyWaste,
        Act2_DryHills,
        Act2_FarOasis,
        Act2_LostCity,
        Act2_ValleyOfSnakes,
        Act2_CanyonOfTheMagi,
        Act2_SewersLevel1,
        Act2_SewersLevel2,
        Act2_SewersLevel3,
        Act2_HaremLevel1,
        Act2_HaremLevel2,
        Act2_PalaceCellarLevel1,
        Act2_PalaceCellarLevel2,
        Act2_PalaceCellarLevel3,
        Act2_StonyTombLevel1,
        Act2_HallsOfTheDeadLevel1,
        Act2_HallsOfTheDeadLevel2,
        Act2_ClawViperTempleLevel1,
        Act2_TreasureVault,
        Act2_HallsOfTheDeadLevel3,
        Act2_RitualChamber,
        Act2_MaggotLairLevel1,
        Act2_MaggotLairLevel2,
        Act2_MaggotLairLevel3,
        Act2_TalRashasTomb1 = Act2_MaggotLairLevel3 + 2,
        Act2_TalRashasTomb2,
        Act2_TalRashasTomb3,
        Act2_TalRashasTomb4,
        Act2_TalRashasTomb5,
        Act2_TalRashasTomb6,
        Act2_TalRashasTomb7,
        Act2_DurielsLair,
        Act2_ArcaneSanctuary,
        Act2_End,
        // Act3
        Act3_Start = Act2_End,
        Act3_KurastDocks = Act3_Start,
        Act3_SpiderForest,
        Act3_GreatMarsh,
        Act3_FlayerJungle,
        Act3_LowerKurast,
        Act3_KurastBazaar,
        Act3_UpperKurast,
        Act3_KurastCauseway,
        Act3_Travincal,
        Act3_ArachnidLair,
        Act3_SpiderCavern,
        Act3_SwampyPitLevel1,
        Act3_SwampyPitLevel2,
        Act3_FlayerDungeonLevel1,
        Act3_FlayerDungeonLevel2,
        Act3_FlayerDungeonLevel3,
        Act3_SewersLevel1 = Act3_FlayerDungeonLevel3 + 2,
        Act3_SewersLevel2,
        Act3_RuinedTemple,
        Act3_DisusedFane,
        Act3_ForgottenReliquary,
        Act3_ForgottenTemple,
        Act3_RuinedFane,
        Act3_DisusedReliquary,
        Act3_DuranceOfHateLevel1,
        Act3_DuranceOfHateLevel2,
        Act3_SeatOfHatred,
        Act3_End,
        // Act4
        Act4_Start = Act3_End,
        Act4_PandemoniumFortress = Act4_Start,
        Act4_OuterSteppes,
        Act4_PlainsOfDespair,
        Act4_CityOfTheDamned,
        Act4_RiverOfFlame,
        Act4_ChaosSanctuary,
        Act4_End,
        // Act5
        Act5_Start = Act4_End,
        Act5_Harrogath = Act5_Start,
        Act5_BloodyFoothills,
        Act5_FrigidHighlands,
        Act5_ArreatPlateau,
        Act5_CrystallinePassage,
        Act5_FrozenRiver,
        Act5_GlacialTrail,
        Act5_DrifterCavern,
        Act5_FrozenTundra,
        Act5_AncientsWay,
        Act5_IcyCellar,
        Act5_ArreatSummit,
        Act5_NihlathaksTemple,
        Act5_HallsOfAnguish,
        Act5_HallsOfPain,
        Act5_HallsOfTorment,
        Act5_HallsOfVaught,
        Act5_WorldstoneKeepLevel1 = 128,
        Act5_WorldstoneKeepLevel2,
        Act5_Antechamber,
        Act5_ThroneOfDestruction,
        Act5_WorldstoneChamber,
        Act5_End,
        // Median Challenges
        MXL_Start = Act5_End,
        SecretCowLevel = MXL_Start,
        TranAthulua = 217,
    };

    std::string to_string(Zone zone)
    {
        switch (zone)
        {
        // Act1
        case Zone::Act1_RogueEncampment:
            return "Rogue Encampment";
        case Zone::Act1_BloodMoor:
            return "Blood Moor";
        case Zone::Act1_ColdPlains:
            return "Cold Plains";
        case Zone::Act1_StonyField:
            return "Stony Field";
        case Zone::Act1_DarkWood:
            return "Dark Wood";
        case Zone::Act1_BlackMarsh:
            return "Black Marsh";
        case Zone::Act1_TamoeHighland:
            return "Tamoe Highland";
        case Zone::Act1_DenOfEvil:
            return "Den of Evil";
        case Zone::Act1_CaveLevel1:
            return "Cave Level 1";
        case Zone::Act1_UndergroundPassage:
            return "Underground Passage";
        case Zone::Act1_HoleLevel1:
            return "Hole Level 1";
        case Zone::Act1_PitLevel1:
            return "Pit Level 1";
        case Zone::Act1_CaveLevel2:
            return "Cave Level 2";
        case Zone::Act1_UndergroundPassageLevel2:
            return "Underground Passage Level 2";
        case Zone::Act1_HoleLevel2:
            return "Hole Level 2";
        case Zone::Act1_PitLevel2:
            return "Pit Level 2";
        case Zone::Act1_BurialGrounds:
            return "Burial Grounds";
        case Zone::Act1_Crypt:
            return "Crypt";
        case Zone::Act1_Mausoleum:
            return "Mausoleum";
        case Zone::Act1_ForgottenTower:
            return "Forgotten Tower";
        case Zone::Act1_TowerCellarLevel1:
            return "Tower Cellar Level 1";
        case Zone::Act1_TowerCellarLevel2:
            return "Tower Cellar Level 2";
        case Zone::Act1_TowerCellarLevel3:
            return "Tower Cellar Level 3";
        case Zone::Act1_TowerCellarLevel4:
            return "Tower Cellar Level 4";
        case Zone::Act1_Bloodthrone:
            return "Bloodthrone";
        case Zone::Act1_MonasteryGate:
            return "Monastery Gate";
        case Zone::Act1_OuterCloister:
            return "Outer Cloister";
        case Zone::Act1_Barracks:
            return "Barracks";
        case Zone::Act1_JailLevel1:
            return "Jail Level 1";
        case Zone::Act1_JailLevel2:
            return "Jail Level 2";
        case Zone::Act1_JailLevel3:
            return "Jail Level 3";
        case Zone::Act1_InnerCloister:
            return "Inner Cloister";
        case Zone::Act1_Cathedral:
            return "Cathedral";
        case Zone::Act1_CatacombsLevel1:
            return "Catacombs Level 1";
        case Zone::Act1_CatacombsLevel2:
            return "Catacombs Level 2";
        case Zone::Act1_CatacombsLevel3:
            return "Catacombs Level 3";
        case Zone::Act1_CatacombsLevel4:
            return "Catacombs Level 4";
        case Zone::Act1_Tristram:
            return "Tristram";
        // Act2
        case Zone::Act2_LutGholein:
            return "Lut Gholein";
        case Zone::Act2_RockyWaste:
            return "Rocky Waste";
        case Zone::Act2_DryHills:
            return "Dry Hills";
        case Zone::Act2_FarOasis:
            return "Far Oasis";
        case Zone::Act2_LostCity:
            return "Lost City";
        case Zone::Act2_ValleyOfSnakes:
            return "Valley of Snakes";
        case Zone::Act2_CanyonOfTheMagi:
            return "Canyon of the Magi";
        case Zone::Act2_SewersLevel1:
            return "Sewers Level 1";
        case Zone::Act2_SewersLevel2:
            return "Sewers Level 2";
        case Zone::Act2_SewersLevel3:
            return "Sewers Level 3";
        case Zone::Act2_HaremLevel1:
            return "Harem Level 1";
        case Zone::Act2_HaremLevel2:
            return "Harem Level 2";
        case Zone::Act2_PalaceCellarLevel1:
            return "Palace Cellar Level 1";
        case Zone::Act2_PalaceCellarLevel2:
            return "Palace Cellar Level 2";
        case Zone::Act2_PalaceCellarLevel3:
            return "Palace Cellar Level 3";
        case Zone::Act2_StonyTombLevel1:
            return "Stony Tomb Level 1";
        case Zone::Act2_HallsOfTheDeadLevel1:
            return "Halls of the Dead Level 1";
        case Zone::Act2_HallsOfTheDeadLevel2:
            return "Halls of the Dead Level 2";
        case Zone::Act2_ClawViperTempleLevel1:
            return "Claw Viper Temple Level 1";
        case Zone::Act2_TreasureVault:
            return "Treasure Vault";
        case Zone::Act2_HallsOfTheDeadLevel3:
            return "Halls of the Dead Level 3";
        case Zone::Act2_RitualChamber:
            return "Ritual Chamber";
        case Zone::Act2_MaggotLairLevel1:
            return "Maggot Lair Level 1";
        case Zone::Act2_MaggotLairLevel2:
            return "Maggot Lair Level 2";
        case Zone::Act2_MaggotLairLevel3:
            return "Maggot Lair Level 3";
        case Zone::Act2_TalRashasTomb1:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb2:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb3:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb4:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb5:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb6:
            [[fallthrough]];
        case Zone::Act2_TalRashasTomb7:
            return "Tal Rasha's Tomb";
        case Zone::Act2_DurielsLair:
            return "Duriel's Lair";
        case Zone::Act2_ArcaneSanctuary:
            return "Arcane Sanctuary";
        // Act3
        case Zone::Act3_KurastDocks:
            return "Kurast Docks";
        case Zone::Act3_SpiderForest:
            return "Spider Forest";
        case Zone::Act3_GreatMarsh:
            return "Great Marsh";
        case Zone::Act3_FlayerJungle:
            return "Flayer Jungle";
        case Zone::Act3_LowerKurast:
            return "Lower Kurast";
        case Zone::Act3_KurastBazaar:
            return "Kurast Bazaar";
        case Zone::Act3_UpperKurast:
            return "Upper Kurast";
        case Zone::Act3_KurastCauseway:
            return "Kurast Causeway";
        case Zone::Act3_Travincal:
            return "Travincal";
        case Zone::Act3_SpiderCavern:
            return "Spider Cavern";
        case Zone::Act3_ArachnidLair:
            return "Arachnid Lair";
        case Zone::Act3_SwampyPitLevel1:
            return "Swampy Pit Level 1";
        case Zone::Act3_SwampyPitLevel2:
            return "Swampy Pit Level 2";
        case Zone::Act3_FlayerDungeonLevel1:
            return "Flayer Dungeon Level 1";
        case Zone::Act3_FlayerDungeonLevel2:
            return "Flayer Dungeon Level 2";
        case Zone::Act3_FlayerDungeonLevel3:
            return "Flayer Dungeon Level 3";
        case Zone::Act3_SewersLevel1:
            return "Sewers Level 1";
        case Zone::Act3_SewersLevel2:
            return "Sewers Level 2";
        case Zone::Act3_RuinedTemple:
            return "Ruined Temple";
        case Zone::Act3_DisusedFane:
            return "Disused Fane";
        case Zone::Act3_ForgottenReliquary:
            return "Forgotten Reliquary";
        case Zone::Act3_ForgottenTemple:
            return "Forgotten Temple";
        case Zone::Act3_RuinedFane:
            return "Ruined Fane";
        case Zone::Act3_DisusedReliquary:
            return "Disused Reliquary";
        case Zone::Act3_DuranceOfHateLevel1:
            return "Durance of Hate Level 1";
        case Zone::Act3_DuranceOfHateLevel2:
            return "Durance of Hate Level 2";
        case Zone::Act3_SeatOfHatred:
            return "Seat of Hatred";
        // Act4
        case Zone::Act4_CityOfTheDamned:
            return "City of the Damned";
        case Zone::Act4_PlainsOfDespair:
            return "Plains of Despair";
        case Zone::Act4_OuterSteppes:
            return "Outer Steppes";
        case Zone::Act4_RiverOfFlame:
            return "River of Flame";
        case Zone::Act4_PandemoniumFortress:
            return "The Pandemonium Fortress";
        case Zone::Act4_ChaosSanctuary:
            return "The Chaos Sanctuary";
        // Act5
        case Zone::Act5_Harrogath:
            return "Harrogath";
        case Zone::Act5_BloodyFoothills:
            return "Bloody Foothills";
        case Zone::Act5_FrigidHighlands:
            return "Frigid Highlands";
        case Zone::Act5_ArreatPlateau:
            return "Arreat Plateau";
        case Zone::Act5_CrystallinePassage:
            return "Crystalline Passage";
        case Zone::Act5_FrozenRiver:
            return "Frozen River";
        case Zone::Act5_GlacialTrail:
            return "Glacial Trail";
        case Zone::Act5_DrifterCavern:
            return "Drifter Cavern";
        case Zone::Act5_FrozenTundra:
            return "Frozen Tundra";
        case Zone::Act5_AncientsWay:
            return "The Ancients' Way";
        case Zone::Act5_IcyCellar:
            return "Icy Cellar";
        case Zone::Act5_ArreatSummit:
            return "Arreat Summit";
        case Zone::Act5_NihlathaksTemple:
            return "Nihlathak's Temple";
        case Zone::Act5_HallsOfAnguish:
            return "Halls of Anguish";
        case Zone::Act5_HallsOfPain:
            return "Halls of Pain";
        case Zone::Act5_HallsOfTorment:
            return "Halls of Torment";
        case Zone::Act5_HallsOfVaught:
            return "Halls of Vaught";
        case Zone::Act5_WorldstoneKeepLevel1:
            return "Worldstone Keep Level 1";
        case Zone::Act5_WorldstoneKeepLevel2:
            return "Worldstone Keep Level 2";
        case Zone::Act5_Antechamber:
            return "Antechamber";
        case Zone::Act5_WorldstoneChamber:
            return "The Worldstone Chamber";
        case Zone::Act5_ThroneOfDestruction:
            return "Throne of Destruction";
        // Median Challenges
        case Zone::SecretCowLevel:
            return "The Secret Cow Level";
        case Zone::TranAthulua:
            return "Tran Athulua";
        // TODO
        default:
            return std::format("Unknown zone ({})", static_cast<uint16_t>(zone));
        }
    }
}
