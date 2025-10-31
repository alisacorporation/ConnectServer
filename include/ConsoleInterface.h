#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

#ifdef __linux__
#include <pthread.h>
#endif

enum class Color {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
};

class ConsoleInterface {
public:
    ConsoleInterface();
    ~ConsoleInterface();

    void initialize();
    void log(Color color, const std::string& message);
    void update_status(const std::string& status, size_t queue_size);
    void start_input_loop();
    void stop();

    // Set command handler callback
    void set_command_handler(std::function<void(const std::string&)> handler);

private:
    std::mutex output_mutex_;
    std::atomic<bool> running_;
    std::thread input_thread_;
    std::function<void(const std::string&)> command_handler_;
#ifdef __linux__
    pthread_t input_pthread_;
#endif

    void process_command(const std::string& cmd);
    void show_help();
    void show_status();
    std::string get_timestamp();
    void print_banner();
    void clear_screen();
};

extern ConsoleInterface* g_console_interface;
