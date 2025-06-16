#pragma once

#include "game_enhancer/impl/backup/backup_engine.h"

#include <chrono>
#include <format>

namespace fs = std::filesystem;

namespace GE
{
    std::string CurrentTimestamp()
    {
        const auto now = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
        return std::format("{:%Y-%m-%d_%H-%M-%S}", now);
    }

    void BackupEngineImpl::CheckTargetDirectorySize() const
    {
        size_t size = 0;
        for (const auto& entry : fs::recursive_directory_iterator(m_targetPath, fs::directory_options::skip_permission_denied))
        {
            if (entry.is_regular_file())
            {
                std::error_code ec;
                size += fs::file_size(entry.path(), ec);
                if (size > m_maxBackupSize)
                {
                    throw std::runtime_error(std::format("Directory exceeds maximum backup size ({} bytes)", m_maxBackupSize));
                }
            }
        }
    }

    BackupEngineImpl::BackupEngineImpl(fs::path aTargetPath, fs::path aBackupPath, std::shared_ptr<spdlog::logger> aLogger)
        : m_targetPath(std::move(aTargetPath))
        , m_backupPath(std::move(aBackupPath))
        , m_logger(std::move(aLogger))
    {
        if (!fs::exists(m_backupPath))
        {
            m_logger->info("Backup path does not exist, creating: {}", m_backupPath.string());
            fs::create_directories(m_backupPath);
        }
    }

    void BackupEngineImpl::Backup(const std::optional<std::string>& aBackupName) const
    {
        const fs::path dest = m_backupPath / aBackupName.value_or(CurrentTimestamp());

        if (!fs::exists(m_targetPath))
        {
            throw std::runtime_error("Target path does not exist: " + m_targetPath.string());
        }

        CheckTargetDirectorySize();

        if (fs::exists(dest))
        {
            throw std::runtime_error("Backup name already exists.");
        }

        m_logger->info("Creating backup: {}", dest.string());
        fs::create_directories(dest);
        fs::copy(m_targetPath, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    void BackupEngineImpl::Restore(const std::string& aBackupName, bool aBackupCurrent) const
    {
        const fs::path source = m_backupPath / aBackupName / m_targetPath.filename();

        if (!fs::exists(source))
        {
            throw std::runtime_error("Backup does not exist.");
        }

        if (!fs::exists(m_targetPath))
        {
            throw std::runtime_error("Target path does not exist: " + m_targetPath.string());
        }

        constexpr auto backupCurrentName = "auto-pre-restore-backup";
        if (aBackupCurrent && aBackupName != backupCurrentName)
        {
            Backup(backupCurrentName);
        }

        m_logger->info("Restoring backup: {}", source.string());
        fs::remove_all(m_targetPath);
        fs::copy(source, m_targetPath.parent_path(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    std::vector<std::string> BackupEngineImpl::GetAvailableBackups() const
    {
        std::vector<std::string> backups;

        if (!fs::exists(m_backupPath))
        {
            return backups;
        }

        for (const auto& entry : fs::directory_iterator(m_backupPath))
        {
            if (entry.is_directory())
            {
                backups.emplace_back(entry.path().filename().string());
            }
        }

        std::ranges::sort(backups);
        return backups;
    }

    void BackupEngineImpl::SetMaxBackupSize(size_t aMaxBackupSize)
    {
        m_logger->info("Setting maximum backup size to {} bytes", aMaxBackupSize);
        m_maxBackupSize = aMaxBackupSize;
    }

    BackupEnginePtr BackupEngine::Create(fs::path aTargetPath, fs::path aBackupPath, std::shared_ptr<spdlog::logger> aLogger)
    {
        if (!aLogger)
        {
            aLogger = std::make_shared<spdlog::logger>("empty");
        }
        if (!fs::exists(aTargetPath) || !fs::is_directory(aTargetPath))
        {
            throw std::invalid_argument("Target path does not exist or is not a directory: " + aTargetPath.string());
        }
        return std::make_shared<BackupEngineImpl>(std::move(aTargetPath), std::move(aBackupPath), aLogger);
    }
}