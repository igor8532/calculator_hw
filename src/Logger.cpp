#include "Logger.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sys/stat.h>

#include <cerrno>
#include <iostream>

namespace calculator
{

void Logger::createDirectory(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == -1)
    {
        if (mkdir(path.c_str(), 0755) == -1 && errno != EEXIST)
        {
            std::cerr << "Warning: Could not create directory " << path
                      << std::endl;
        }
    }
}

class Logger::Impl
{
  public:
    std::shared_ptr<spdlog::logger> logger;

    Impl()
    {
        Logger::createDirectory("logs");

        try
        {
            std::vector<spdlog::sink_ptr> sinks;

            auto console_sink =
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::info);
            sinks.push_back(console_sink);

            try
            {
                auto file_sink =
                    std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        "logs/calculator.log", 1024 * 1024 * 5, 3);
                file_sink->set_level(spdlog::level::debug);
                sinks.push_back(file_sink);
            }
            catch (const spdlog::spdlog_ex& ex)
            {
                std::cerr << "Warning: File logger disabled: " << ex.what()
                          << std::endl;
            }

            logger = std::make_shared<spdlog::logger>(
                "calculator", sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::debug);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cerr << "Failed to create logger: " << ex.what() << std::endl;
            logger = std::make_shared<spdlog::logger>("null", nullptr);
        }
    }
};

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger() : pImpl(std::make_unique<Impl>())
{}

Logger::~Logger() = default;

void Logger::debug(const std::string& message)
{
    pImpl->logger->debug(message);
}

void Logger::info(const std::string& message)
{
    pImpl->logger->info(message);
}

void Logger::warn(const std::string& message)
{
    pImpl->logger->warn(message);
}

void Logger::error(const std::string& message)
{
    pImpl->logger->error(message);
}

} // namespace calculator