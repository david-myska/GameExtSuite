#pragma once

#include <memory>

#include <gtest/gtest.h>

#include "spdlog/spdlog.h"

class GEFixture : public ::testing::Test
{
protected:
    static std::shared_ptr<spdlog::logger> GetConsoleLogger();
};