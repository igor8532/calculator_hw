#include "SignalHandler.h"

#include "Logger.h"

#include <unistd.h>

#include <stdexcept>

namespace calculator
{

volatile sig_atomic_t SignalHandler::running_ = 1;

void SignalHandler::setup()
{
    auto& logger = Logger::getInstance();

    struct sigaction action{};

    action.sa_handler = handleSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, nullptr) == -1)
    {
        logger.error("SignalHandler::setup: Failed to register SIGINT");
        throw std::runtime_error("Failed to register SIGINT handler");
    }

    if (sigaction(SIGTERM, &action, nullptr) == -1)
    {
        logger.error("SignalHandler::setup: Failed to register SIGTERM");
        throw std::runtime_error("Failed to register SIGTERM handler");
    }

    logger.info("SignalHandler::setup: Signal handlers registered");
}

void SignalHandler::wait()
{
    auto& logger = Logger::getInstance();

    logger.info("SignalHandler::wait: Waiting for signals");

    while (running_)
    {
        pause();
    }

    logger.info("SignalHandler::wait: Signal received, shutting down");
}

void SignalHandler::handleSignal(int signal)
{
    if (signal == SIGINT)
    {
        write(STDOUT_FILENO, "\nSIGINT received\n", 17);
    }
    else if (signal == SIGTERM)
    {
        write(STDOUT_FILENO, "\nSIGTERM received\n", 18);
    }

    running_ = 0;
}

} // namespace calculator