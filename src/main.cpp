#include "Application.h"
#include "Logger.h"

#include <iostream>

int main(int argc, char** argv)
{
    auto& logger = calculator::Logger::getInstance();

    try
    {
        logger.info("=== Application started ===");
        calculator::Application application;
        application.run(argc, argv);
        logger.info("=== Application finished successfully ===");
    }
    catch (const std::exception& e)
    {
        logger.error("=== Application crashed: " + std::string(e.what()) +
                     " ===");
        std::cerr << e.what() << '\n';
    }
}