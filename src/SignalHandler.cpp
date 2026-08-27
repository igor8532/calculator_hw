#include "SignalHandler.h"

#include "Logger.h"

#include <pthread.h>

#include <stdexcept>

namespace calculator
{

sigset_t SignalHandler::makeSignalSet()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    return set;
}

void SignalHandler::blockSignals()
{
    sigset_t set = makeSignalSet();

    if (pthread_sigmask(SIG_BLOCK, &set, nullptr) != 0)
    {
        Logger::getInstance().error(
            "SignalHandler::blockSignals: pthread_sigmask failed");
        throw std::runtime_error("Failed to block signals");
    }

    Logger::getInstance().info(
        "SignalHandler::blockSignals: SIGINT/SIGTERM blocked");
}

SignalHandler::SignalHandler(ShutdownCoordinator& coordinator) :
    coordinator_(coordinator)
{}

SignalHandler::~SignalHandler()
{
    if (thread_.joinable())
    {
        // Поток может всё ещё ждать в sigwait(), если объект уничтожается
        // из-за исключения при старте приложения, а не из-за настоящего
        // SIGTERM/SIGINT. Без этого join() ниже заблокируется навсегда.
        // SIGTERM здесь не завершает поток/процесс: сигнал заблокирован
        // (blockSignals()) и синхронно вычитывается через sigwait() в
        // этом же потоке, а не через дефолтный обработчик ОС.
        // NOLINTNEXTLINE(bugprone-bad-signal-to-kill-thread)
        pthread_kill(thread_.native_handle(), SIGTERM);
        thread_.join();
    }
}

void SignalHandler::start()
{
    thread_ = std::thread(&SignalHandler::waitLoop, this);
}

void SignalHandler::waitLoop()
{
    auto& logger = Logger::getInstance();
    logger.info("SignalHandler::waitLoop: Waiting for SIGINT/SIGTERM");

    sigset_t set = makeSignalSet();
    int receivedSignal = 0;

    if (sigwait(&set, &receivedSignal) != 0)
    {
        logger.error("SignalHandler::waitLoop: sigwait failed");
        return;
    }

    logger.info("SignalHandler::waitLoop: Received signal " +
                std::to_string(receivedSignal) + ", requesting shutdown");

    coordinator_.requestStop();
}

} // namespace calculator
