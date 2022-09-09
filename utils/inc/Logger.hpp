#pragma once

#include <string_view>

#include "spdlog/spdlog.h"
#include "Constants.hpp"

namespace utils::logger
{
    void setup(const std::string_view& level);
    void setup(const utils::constants::LogLevels& level);


    template<typename Message, typename... Args >
    void errorMessage(Message&& msg, Args&&... args )
    {
        spdlog::error(std::forward<Message>(msg), std::forward<Args>(args)...);
    }

    template<typename Message, typename... Args >
    void criticalMessage(Message&& msg, Args&&... args )
    {
        spdlog::critical(std::forward<Message>(msg), std::forward<Args>(args)...);
    }

    template<typename Message, typename... Args>
    void warningMessage(Message&& msg, Args&&... args)
    {
        spdlog::warn(std::forward<Message>(msg), std::forward<Args>(args)...);
    }

    template<typename Message, typename... Args>
    void debugMessage(Message& msg, Args&&... args)
    {
        spdlog::debug(std::forward<Message>(msg), std::forward<Args>(args)...);
    }

    template<typename Message, typename... Args>
    void infoMessage(Message&& msg, Args&&... args)
    {
        spdlog::info(std::forward<Message>(msg), std::forward<Args>(args)...);
    }
}