#include "ge_fixture.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> GEFixture::GetConsoleLogger()
{
    static std::shared_ptr<spdlog::logger> s_logger = spdlog::stdout_color_mt("console");
    return s_logger;
}
