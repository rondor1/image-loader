#include "Logger.hpp"

#include <stdexcept>

namespace utils::logger
{
    void setup(const std::string_view& level)
    {
        if(level.compare(utils::constants::error) != 0 &&
           level.compare(utils::constants::critical) != 0&&
           level.compare(utils::constants::warning) != 0&&
           level.compare(utils::constants::debug) != 0&&
           level.compare(utils::constants::info) != 0)
        {
            throw std::invalid_argument{"Invalid configuration parameter for logger provided!"};
        }

        if(level.compare(utils::constants::error) == 0)
        {
            spdlog::set_level(spdlog::level::err);
        }
        if(level.compare(utils::constants::critical) == 0)
        {
            spdlog::set_level(spdlog::level::critical);
        }
        if(level.compare(utils::constants::warning) == 0)
        {
            spdlog::set_level(spdlog::level::warn);
        }
        if(level.compare(utils::constants::debug) == 0)
        {
            spdlog::set_level(spdlog::level::debug);
        }
        if(level.compare(utils::constants::info) == 0)
        {
            spdlog::set_level(spdlog::level::info);
        }

    }

    void setup(const utils::constants::LogLevels& level)
    {
        switch (level)
        {
        case utils::constants::LogLevels::error :
            spdlog::set_level(spdlog::level::err);
            break;
        case utils::constants::LogLevels::critical :
            spdlog::set_level(spdlog::level::critical);
            break;
            case utils::constants::LogLevels::warning :
            spdlog::set_level(spdlog::level::warn);
            break;

            case utils::constants::LogLevels::debug :
            spdlog::set_level(spdlog::level::debug);
            break;
            case utils::constants::LogLevels::info :
            spdlog::set_level(spdlog::level::info);
            break;
        default:
            throw std::invalid_argument{"Invalid argument provided for logger setup!"};
            break;
        }
    }
} // namespace imageload::utils::logger
