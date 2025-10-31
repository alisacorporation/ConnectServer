#include "Queue.h"
#include <chrono>

CQueue::CQueue() {
}

CQueue::~CQueue() {
}

void CQueue::ClearQueue() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clear the queue by swapping with empty queue
    std::queue<QUEUE_INFO> empty;
    std::swap(m_queue, empty);
}

uint32_t CQueue::GetQueueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<uint32_t>(m_queue.size());
}

bool CQueue::AddToQueue(const QUEUE_INFO& info) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if queue is full
    if (m_queue.size() >= MAX_QUEUE_SIZE) {
        return false;
    }
    
    // Add to queue
    m_queue.push(info);
    
    // Notify one waiting thread
    m_cv.notify_one();
    
    return true;
}

bool CQueue::GetFromQueue(QUEUE_INFO& info, int timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (timeout_ms < 0) {
        // Infinite wait
        m_cv.wait(lock, [this] { return !m_queue.empty(); });
    } else if (timeout_ms == 0) {
        // No wait - check immediately
        if (m_queue.empty()) {
            return false;
        }
    } else {
        // Wait with timeout
        if (!m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                          [this] { return !m_queue.empty(); })) {
            return false; // Timeout
        }
    }
    
    // Get item from queue
    info = m_queue.front();
    m_queue.pop();
    
    return true;
}
