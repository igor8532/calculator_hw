#include "ShutdownCoordinator.h"

namespace calculator
{

void ShutdownCoordinator::requestStop()
{
    std::vector<std::function<void()>> callbacksToRun;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopRequested_)
        {
            return;
        }
        stopRequested_ = true;
        callbacksToRun = callbacks_;
    }

    for (const auto& callback : callbacksToRun)
    {
        callback();
    }

    cv_.notify_all();
}

void ShutdownCoordinator::waitForStop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return stopRequested_.load(); });
}

bool ShutdownCoordinator::stopRequested() const noexcept
{
    return stopRequested_.load();
}

void ShutdownCoordinator::onStop(std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(std::move(callback));
}

} // namespace calculator
