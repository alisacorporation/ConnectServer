#include "TimerManager.h"
#include "Util.h"

TimerManager* g_timer_manager = nullptr;

TimerManager::TimerManager(boost::asio::io_context& io)
    : io_context_(io)
    , timer_1s_(io)
    , timer_5s_(io)
    , running_(false)
{
}

TimerManager::~TimerManager() {
    stop();
}

void TimerManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    LogAdd(2, "[TimerManager] Timers started");
    
    schedule_1s_timer();
    schedule_5s_timer();
}

void TimerManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    timer_1s_.cancel();
    timer_5s_.cancel();
    
    LogAdd(2, "[TimerManager] Timers stopped");
}

void TimerManager::set_1s_callback(std::function<void()> callback) {
    callback_1s_ = callback;
}

void TimerManager::set_5s_callback(std::function<void()> callback) {
    callback_5s_ = callback;
}

void TimerManager::schedule_1s_timer() {
    if (!running_) {
        return;
    }
    
    timer_1s_.expires_after(std::chrono::seconds(1));
    timer_1s_.async_wait([this](const boost::system::error_code& error) {
        if (error) {
            if (error != boost::asio::error::operation_aborted) {
                LogAdd(1, "[TimerManager] 1s timer error: %s", error.message().c_str());
            }
            return;
        }
        
        // Execute callback
        if (callback_1s_) {
            callback_1s_();
        }
        
        // Reschedule
        schedule_1s_timer();
    });
}

void TimerManager::schedule_5s_timer() {
    if (!running_) {
        return;
    }
    
    timer_5s_.expires_after(std::chrono::seconds(5));
    timer_5s_.async_wait([this](const boost::system::error_code& error) {
        if (error) {
            if (error != boost::asio::error::operation_aborted) {
                LogAdd(1, "[TimerManager] 5s timer error: %s", error.message().c_str());
            }
            return;
        }
        
        // Execute callback
        if (callback_5s_) {
            callback_5s_();
        }
        
        // Reschedule
        schedule_5s_timer();
    });
}
