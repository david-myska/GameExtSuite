#pragma once

#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

#include "raw.h"
#include "stats.h"

#include "game_enhancer/data_accessor.h"

namespace D2::Data
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

    enum class ItemSlot
    {
        // TODO
    };

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

        int32_t GetValue(StatType aStat) const { return m_stats.at(aStat); }

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
            std::rotate(m_frames.rbegin(), m_frames.rbegin() + 1, m_frames.rend());
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

    private:
        std::shared_ptr<GE::DataAccessor> m_dataAccess;

        const Difficulty m_difficulty;
        const GameType m_gameType;

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
        SharedData(std::shared_ptr<DataAccess> aDataAccess)
            : m_dataAccess(std::move(aDataAccess))
        {
            Update();
        }

        void Update() noexcept
        {
            // TODO all the fucking updates
        }

    private:
        std::shared_ptr<DataAccess> m_dataAccess;

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
