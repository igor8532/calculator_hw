#include "Application.h"
#include "Logger.h"
#include "ShutdownCoordinator.h"
#include "SignalHandler.h"

#include <iostream>
#include <thread>

int main(int argc, char** argv)
{
    auto& logger = calculator::Logger::getInstance();

    try
    {
        logger.info("=== Application started ===");

        calculator::SignalHandler::blockSignals();

        calculator::ShutdownCoordinator coordinator;
        calculator::SignalHandler signalHandler(coordinator);
        signalHandler.start();

        calculator::Application application;

        std::thread worker([&application, argc, argv]() {
            auto& workerLogger = calculator::Logger::getInstance();
            try
            {
                application.run(argc, argv);
            }
            catch (const std::exception& e)
            {
                workerLogger.error("=== Worker thread crashed: " +
                                   std::string(e.what()) + " ===");
                std::cerr << e.what() << '\n';
            }
        });

        logger.info("=== Application waiting for signals ===");
        coordinator.waitForStop();

        if (worker.joinable())
        {
            worker.join();
        }

        logger.info("=== Application finished successfully ===");
    }
    catch (const std::exception& e)
    {
        logger.error("=== Application crashed: " + std::string(e.what()) +
                     " ===");
        std::cerr << e.what() << '\n';
    }
}
