#pragma once

#include <csignal>

namespace calculator
{

class SignalHandler
{
  public:
    static void setup();
    static void wait();

  private:
    static void handleSignal(int signal);

    static volatile sig_atomic_t running_;
};

} // namespace calculator