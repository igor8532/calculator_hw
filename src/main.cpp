#include "Application.h"
#include "Logger.h"
#include "SignalHandler.h"

#include <iostream>

int main(int argc, char** argv)
{
    auto& logger = calculator::Logger::getInstance();

    try
    {
        logger.info("=== Application started ===");

        calculator::SignalHandler::setup();

        calculator::Application application;
        application.run(argc, argv);
        logger.info("=== Application waiting for signals ===");

        calculator::SignalHandler::wait();

        logger.info("=== Application finished successfully ===");
    }
    catch (const std::exception& e)
    {
        logger.error("=== Application crashed: " + std::string(e.what()) +
                     " ===");
        std::cerr << e.what() << '\n';
    }
}