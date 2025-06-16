#pragma once

#include <filesystem>
#include <iostream>
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

        std::filesystem::path m_pathToStorage;

        std::map<uint32_t, std::unique_ptr<AchievementType>> m_activeAchievements;

        std::shared_ptr<spdlog::logger> m_logger;

    public:
        AchievementManager(std::function<std::map<uint32_t, std::unique_ptr<AchievementType>>()> aAchievementCreator,
                           std::filesystem::path aPathToStorage, std::shared_ptr<spdlog::logger> aLogger)
            : m_achievementCreator(std::move(aAchievementCreator))
            , m_pathToStorage(std::move(aPathToStorage))
            , m_logger(std::move(aLogger))
        {
            m_logger->debug("AchievementManager created with {} achievements", m_achievementCreator().size());
            if (!std::filesystem::exists(m_pathToStorage))
            {
                m_logger->info("Creating achievements storage directory: {}", m_pathToStorage.string());
                std::filesystem::create_directories(m_pathToStorage);
            }
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
        void Save(const std::string& aId)
        {
            if (aId.empty())
            {
                throw std::invalid_argument("Save Id cannot be empty");
            }
            auto aOutPath = m_pathToStorage / aId;
            auto aOut = std::ofstream(aOutPath, std::ios::binary);
            m_logger->info("Saving achievements progress");
            for (const auto& [id, achievement] : m_activeAchievements)
            {
                aOut.write(&id, sizeof(id));
                achievement->Serialize(aOut);
            }
        }

        auto Load(std::optional<std::string> aId)
        {
            m_logger->info("Loading achievements progress");
            auto loadedAchis = m_achievementCreator();
            if (!aId)
            {
                m_logger->info("No achievements file specified, creating new achievements");
                return loadedAchis;
            }
            auto aInPath = m_pathToStorage / aId.value();
            if (!std::filesystem::exists(aInPath.value()))
            {
                m_logger->error("Selected achievement file does not exist: {}", aInPath->string());
                throw std::runtime_error("Selected achievement file does not exist");
            }
            auto aIn = std::ifstream(aInPath.value(), std::ios::binary);
            while (aIn)
            {
                uint32_t id = 0;
                aIn.read(&id, sizeof(id));
                if (aIn.eof())
                {
                    break;
                }
                loadedAchis.at(id)->Deserialize(aIn);
            }
            return loadedAchis;
        }

        void LoadAndActivate(std::istream& aIn) { Activate(Load(aIn)); }
    };
}
