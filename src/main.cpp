#include "ConfigManager.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "TimerManager.h"
#include "ConsoleInterface.h"
#include "Console.h"
#include "IpManager.h"
#include "ServerList.h"
#include "Util.h"
#include "Version.h"

#ifdef __linux__
#include "platform/linux/SignalHandler.h"
#elif defined(_WIN32)
#include "platform/windows/CrashHandler.h"
#endif

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <csignal>

// Global io_context
boost::asio::io_context* g_io_context = nullptr;
std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    std::cout << "\nShutdown signal received..." << std::endl;
    g_running = false;
    if (g_io_context) {
        g_io_context->stop();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== ConnectServer Cross-Platform Edition ===" << std::endl;
    std::cout << "Version: " << Version::GetVersion() << std::endl;
    std::cout << "Build: " << Version::GetBuildNumber() << std::endl;
    std::cout << "Date: " << Version::GetBuildDate() << " " << Version::GetBuildTime() << std::endl;
    std::cout << std::endl;

    // Install crash handler
#ifdef __linux__
    SignalHandler::install();
    std::cout << "[OK] Linux signal handler installed" << std::endl;
#elif defined(_WIN32)
    CMiniDump::Start();
    std::cout << "[OK] Windows crash handler installed" << std::endl;
#endif

    // Install signal handlers for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Load configuration
    // Log build info
    LogAdd(0, "=== ConnectServer Cross-Platform Edition ===");
    LogAdd(0, "Version: %s", Version::GetVersion());
    LogAdd(0, "Build: %s", Version::GetBuildNumber());
    LogAdd(0, "Date: %s %s", Version::GetBuildDate(), Version::GetBuildTime());
    LogAdd(0, "");

    std::cout << "\n--- Loading Configuration ---" << std::endl;
    ConfigManager config;
    
    if (!config.load("ConnectServer.ini")) {
        std::cout << "[WARN] Configuration file not found, using defaults" << std::endl;
    }
    
    int tcp_port = config.get_int("ConnectServerInfo", "ConnectServerPortTCP", 44405);
    int udp_port = config.get_int("ConnectServerInfo", "ConnectServerPortUDP", 55557);
    MaxIpConnection = config.get_int("ConnectServerInfo", "MaxIpConnection", 0);
    
    std::cout << "  TCP Port: " << tcp_port << std::endl;
    std::cout << "  UDP Port: " << udp_port << std::endl;
    std::cout << "  Max IP Connection: " << MaxIpConnection << std::endl;

    // Load ServerList
    std::cout << "\n--- Loading ServerList ---" << std::endl;
    gServerList.Load("ServerList.dat");

    // Initialize console interface
    std::cout << "\n--- Initializing Console ---" << std::endl;
    ConsoleInterface console;
    g_console_interface = &console;
    console.initialize();

    // Create io_context
    boost::asio::io_context io_context;
    g_io_context = &io_context;

    // Create work guard to keep io_context running
    auto work_guard = boost::asio::make_work_guard(io_context);

    // Create managers
    std::cout << "\n--- Creating Network Managers ---" << std::endl;
    
    SocketManager socket_manager(io_context);
    g_socket_manager = &socket_manager;
    
    SocketManagerUdp socket_manager_udp(io_context);
    g_socket_manager_udp = &socket_manager_udp;
    
    TimerManager timer_manager(io_context);
    g_timer_manager = &timer_manager;

    // Start TCP server
    std::cout << "\n--- Starting TCP Server ---" << std::endl;
    if (!socket_manager.start(tcp_port)) {
        std::cerr << "[ERROR] Failed to start TCP server" << std::endl;
        return 1;
    }
    console.log(Color::GREEN, "TCP server started on port " + std::to_string(tcp_port));

    // Start UDP server
    std::cout << "\n--- Starting UDP Server ---" << std::endl;
    if (!socket_manager_udp.start(udp_port)) {
        std::cerr << "[ERROR] Failed to start UDP server" << std::endl;
        return 1;
    }
    console.log(Color::GREEN, "UDP server started on port " + std::to_string(udp_port));

    // Set up timer callbacks
    timer_manager.set_1s_callback([]() {
        // 1-second timer - ServerList maintenance
        gServerList.MainProc();
    });

    timer_manager.set_5s_callback([]() {
        // 5-second timer - timeout checks
        // TODO: Implement client timeout checking
    });

    // Start timers
    std::cout << "\n--- Starting Timers ---" << std::endl;
    timer_manager.start();
    console.log(Color::GREEN, "Timers started");

    // Create worker threads
    std::cout << "\n--- Creating Worker Threads ---" << std::endl;
    unsigned int thread_count = std::min(std::thread::hardware_concurrency(), 8u);
    if (thread_count == 0) thread_count = 4;
    
    std::cout << "  Worker threads: " << thread_count << std::endl;
    
    std::vector<std::thread> worker_threads;
    for (unsigned int i = 0; i < thread_count; ++i) {
        worker_threads.emplace_back([&io_context]() {
            io_context.run();
        });
    }

    console.log(Color::GREEN, "Server is running!");
    console.log(Color::YELLOW, "Press Ctrl+C to shutdown");
    std::cout << "\n=== Phase 2 Test Complete - Server Running ===" << std::endl;
    std::cout << std::endl;

    // Set up console command handler
    console.set_command_handler([&](const std::string& cmd) {
        if (cmd.find("reload") == 0) {
            console.log(Color::YELLOW, "Reload command (will be implemented in Phase 3)");
        } else if (cmd.find("log") == 0) {
            // Parse log commands
            if (cmd.find("tcp_recv on") != std::string::npos) {
                gConsole.EnableOutput[CON_PROTO_TCP_RECV] = true;
                console.log(Color::GREEN, "TCP receive logging enabled");
            } else if (cmd.find("tcp_recv off") != std::string::npos) {
                gConsole.EnableOutput[CON_PROTO_TCP_RECV] = false;
                console.log(Color::GREEN, "TCP receive logging disabled");
            } else if (cmd.find("tcp_send on") != std::string::npos) {
                gConsole.EnableOutput[CON_PROTO_TCP_SEND] = true;
                console.log(Color::GREEN, "TCP send logging enabled");
            } else if (cmd.find("tcp_send off") != std::string::npos) {
                gConsole.EnableOutput[CON_PROTO_TCP_SEND] = false;
                console.log(Color::GREEN, "TCP send logging disabled");
            }
        }
    });

    // Start console input loop
    console.start_input_loop();

    // Wait for shutdown signal
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Shutdown
    std::cout << "\n--- Shutting Down ---" << std::endl;
    console.log(Color::YELLOW, "Shutting down server...");

    timer_manager.stop();
    socket_manager.stop();
    socket_manager_udp.stop();

    work_guard.reset();
    io_context.stop();

    // Wait for worker threads
    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    console.stop();

    // Cleanup
#ifdef __linux__
    SignalHandler::cleanup();
#elif defined(_WIN32)
    CMiniDump::Clean();
#endif

    std::cout << "\nShutdown complete." << std::endl;
    return 0;
}
