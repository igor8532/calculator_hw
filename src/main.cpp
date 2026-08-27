#include "Application.h"
#include "Logger.h"
#include "NetworkServer.h"
#include "ShutdownCoordinator.h"
#include "SignalHandler.h"

#include <cstdlib>
#include <iostream>
#include <memory>
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

        std::thread worker;
        std::unique_ptr<calculator::NetworkServer> server;

        if (argc > 1)
        {
            worker = std::thread([&application, argc, argv]() {
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
        }
        else
        {
            server = std::make_unique<calculator::NetworkServer>(application,
                                                                 coordinator);
            server->start();
            worker = std::thread([&server]() { server->runEventLoop(); });
            logger.info("=== Application listening for network requests ===");
        }

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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
