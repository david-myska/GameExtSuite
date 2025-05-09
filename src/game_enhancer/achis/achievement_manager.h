#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>

#include "game_enhancer/achis/achievement.h"

namespace GE
{
    template <class AchievementType>
    class AchievementManager
    {
        std::function<std::map<uint32_t, std::unique_ptr<AchievementType>>()> m_achievementCreator;

        std::map<uint32_t, std::unique_ptr<AchievementType>> m_activeAchievements;

    public:
        AchievementManager(std::function<std::map<uint32_t, std::unique_ptr<AchievementType>>()> aAchievementCreator)
            : m_achievementCreator(std::move(aAchievementCreator))
        {
        }

        void Activate(std::map<uint32_t, std::unique_ptr<AchievementType>> aAchievements)
        {
            Deactivate();
            m_activeAchievements = std::move(aAchievements);
        }

        /*
         * Applies to the current active set of achievements.
         */
        void Deactivate() { m_activeAchievements.clear(); }

        /*
         * Applies to the current active set of achievements.
         */
        void Update(const typename AchievementType::_DataAccess& aDataAccess,
                    const typename AchievementType::_SharedData& aSharedData)
        {
            for (auto& [_, achievement] : m_activeAchievements)
            {
                achievement->Update(aDataAccess, aSharedData);
            }
        }

        const auto& GetActiveAchievements() const { return m_activeAchievements; }

        /*
         * Applies to the current active set of achievements.
         */
        void Save(std::ostream& aOut)
        {
            for (const auto& [id, achievement] : m_activeAchievements)
            {
                aOut << id;
                achievement->Serialize(aOut);
            }
        }

        auto Load(std::istream& aIn)
        {
            auto loadedAchis = m_achievementCreator();
            while (aIn)
            {
                uint32_t id = 0;
                aIn >> id;
                loadedAchis.at(id)->Deserialize(aIn);
            }
            // TODO define file structure & add error checking
            return loadedAchis;
        }

        void LoadAndActivate(std::istream& aIn) { Activate(Load(aIn)); }
    };
}
