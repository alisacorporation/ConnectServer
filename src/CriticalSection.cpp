#include "CriticalSection.h"

CCriticalSection::CCriticalSection() {
    // std::recursive_mutex constructor handles initialization
}

CCriticalSection::~CCriticalSection() {
    // std::recursive_mutex destructor handles cleanup
}

void CCriticalSection::lock() {
    m_mutex.lock();
}

void CCriticalSection::unlock() {
    m_mutex.unlock();
}

bool CCriticalSection::try_lock() {
    return m_mutex.try_lock();
}
