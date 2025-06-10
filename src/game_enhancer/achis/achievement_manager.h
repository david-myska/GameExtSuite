#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>

#include "spdlog/spdlog.h"

namespace GE
{
    template <class AchievementType>
    class AchievementManager
    {
        std::function<std::map<uint32_t, std::unique_ptr<AchievementType>>()> m_achievementCreator;

        std::map<uint32_t, std::unique_ptr<AchievementType>> m_activeAchievements;

        std::shared_ptr<spdlog::logger> m_logger;

    public:
        AchievementManager(std::function<std::map<uint32_t, std::unique_ptr<AchievementType>>()> aAchievementCreator,
                           std::shared_ptr<spdlog::logger> aLogger)
            : m_achievementCreator(std::move(aAchievementCreator))
            , m_logger(std::move(aLogger))
        {
            m_logger->debug("AchievementManager created with {} achievements", m_achievementCreator().size());
        }

        void Activate(std::map<uint32_t, std::unique_ptr<AchievementType>> aAchievements)
        {
            Deactivate();
            m_logger->info("New achievements activated");
            m_activeAchievements = std::move(aAchievements);
        }

        /*
         * Applies to the current active set of achievements.
         */
        void Deactivate()
        {
            m_logger->info("Achievements deactivated");
            m_activeAchievements.clear();
        }

        /*
         * Applies to the current active set of achievements.
         */
        void Update(const typename AchievementType::_DataAccess& aDataAccess,
                    const typename AchievementType::_SharedData& aSharedData)
        {
            m_logger->trace("Updating achievements");
            for (auto& [_, achievement] : m_activeAchievements)
            {
                achievement->Update(aDataAccess, aSharedData);
            }
            m_logger->trace("Finished updating achievements");
        }

        const auto& GetActiveAchievements() const { return m_activeAchievements; }

        /*
         * Applies to the current active set of achievements.
         */
        void Save(std::ostream& aOut)
        {
            m_logger->info("Saving achievements progress");
            for (const auto& [id, achievement] : m_activeAchievements)
            {
                aOut << id;
                achievement->Serialize(aOut);
            }
        }

        auto Load(std::istream& aIn)
        {
            m_logger->info("Loading achievements progress");
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
