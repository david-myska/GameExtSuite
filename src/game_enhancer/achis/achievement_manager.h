#pragma once

#include <map>

#include "game_enhancer/achis/achievement.h"

namespace GE
{
    template <typename Metadata, typename SharedData, typename DataAccess = GE::DataAccessor>
    class AchievementManager
    {
        std::map<uint32_t, Achievement<Metadata, SharedData, DataAccess>> m_achievements;

    public:
        AchievementManager(std::map<uint32_t, Achievement<Metadata, SharedData, DataAccess>> aAchievements)
            : m_achievements(std::move(aAchievements))
        {
        }

        void Update(const DataAccess& aDataAccess, const SharedData& aSharedData)
        {
            for (auto& [_, achievement] : m_achievements)
            {
                achievement.Update(aDataAccess, aSharedData);
            }
        }

        void Reset()
        {
            for (auto& [_, achievement] : m_achievements)
            {
                achievement.Reset();
            }
        }

        void Save()
        {
            for (auto& [_, achievement] : m_achievements)
            {
                achievement.Save();
            }
        }

        void Load()
        {
            for (auto& [_, achievement] : m_achievements)
            {
                achievement.Load();
            }
        }
    };
}
