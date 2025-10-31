#include "ConsoleInterface.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

ConsoleInterface* g_console_interface = nullptr;

ConsoleInterface::ConsoleInterface() : running_(false) {
}

ConsoleInterface::~ConsoleInterface() {
    stop();
}

void ConsoleInterface::initialize() {
    print_banner();
    running_ = true;
}

void ConsoleInterface::print_banner() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║  MuEmu ConnectServer Ex097 (Cross)    ║\n";
    std::cout << "║  Cross-Platform Edition                ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void ConsoleInterface::log(Color color, const std::string& message) {
    std::lock_guard<std::mutex> lock(output_mutex_);
    
    // ANSI color codes
    std::cout << "\033[" << static_cast<int>(color) << "m"
              << get_timestamp() << " " << message
              << "\033[0m" << std::endl;
}

void ConsoleInterface::update_status(const std::string& status, size_t queue_size) {
    // Update terminal title (works in most terminals)
    std::cout << "\033]0;ConnectServer - " << status 
              << " (Queue: " << queue_size << ")\007" << std::flush;
}

std::string ConsoleInterface::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "]";
    return ss.str();
}

void ConsoleInterface::start_input_loop() {
    if (input_thread_.joinable()) {
        return; // Already running
    }
    
    input_thread_ = std::thread([this]() {
#ifdef __linux__
        input_pthread_ = pthread_self();
#endif
        std::string command;
        while (running_) {
            std::cout << "> " << std::flush;
            if (std::getline(std::cin, command)) {
                if (!command.empty()) {
                    process_command(command);
                }
            }
            
            if (std::cin.eof() || std::cin.fail()) {
                break;
            }
        }
    });
}

void ConsoleInterface::stop() {
    running_ = false;
    
#ifdef __linux__
    if (input_thread_.joinable()) {
        // Cancel the thread to break out of blocking getline
        pthread_cancel(input_pthread_);
        // Detach to avoid join hanging
        input_thread_.detach();
    }
#else
    if (input_thread_.joinable()) {
        input_thread_.join();
    }
#endif
}

void ConsoleInterface::set_command_handler(std::function<void(const std::string&)> handler) {
    command_handler_ = handler;
}

void ConsoleInterface::process_command(const std::string& cmd) {
    if (cmd == "help" || cmd == "?") {
        show_help();
    } else if (cmd == "status") {
        show_status();
    } else if (cmd == "clear" || cmd == "cls") {
        clear_screen();
    } else if (cmd == "exit" || cmd == "quit") {
        std::cout << "Shutting down server...\n";
        running_ = false;
        // Trigger global shutdown
        extern std::atomic<bool> g_running;
        g_running = false;
    } else {
        // Pass to external handler if set
        if (command_handler_) {
            command_handler_(cmd);
        } else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
}

void ConsoleInterface::show_help() {
    std::cout << "\n╔═══════════ Available Commands ═══════════╗\n";
    std::cout << "║ help, ?          - Show this help        ║\n";
    std::cout << "║ status           - Show server status    ║\n";
    std::cout << "║ reload           - Reload ServerList.dat ║\n";
    std::cout << "║ log tcp_recv on  - Enable TCP recv log  ║\n";
    std::cout << "║ log tcp_recv off - Disable TCP recv log ║\n";
    std::cout << "║ log tcp_send on  - Enable TCP send log  ║\n";
    std::cout << "║ log tcp_send off - Disable TCP send log ║\n";
    std::cout << "║ clear, cls       - Clear screen         ║\n";
    std::cout << "║ exit, quit       - Shutdown server      ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";
}

void ConsoleInterface::show_status() {
    std::cout << "\n╔═══════════ Server Status ═══════════╗\n";
    std::cout << "║ Status:      Running                ║\n";
    std::cout << "║ Mode:        Active                 ║\n";
    std::cout << "║ Queue Size:  0                      ║\n";
    std::cout << "║ Clients:     0                      ║\n";
    std::cout << "╚═════════════════════════════════════╝\n\n";
}

void ConsoleInterface::clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
