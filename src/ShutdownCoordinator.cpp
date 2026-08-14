#include "ShutdownCoordinator.h"

namespace calculator
{

void ShutdownCoordinator::requestStop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopRequested_ = true;
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

} // namespace calculator
