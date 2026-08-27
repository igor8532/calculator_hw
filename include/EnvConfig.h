#pragma once

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>

namespace calculator
{

inline std::string getEnvOr(const char* name, const std::string& defaultValue)
{
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

inline std::uint16_t getEnvOr(const char* name, std::uint16_t defaultValue)
{
    const char* value = std::getenv(name);
    if (!value)
    {
        return defaultValue;
    }

    try
    {
        return static_cast<std::uint16_t>(std::stoul(value));
    }
    catch (const std::exception&)
    {
        return defaultValue;
    }
}

} // namespace calculator
