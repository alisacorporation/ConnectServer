#pragma once

#include <mutex>

// Cross-platform wrapper for critical section/mutex
// Maintains same interface as original Windows CRITICAL_SECTION
class CCriticalSection {
public:
    CCriticalSection();
    ~CCriticalSection();

    void lock();
    void unlock();
    bool try_lock();

private:
    std::recursive_mutex m_mutex;
};
