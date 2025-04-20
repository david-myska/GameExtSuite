#pragma once

#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

#include "raw.h"
#include "stats.h"
#include "zone.h"

#include "game_enhancer/data_accessor.h"

struct ScatteredLayout
{
    uint16_t* m_inGame;
};

struct GameUtilsLayout
{
    D2::Data::Zone* m_zone;
};

namespace D2::Data
{
    enum class Difficulty
    {
        Normal,
        Nightmare,
        Hell
    };

    enum class Act
    {
        Act1,
        Act2,
        Act3,
        Act4,
        Act5
    };

    enum class GameType
    {
        Unknown
    };

    enum class ItemSlot
    {
        // TODO
    };

    using GUID = uint32_t;

    struct Stats
    {
        Stats(const Raw::StatListEx* raw)
        {
            auto& stats = raw->m_fullStats;
            for (uint32_t i = 0; i < stats.m_count; ++i)
            {
                m_stats[static_cast<StatType>(stats.m_pStats->m_statId)] = stats.m_pStats->m_value;
            }
            for (auto stat : {StatType::Life, StatType::MaxLife, StatType::Mana, StatType::MaxMana})
            {
                if (m_stats.contains(stat))
                {
                    m_stats[stat] >>= 8;
                }
            }
        }

        bool Has(StatType aStat) const { return m_stats.contains(aStat); }

        std::optional<int32_t> GetValue(StatType aStat) const
        {
            if (Has(aStat))
            {
                return m_stats.at(aStat);
            }
            return {};
        }

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
        Unit(const Raw::StatListEx* aStatList, uint16_t x, uint16_t y, uint32_t id)
            : m_stats(aStatList)
            , m_pos({x, y})
            , m_id(id)
        {
        }

        bool IsAlive() const { return m_stats.GetValue(StatType::Life).value_or(0) > 0; }

        bool IsDead() const { return !IsAlive(); }

        virtual ~Unit() = default;

        const Stats m_stats;
        const Position m_pos;
        const GUID m_id;
    };

    struct Item : public Unit
    {
        Item(const Raw::UnitData<Raw::ItemData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos, aRaw->m_GUID)
        {
        }

        using Raw = Raw::ItemData;
    };

    struct Player : public Unit
    {
        Player(const Raw::UnitData<Raw::PlayerData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos, aRaw->m_GUID)
        {
        }

        using Raw = Raw::PlayerData;
    };

    struct Monster : public Unit
    {
        Monster(const Raw::UnitData<Raw::MonsterData>* aRaw)
            : Unit(aRaw->m_pStatListEx, aRaw->m_pPath->m_xPos, aRaw->m_pPath->m_yPos, aRaw->m_GUID)
            , m_name(ConvertName(aRaw->m_pUnitData->m_pMonNameOrAiParams))
        {
        }

        using Raw = Raw::MonsterData;

        const std::string m_name;

    private:
        static std::string ConvertWCharToString(const wchar_t* aWChar)
        {
#pragma warning(push)
#pragma warning(disable : 4244)
            return std::string(aWChar, aWChar + wcslen(aWChar));  // D2 is using only ASCII
#pragma warning(pop)
        }

        static std::string ConvertName(const wchar_t* aWChar)
        {
            auto name = ConvertWCharToString(aWChar);
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);
            return name;
        }
    };

    template <typename UnitType>
        requires std::is_base_of_v<Unit, UnitType>
    class Units
    {
        const std::map<GUID, std::unique_ptr<UnitType>> m_theUnits;

    protected:
        std::map<GUID, const UnitType*> m_units;

        static auto InitializeUnits(const Raw::UnitData<typename UnitType::Raw>* const aRaw[128])
        {
            std::map<GUID, std::unique_ptr<UnitType>> result;
            for (uint32_t i = 0; i < 128; ++i)
            {
                IterateThroughUnits(aRaw[i], result);
            }
            return result;
        }

        static void IterateThroughUnits(const Raw::UnitData<typename UnitType::Raw>* aUnit,
                                        std::map<GUID, std::unique_ptr<UnitType>>& aOutput)
        {
            if (!aUnit)
            {
                return;
            }
            aOutput[aUnit->m_GUID] = std::make_unique<UnitType>(aUnit);
            IterateThroughUnits(aUnit->m_pPrevUnit, aOutput);
        }

    public:
        Units(const Raw::UnitData<typename UnitType::Raw>* const aRaw[128])
            : m_theUnits(InitializeUnits(aRaw))
        {
            for (const auto& [id, unit] : m_theUnits)
            {
                m_units[id] = unit.get();
            }
        }

        const std::map<GUID, const UnitType*>& Get() const { return m_units; }

        const UnitType* GetById(uint32_t aId) const { return m_units.contains(aId) ? m_units.at(aId) : nullptr; }
    };

    struct Players : public Units<Player>
    {
        Players(const Raw::UnitData<Raw::PlayerData>* const aRaw[128])
            : Units(aRaw)
        {
        }

        const Player* GetLocal() const
        {
            // TODO somehow decide which is local
            return m_units.begin()->second;
        }
    };

    struct Monsters : public Units<Monster>
    {
        Monsters(const Raw::UnitData<Raw::MonsterData>* const aRaw[128])
            : Units(aRaw)
        {
            for (const auto& [id, mon] : m_units)
            {
                if (mon->IsAlive())
                {
                    m_alive[id] = mon;
                }
                else
                {
                    m_dead[id] = mon;
                }
            }
        }

        const std::map<GUID, const Monster*>& GetAlive() const { return m_alive; }

        const std::map<GUID, const Monster*>& GetDead() const { return m_dead; }

        std::map<GUID, const Monster*> GetByName(const std::string& aName) const
        {
            std::map<GUID, const Monster*> result;
            for (const auto& [id, unit] : m_units)
            {
                if (unit->m_name == aName)
                {
                    result[id] = unit;
                }
            }
            return result;
        }

    private:
        std::map<GUID, const Monster*> m_alive;
        std::map<GUID, const Monster*> m_dead;
    };

    struct Items : public Units<Item>
    {
        Items(const Raw::UnitData<Raw::ItemData>* const aRaw[128])
            : Units(aRaw)
        {
            for (const auto& [id, item] : m_units)
            {
                SortOutItem(id, item);
            }
        }

        const std::map<GUID, const Item*>& GetEquipped() const { return m_equipped; }

        const std::map<GUID, const Item*>& GetDropped() const { return m_dropped; }

        const std::map<GUID, const Item*>& GetInInventory() const { return m_inInventory; }

        const std::map<GUID, const Item*>& GetInCube() const { return m_inCube; }

        const std::map<GUID, const Item*>& GetInStash() const { return m_inStash; }

        std::optional<const Item*> GetInHand() const { return m_inHand; }

        std::optional<const Item*> GetEquipped(ItemSlot aSlot) const { return {}; }

    private:
        void SortOutItem(GUID id, const Item* item)
        {
            // TODO
        }

        std::map<GUID, const Item*> m_equipped;
        std::map<GUID, const Item*> m_dropped;
        std::map<GUID, const Item*> m_inInventory;
        std::map<GUID, const Item*> m_inCube;
        std::map<GUID, const Item*> m_inStash;
        std::optional<const Item*> m_inHand;
    };

    struct Misc
    {
        Misc(Zone aZone)
            : m_zone(aZone)
        {
        }

        Zone GetZone() const { return m_zone; }

    private:
        const Zone m_zone;
    };

    template <typename T>
    std::set<T> Union(const std::set<T>& l, const std::set<T>& r)
    {
        std::set<T> result;
        std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
        return result;
    }

    template <typename T>
    std::map<uint32_t, T> Union(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        std::map<uint32_t, T> result(l);
        std::map<uint32_t, T> tmp(r);
        result.merge(tmp);
        return result;
    }

    template <typename T>
    std::set<T> operator+(const std::set<T>& l, const std::set<T>& r)
    {
        return Union(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> operator+(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        return Union(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> Intersection(const std::map<uint32_t, T>& l, const std::set<uint32_t>& r)
    {
        std::map<uint32_t, T> result;
        auto insertIt = std::inserter(result, result.begin());
        auto first1 = l.begin();
        auto last1 = l.end();
        auto first2 = r.begin();
        auto last2 = r.end();

        while (first1 != last1 && first2 != last2)
        {
            if (first1->first < *first2)
            {
                ++first1;
            }
            else
            {
                if (!(*first2 < first1->first))
                {
                    *insertIt++ = *first1++;
                }
                ++first2;
            }
        }
        return result;
    }

    template <typename T>
    std::map<uint32_t, T> Intersection(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        std::map<uint32_t, T> result;
        auto insertIt = std::inserter(result, result.begin());
        auto first1 = l.begin();
        auto last1 = l.end();
        auto first2 = r.begin();
        auto last2 = r.end();

        while (first1 != last1 && first2 != last2)
        {
            if (first1->first < first2->first)
            {
                ++first1;
            }
            else
            {
                if (!(first2->first < first1->first))
                {
                    *insertIt++ = *first1++;
                }
                ++first2;
            }
        }
        return result;
    }

    template <typename T>
    std::set<T> Intersection(const std::set<T>& l, const std::set<T>& r)
    {
        std::set<T> result;
        std::set_intersection(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
        return result;
    }

    template <typename T>
    std::set<T> operator&(const std::set<T>& l, const std::set<T>& r)
    {
        return Intersection(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> operator&(const std::map<uint32_t, T>& l, const std::set<uint32_t>& r)
    {
        return Intersection(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> operator&(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        return Intersection(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> Difference(const std::map<uint32_t, T>& l, const std::set<uint32_t>& r)
    {
        std::map<uint32_t, T> result;
        auto insertIt = std::inserter(result, result.begin());
        auto first1 = l.begin();
        auto last1 = l.end();
        auto first2 = r.begin();
        auto last2 = r.end();

        while (first1 != last1)
        {
            if (first2 == last2)
            {
                std::copy(first1, last1, insertIt);
                break;
            }

            if (first1->first < *first2)
            {
                *insertIt++ = *first1++;
            }
            else
            {
                if (!(*first2 < first1->first))
                {
                    ++first1;
                }
                ++first2;
            }
        }
        return result;
    }

    template <typename T>
    std::map<uint32_t, T> Difference(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        std::map<uint32_t, T> result;
        auto insertIt = std::inserter(result, result.begin());
        auto first1 = l.begin();
        auto last1 = l.end();
        auto first2 = r.begin();
        auto last2 = r.end();

        while (first1 != last1)
        {
            if (first2 == last2)
            {
                std::copy(first1, last1, insertIt);
                break;
            }

            if (first1->first < first2->first)
            {
                *insertIt++ = *first1++;
            }
            else
            {
                if (!(first2->first < first1->first))
                {
                    ++first1;
                }
                ++first2;
            }
        }
        return result;
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

    template <typename T>
    std::map<uint32_t, T> operator-(const std::map<uint32_t, T>& l, const std::set<uint32_t>& r)
    {
        return Difference(l, r);
    }

    template <typename T>
    std::map<uint32_t, T> operator-(const std::map<uint32_t, T>& l, const std::map<uint32_t, T>& r)
    {
        return Difference(l, r);
    }

    template <typename T>
        requires std::is_base_of_v<Unit, T>
    std::set<uint32_t> ToIds(const std::set<const T*>& aUnits)
    {
        std::set<uint32_t> result;
        std::transform(aUnits.begin(), aUnits.end(), std::inserter(result, result.begin()), [](const T* unit) {
            return unit->m_id;
        });
        return result;
    }

    struct DataAccess
    {
        DataAccess(std::shared_ptr<GE::DataAccessor> aDataAccess)
            : m_dataAccess(std::move(aDataAccess))
            , m_difficulty(static_cast<Difficulty>(m_dataAccess->Get<Raw::Game>("Game")->m_difficultyLevel))
            , m_gameType(GameType{})  // TODO
        {
            const size_t frames = m_dataAccess->GetNumberOfFrames();
            for (size_t i = 1; i <= frames; ++i)
            {
                m_frames.push_back(std::make_unique<FrameData>(*m_dataAccess, frames - i));
            }
        }

        void AdvanceFrame()
        {
            // Trying out how it will work with allMonsters being updated 1 frame later
            for (const auto& mon : m_frames.back()->m_monsters.Get())
            {
                m_allMonsters.insert(mon.first);
            }
            std::rotate(m_frames.rbegin(), m_frames.rbegin() + 1, m_frames.rend());  // inefficient for vector
            m_frames.front() = std::make_unique<FrameData>(*m_dataAccess);
        }

        // Frame independent
        Difficulty GetDifficulty() const { return m_difficulty; }

        GameType GetGameType() const { return m_gameType; }

        // Frame dependent
        uint32_t GetCurrentGameFrame() const { return m_dataAccess->Get<Raw::Game>("Game")->m_gameFrame; }

        const Players& GetPlayers(size_t aFrame = 0) const { return m_frames.at(aFrame)->m_players; }

        const Monsters& GetMonsters(size_t aFrame = 0) const { return m_frames.at(aFrame)->m_monsters; }

        const Items& GetItems(size_t aFrame = 0) const { return m_frames.at(aFrame)->m_items; }

        const Misc& GetMisc(size_t aFrame = 0) const { return m_frames.at(aFrame)->m_misc; }

    private:
        std::shared_ptr<GE::DataAccessor> m_dataAccess;

        const Difficulty m_difficulty;
        const GameType m_gameType;

        struct FrameData
        {
            FrameData(const GE::DataAccessor& aDataAccess, size_t aFrame = 0)
                : m_players(aDataAccess.Get<Raw::ClientUnits>("ClientUnits", aFrame)->m_pPlayerList)
                , m_monsters(aDataAccess.Get<Raw::ClientUnits>("ClientUnits", aFrame)->m_pMonsterList)
                , m_items(aDataAccess.Get<Raw::ClientUnits>("ClientUnits", aFrame)->m_pItemList)
                , m_misc(*aDataAccess.Get<GameUtilsLayout>("GameUtils")->m_zone)
            {
            }

            Players m_players;
            Monsters m_monsters;
            Items m_items;
            Misc m_misc;
        };

        std::vector<std::unique_ptr<FrameData>> m_frames;

    public:
        // All stuff
        // std::set<uint32_t> m_allPlayers;
        std::set<uint32_t> m_allMonsters;
        // std::set<uint32_t> m_allItems;
    };

    struct SharedData
    {
        SharedData(std::shared_ptr<DataAccess> aDataAccess)
            : m_dataAccess(std::move(aDataAccess))
        {
            Update();
        }

        void Update() noexcept
        {
            m_droppedItems = m_dataAccess->GetItems().GetDropped() - m_dataAccess->GetItems(1).GetDropped();
            m_pickedItems = (m_dataAccess->GetItems().GetEquipped() + m_dataAccess->GetItems().GetInInventory()) -
                            (m_dataAccess->GetItems(1).GetEquipped() + m_dataAccess->GetItems(1).GetInInventory());
            m_equippedItems = m_dataAccess->GetItems().GetEquipped() - m_dataAccess->GetItems(1).GetEquipped();
            m_unequippedItems = m_dataAccess->GetItems(1).GetEquipped() - m_dataAccess->GetItems().GetEquipped();

            m_newMonsters = m_dataAccess->GetMonsters().Get() - m_dataAccess->m_allMonsters;
            m_deadMonsters = m_dataAccess->GetMonsters().GetDead() & m_dataAccess->GetMonsters(1).GetAlive();
            m_outMonsters = m_dataAccess->GetMonsters(1).GetAlive() - m_dataAccess->GetMonsters().GetAlive();
        }

    private:
        std::shared_ptr<DataAccess> m_dataAccess;

        // Items
        std::map<GUID, const Item*> m_droppedItems;
        std::map<GUID, const Item*> m_pickedItems;
        std::map<GUID, const Item*> m_equippedItems;
        std::map<GUID, const Item*> m_unequippedItems;

        // Monsters
        std::map<GUID, const Monster*> m_newMonsters;
        std::map<GUID, const Monster*> m_deadMonsters;
        std::map<GUID, const Monster*> m_outMonsters;
    };
}
