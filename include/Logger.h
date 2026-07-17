#pragma once

#include <memory>
#include <string>

namespace calculator
{

class Logger
{
  public:
    static Logger& getInstance();

    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

  private:
    Logger();
    ~Logger();

    static void createDirectory(const std::string& path);

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace calculator