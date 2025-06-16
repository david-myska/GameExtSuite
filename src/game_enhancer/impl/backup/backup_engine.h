#pragma once

#include "game_enhancer/backup/backup_engine.h"

namespace GE
{
    class BackupEngineImpl : public BackupEngine
    {
        std::filesystem::path m_targetPath;
        std::filesystem::path m_backupPath;
        size_t m_maxBackupSize = 16 * 1024 * 1024;  // 16 MB

        std::shared_ptr<spdlog::logger> m_logger;

        void CheckTargetDirectorySize() const;

    public:
        BackupEngineImpl(std::filesystem::path aTargetPath, std::filesystem::path aBackupPath,
                         std::shared_ptr<spdlog::logger> aLogger);

        void Backup(const std::optional<std::string>& aBackupName) const override;

        void Restore(const std::string& aBackupName, bool aBackupCurrent) const override;

        std::vector<std::string> GetAvailableBackups() const override;

        void SetMaxBackupSize(size_t aMaxBackupSize) override;
    };
}