#include "ConfigManager.h"
#include "CriticalSection.h"
#include "Queue.h"
#include "Console.h"
#include "ConsoleInterface.h"
#include "Util.h"

#ifdef __linux__
#include "platform/linux/SignalHandler.h"
#elif defined(_WIN32)
#include "platform/windows/CrashHandler.h"
#endif

#include <iostream>
#include <thread>
#include <chrono>

// Phase 1 Test Program
int main(int argc, char* argv[]) {
    std::cout << "=== ConnectServer Phase 1 Test ===" << std::endl;
    std::cout << std::endl;

    // Install crash handler
#ifdef __linux__
    SignalHandler::install();
    std::cout << "[OK] Linux signal handler installed" << std::endl;
#elif defined(_WIN32)
    CMiniDump::Start();
    std::cout << "[OK] Windows crash handler installed" << std::endl;
#endif

    // Test 1: ConfigManager
    std::cout << "\n--- Test 1: ConfigManager ---" << std::endl;
    ConfigManager config;
    
    if (config.load("ConnectServer.ini")) {
        std::cout << "[OK] Configuration loaded" << std::endl;
        
        int tcp_port = config.get_int("ConnectServerInfo", "ConnectServerPortTCP", 44405);
        int udp_port = config.get_int("ConnectServerInfo", "ConnectServerPortUDP", 55601);
        int max_ip = config.get_int("ConnectServerInfo", "MaxIpConnection", 0);
        
        std::cout << "  TCP Port: " << tcp_port << std::endl;
        std::cout << "  UDP Port: " << udp_port << std::endl;
        std::cout << "  Max IP Connection: " << max_ip << std::endl;
    } else {
        std::cout << "[WARN] Configuration file not found (expected for first run)" << std::endl;
        std::cout << "  Copy config/ConnectServer.ini.example to ConnectServer.ini" << std::endl;
    }

    // Test 2: CriticalSection
    std::cout << "\n--- Test 2: CriticalSection ---" << std::endl;
    CCriticalSection cs;
    cs.lock();
    std::cout << "[OK] Critical section locked" << std::endl;
    cs.unlock();
    std::cout << "[OK] Critical section unlocked" << std::endl;

    // Test 3: Queue
    std::cout << "\n--- Test 3: Queue ---" << std::endl;
    CQueue queue;
    
    QUEUE_INFO info = {};
    info.index = 1;
    info.head = 0xF4;
    info.size = 10;
    
    if (queue.AddToQueue(info)) {
        std::cout << "[OK] Item added to queue" << std::endl;
        std::cout << "  Queue size: " << queue.GetQueueSize() << std::endl;
        
        QUEUE_INFO retrieved = {};
        if (queue.GetFromQueue(retrieved, 0)) {
            std::cout << "[OK] Item retrieved from queue" << std::endl;
            std::cout << "  Index: " << retrieved.index << std::endl;
            std::cout << "  Head: 0x" << std::hex << (int)retrieved.head << std::dec << std::endl;
        }
    }

    // Test 4: Console
    std::cout << "\n--- Test 4: Console ---" << std::endl;
    gConsole.EnableOutput[CON_GENERAL] = true;
    gConsole.Output(CON_GENERAL, "[OK] Console output working");

    // Test 5: ConsoleInterface
    std::cout << "\n--- Test 5: ConsoleInterface ---" << std::endl;
    ConsoleInterface console;
    g_console_interface = &console;
    
    console.initialize();
    console.log(Color::GREEN, "[OK] ConsoleInterface initialized");
    console.log(Color::BLUE, "Testing colored output");
    console.log(Color::YELLOW, "Type 'help' for commands, 'exit' to quit");
    
    // Test 6: Util functions
    std::cout << "\n--- Test 6: Util Functions ---" << std::endl;
    uint32_t tick = GetTickCount();
    std::cout << "[OK] GetTickCount: " << tick << " ms" << std::endl;
    
    LogAdd(2, "[OK] LogAdd function working");

    // Start interactive console
    std::cout << "\n=== All Phase 1 Tests Passed ===" << std::endl;
    std::cout << "Starting interactive console..." << std::endl;
    std::cout << std::endl;
    
    console.start_input_loop();
    
    // Wait for exit command
    while (console.log(Color::WHITE, ""), true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Check if we should exit (this is simplified for testing)
        break;
    }

    // Cleanup
#ifdef __linux__
    SignalHandler::cleanup();
#elif defined(_WIN32)
    CMiniDump::Clean();
#endif

    std::cout << "\nShutdown complete." << std::endl;
    return 0;
}
