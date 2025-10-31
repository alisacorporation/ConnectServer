#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>

#define MAX_QUEUE_SIZE 2048

struct QUEUE_INFO {
    uint16_t index;
    uint8_t head;
    uint8_t buff[2048];
    uint32_t size;
};

class CQueue {
public:
    CQueue();
    ~CQueue();

    // Clear all items from queue
    void ClearQueue();

    // Get current queue size
    uint32_t GetQueueSize() const;

    // Add item to queue (returns false if queue is full)
    bool AddToQueue(const QUEUE_INFO& info);

    // Get item from queue (blocks until item available or timeout)
    // timeout_ms: -1 for infinite wait, 0 for no wait, >0 for timeout in milliseconds
    bool GetFromQueue(QUEUE_INFO& info, int timeout_ms = -1);

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<QUEUE_INFO> m_queue;
};
