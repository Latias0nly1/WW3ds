#pragma once
#include <atomic>
namespace n3ds_warm {
inline std::atomic<bool> g_lockControls{false};
inline bool blockingInput() { return g_lockControls.load(std::memory_order_acquire); }
}
