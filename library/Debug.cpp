/**
Copyright © 2018 Pauli <suokkos@gmail.com>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
   not claim that you wrote the original software. If you use this
   software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
 */
#define _POSIX_C_SOURCE 200809L
#include "Core.h"

#include "Debug.h"
#include "DebugManager.h"

//#include <chrono>
#include <iomanip>
//#include <thread>
#include "time.h"

#ifdef _MSC_VER
static tm* localtime_r(const time_t* time, tm* result)
{
    localtime_s(result, time);
    return result;
}
#endif

namespace DFHack {
DBG_DECLARE(core, debug);

//namespace debug { namespace plugin {
//DebugCategory debug_debug("core", "debug");
//DebugRegister<&debug_debug> register_debug;
//}}
//using debug::plugin::debug_debug;

void DebugManager::registerCategory(DebugCategory& cat)
{
    //DEBUG(debug) << "register DebugCategory '" << cat.category()
    //    << "' from '" << cat.plugin()
    //    << "' allowed " << int32(cat.allowed()) << "\n";
    OUT_DEBUG_DEFAULT(debug, "register DebugCategory '%s' from '%s' allowed '%i'\n",
        cat.category(), cat.plugin(), int32(cat.allowed()));
    tthread::lock_guard<tthread::mutex> guard(access_mutex_);
    push_back(&cat);
    //categorySignal(CAT_ADD, cat);
}

void DebugManager::unregisterCategory(DebugCategory& cat)
{
    //DEBUG(debug) << "unregister DebugCategory '" << cat.category()
    //    << "' from '" << cat.plugin()
    //    << "' allowed " << int32(cat.allowed()) << "\n";
    OUT_DEBUG_DEFAULT(debug, "unregister DebugCategory '%s' from '%s' allowed '%i'\n",
        cat.category(), cat.plugin(), int32(cat.allowed()));
    tthread::lock_guard<tthread::mutex> guard(access_mutex_);
    iterator iter = std::find(begin(), end(), &cat);
    //std::swap(*iter, back());
    if (iter == end())
        return;
    std::swap(*iter, *(end()-1));
    pop_back();
    //categorySignal(CAT_REMOVE, cat);
}

DebugRegisterBase::DebugRegisterBase(DebugCategory* cat)
{
    // Make sure Core lives at least as long any DebugCategory to
    // allow debug prints until all Debugcategories has been destructed
    Core::getInstance();
    DebugManager::getInstance().registerCategory(*cat);
}

void DebugRegisterBase::unregister(DebugCategory* cat)
{
    DebugManager::getInstance().unregisterCategory(*cat);
}

static color_value selectColor(const DebugCategory::level msgLevel)
{
    switch(msgLevel)
    {
    case DebugCategory::LTRACE:
        return COLOR_GREY;
    case DebugCategory::LDEBUG:
        return COLOR_LIGHTBLUE;
    case DebugCategory::LINFO:
        return COLOR_CYAN;
    case DebugCategory::LWARNING:
        return COLOR_YELLOW;
    case DebugCategory::LERROR:
        return COLOR_LIGHTRED;
    }
    return COLOR_WHITE;
}

#if __GNUC__
// Allow gcc to optimize tls access. It also makes sure initialized is done as
// early as possible. The early initialization helps to give threads same ids as
// gdb shows.
#define EXEC_ATTR __attribute__((tls_model("initial-exec")))
#else
#define EXEC_ATTR
#endif

namespace {
//static std::atomic<uint32_t> nextId{0};
//static EXEC_ATTR thread_local uint32_t thread_id{nextId.fetch_add(1)+1};
//static tthread::atomic<uint32_t> nextId(1);
//static EXEC_ATTR thread_local uint32_t thread_id = 1;
#define this_thread_id tthread::this_thread::get_id().get()
}

DebugCategory::ostream_proxy_prefix::ostream_proxy_prefix(
    const DebugCategory& cat, color_ostream& target, const DebugCategory::level msgLevel) :
    color_ostream_proxy(target)
{
    color(selectColor(msgLevel));
    //auto now = std::chrono::system_clock::now();
    time_t ms = 0;
    // Output time in format %02H:%02M:%02S.%03ms
    char buffer[32];
    time_t now_c;
    tm local;
    time(&now_c);
    size_t sz = strftime(buffer, sizeof(buffer), "%H:%M:%S.", localtime_r(&now_c, &local));
    *this << (sz > 0 ? buffer : "HH:MM:SS.")
        << std::setfill('0') << std::setw(3) << uint32(ms)
        // Thread id is allocated in the thread creation order to a thread_local
        // variable
        //<< ":t" << thread_id
        << ":t" << this_thread_id
        // Output plugin and category names to make it easier to locate where
        // the message is coming. It would be easy replaces these with __FILE__
        // and __LINE__ passed from the macro if that would be preferred prefix.
        << ':' << cat.plugin() << ':' << cat.category() << ": ";
}


DebugCategory::level DebugCategory::allowed() const
{
    return DebugCategory::level(allowed_.load(tthread::memory_order_relaxed));
}

void DebugCategory::allowed(DebugCategory::level value)
{
    level old = DebugCategory::level(allowed_.exchange(uint32(value), tthread::memory_order_relaxed));
    if (old == value)
        return;
    //TRACE(debug) << "modify DebugCategory '" << category()
    //    << "' from '" << plugin()
    //    << "' allowed " << int32(value) << "\n";
    OUT_TRACE_DEFAULT(debug, "modify DebugCategory '%s' from '%s' allowed '%i'\n",
        category(), plugin(), int32(value));
    //DebugManager& manager = DebugManager::getInstance();
    //manager.categorySignal(DebugManager::CAT_MODIFIED, *this);
}

DebugCategory::cstring_ref DebugCategory::category() const
{
    return category_;
}

DebugCategory::cstring_ref DebugCategory::plugin() const
{
    return plugin_;
}

/*
#if __cplusplus < 201703L && __cpp_lib_atomic_is_always_lock_free < 201603
//! C++17 has std::atomic::is_always_lock_free for static_assert. Older
//! standards only provide runtime checks if an atomic type is lock free
struct failIfEnumAtomicIsNotLockFree
{
    failIfEnumAtomicIsNotLockFree()
    {
        tthread::atomic<uint32> test;
        if (test.is_lock_free())
            return;
        std::cerr << __FILE__ << ':' << __LINE__
            << ": error: std::atomic<DebugCategory::level> should be lock free. Your compiler reports the atomic requires runtime locks. Either you are using a very old CPU or we need to change code to use integer atomic type." << "\n";
        std::abort();
    }
} failIfEnumAtomicIsNotLockFree;
#endif
*/

}
