#pragma once

#include <boost/asio.hpp>
#include <atomic>
#include <functional>

class TimerManager {
public:
    TimerManager(boost::asio::io_context& io);
    ~TimerManager();

    void start();
    void stop();
    
    // Set callback for 1-second timer
    void set_1s_callback(std::function<void()> callback);
    
    // Set callback for 5-second timer
    void set_5s_callback(std::function<void()> callback);

private:
    void schedule_1s_timer();
    void schedule_5s_timer();

    boost::asio::io_context& io_context_;
    boost::asio::steady_timer timer_1s_;
    boost::asio::steady_timer timer_5s_;
    
    std::atomic<bool> running_;
    
    std::function<void()> callback_1s_;
    std::function<void()> callback_5s_;
};

extern TimerManager* g_timer_manager;
