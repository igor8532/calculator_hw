#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

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

    // Регистрирует callback, который будет вызван один раз при первом
    // requestStop() (из того потока, который его вызвал).
    void onStop(std::function<void()> callback);

  private:
    std::atomic<bool> stopRequested_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::function<void()>> callbacks_;
};

} // namespace calculator
