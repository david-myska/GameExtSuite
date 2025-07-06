#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "spdlog/spdlog.h"

namespace GE
{
    struct BackupEngine;
    using BackupEnginePtr = std::shared_ptr<BackupEngine>;

    struct BackupEngine
    {
        virtual ~BackupEngine() = default;

        /*
         * aTargetPath specifies where the target files to be backed up are located.
         * aBackupPath specifies where the backup files will be stored.
         */
        static [[nodiscard]] BackupEnginePtr Create(std::filesystem::path aTargetPath, std::filesystem::path aBackupPath,
                                                    std::shared_ptr<spdlog::logger> aLogger = {});

        /*
         * Backup all files from the target directory to the backup directory. Keeping the relative directory structure.
         * If aBackupName is provided, it will be used as the name of the backup directory, default is the current date and time.
         */
        virtual void Backup(const std::optional<std::string>& aBackupName = {}, bool aAppendTimestamp = false) const = 0;

        /*
         * Restore files from the backup directory to the target directory.
         * aBackupName specifies which backup to restore.
         */
        virtual void Restore(const std::string& aBackupName, bool aBackupCurrent = true) const = 0;

        /*
         * GetAvailableBackups returns a list of available backup names.
         */
        virtual [[nodiscard]] std::vector<std::string> GetAvailableBackups() const = 0;

        /*
         * Removes all backups from the backup directory.
         */
        virtual void RemoveAllBackups() const = 0;
        /*
         * SetMaxBackupSize sets the maximum size of a backup in bytes.
         * If the size of the target directory exceeds this limit, an exception will be thrown during backup.
         * Default is 16 MB (16 * 1024 * 1024 bytes).
         */
        virtual void SetMaxBackupSize(size_t aMaxBackupSize) = 0;
    };
}