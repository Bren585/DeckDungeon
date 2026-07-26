#pragma once
// This RENDER_LOCK definition is used to prevent mmultiple threads from accessing the device context simultaneously.
#include <mutex>
//#define RENDER_LOCK std::lock_guard<std::mutex> lock(render_lock)
#define RENDER_LOCK std::lock_guard<std::recursive_mutex> lock(BLIB::render_lock);
namespace BLIB { extern std::recursive_mutex render_lock; }