#pragma once

#include "ShutdownCoordinator.h"

#include <csignal>
#include <thread>

namespace calculator
{

class SignalHandler
{
  public:
    explicit SignalHandler(ShutdownCoordinator& coordinator);
    ~SignalHandler();

    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;

    // Вызывать в main-потоке ДО создания любых std::thread —
    // маска сигналов наследуется потоками на момент их создания.
    static void blockSignals();

    void start();

  private:
    void waitLoop();

    static sigset_t makeSignalSet();

    ShutdownCoordinator& coordinator_;
    std::thread thread_;
};

} // namespace calculator
