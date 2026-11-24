#pragma once
#include <atomic>
namespace n3ds_ui { inline std::atomic<int> inPlay{0}; }
extern "C" inline void n3dsUiPublishInPlay(int on) { n3ds_ui::inPlay.store(on ? 1 : 0, std::memory_order_release); }
extern "C" inline int n3dsUiInPlay(void) { return n3ds_ui::inPlay.load(std::memory_order_acquire); }
