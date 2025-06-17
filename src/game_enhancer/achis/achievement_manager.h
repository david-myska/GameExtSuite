#pragma once

#include <filesystem>
#include <fstream>
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
            auto outStream = std::ofstream(m_pathToStorage / aId, std::ios::binary);
            BinWriter bw(outStream);
            m_logger->info("Saving achievements progress");
            for (const auto& [id, achievement] : m_activeAchievements)
            {
                bw.Write(id);
                achievement->Serialize(bw);
            }
        }

        auto Load(std::optional<std::string> aId)
        {
            m_logger->info("Loading achievements progress");
            auto loadedAchis = m_achievementCreator();
            if (!aId)
            {
                m_logger->info("Using default achievements - no achievements file specified");
                return loadedAchis;
            }
            auto inPath = m_pathToStorage / aId.value();
            if (!std::filesystem::exists(inPath))
            {
                m_logger->warn("Using default achievements - selected achievement file does not exist: {}", inPath.string());
                return loadedAchis;
            }
            auto inStream = std::ifstream(inPath, std::ios::binary);
            BinReader br(inStream);
            while (inStream)
            {
                uint32_t id = br.Read<uint32_t>();
                if (inStream.eof())
                {
                    break;
                }
                loadedAchis.at(id)->Deserialize(br);
            }
            return loadedAchis;
        }

        void LoadAndActivate(std::optional<std::string> aId) { Activate(Load(std::move(aId))); }
    };
}
