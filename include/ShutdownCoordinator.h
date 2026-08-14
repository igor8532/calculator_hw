#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace calculator
{

class ShutdownCoordinator
{
  public:
    ShutdownCoordinator() = default;

    ShutdownCoordinator(const ShutdownCoordinator&) = delete;
    ShutdownCoordinator& operator=(const ShutdownCoordinator&) = delete;

    void requestStop();
    void waitForStop();
    bool stopRequested() const noexcept;

  private:
    std::atomic<bool> stopRequested_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace calculator
