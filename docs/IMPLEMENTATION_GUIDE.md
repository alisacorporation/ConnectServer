# ConnectServer Implementation Guide

## Getting Started

This guide provides step-by-step instructions for implementing the cross-platform ConnectServer.

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libboost-all-dev
sudo apt install libncurses-dev  # Optional, for TUI
```

### Linux (CentOS/RHEL)
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
sudo yum install boost-devel
sudo yum install ncurses-devel  # Optional, for TUI
```

### Windows
- Visual Studio 2019 or later with C++ workload
- CMake 3.15+
- Boost 1.70+ (install via vcpkg or download from boost.org)

```powershell
# Using vcpkg
vcpkg install boost:x64-windows
```

## Phase 1: Foundation (Week 1-2)

### Step 1.1: Set Up Project Structure

All directories are already created. Verify structure:
```bash
cd ConnectServerCross
tree -L 2
```

### Step 1.2: Implement ConfigManager

**File**: `include/ConfigManager.h`
```cpp
#pragma once
#include <string>
#include <map>

class ConfigManager {
public:
    bool load(const std::string& filename);
    int get_int(const std::string& section, const std::string& key, int default_value) const;
    std::string get_string(const std::string& section, const std::string& key, 
                          const std::string& default_value) const;
    
private:
    std::map<std::string, std::map<std::string, std::string>> config_;
    void trim(std::string& str);
};
```

**File**: `src/ConfigManager.cpp`
- Implement INI file parsing
- Handle sections, key=value pairs
- Support comments (;  and #)
- Trim whitespace

### Step 1.3: Implement CriticalSection

**File**: `include/CriticalSection.h`
```cpp
#pragma once
#include <mutex>

class CCriticalSection {
public:
    void lock();
    void unlock();
    bool try_lock();
    
private:
    std::recursive_mutex m_mutex;
};
```

**File**: `src/CriticalSection.cpp`
- Wrap std::recursive_mutex
- Maintain same interface as original

### Step 1.4: Implement Queue

**File**: `include/Queue.h`
```cpp
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ProtocolDefines.h"

#define MAX_QUEUE_SIZE 2048

struct QUEUE_INFO {
    uint16_t index;
    uint8_t head;
    uint8_t buff[2048];
    uint32_t size;
};

class CQueue {
public:
    void clear_queue();
    uint32_t get_queue_size();
    bool add_to_queue(const QUEUE_INFO& info);
    bool get_from_queue(QUEUE_INFO& info, int timeout_ms = -1);
    
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<QUEUE_INFO> m_queue;
};
```

**File**: `src/Queue.cpp`
- Thread-safe queue implementation
- Use condition_variable for blocking
- Support timeout on get

### Step 1.5: Implement ConsoleInterface

**File**: `include/ConsoleInterface.h`
```cpp
#pragma once
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

enum class Color {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
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
    
private:
    std::mutex output_mutex_;
    std::atomic<bool> running_;
    std::thread input_thread_;
    
    void process_command(const std::string& cmd);
    void show_help();
    void show_status();
    std::string get_timestamp();
};
```

**File**: `src/ConsoleInterface.cpp`
- ANSI color support
- Command processing
- Status display
- Input handling

### Step 1.6: Implement Util Functions

**File**: `include/Util.h`
```cpp
#pragma once
#include <cstdint>
#include <chrono>
#include "ServerDisplayer.h"

void error_message_box(const char* message, ...);
void log_add(LogColor color, const char* text, ...);
void console_protocol_log(int type, const uint8_t* msg, int size);
void connect_server_timeout_proc();
int get_free_client_index();

// Time functions
inline uint32_t get_tick_count() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return static_cast<uint32_t>(ms.count());
}

extern int gClientCount;
```

**File**: `src/Util.cpp`
- Implement utility functions
- Replace GetTickCount with chrono
- Port logging functions

### Step 1.7: Implement Platform-Specific Crash Handlers

**Linux**: `src/platform/linux/SignalHandler.cpp`
```cpp
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <ctime>
#include <cstdio>

class SignalHandler {
public:
    static void install() {
        signal(SIGSEGV, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGFPE, signal_handler);
        signal(SIGILL, signal_handler);
    }
    
private:
    static void signal_handler(int sig) {
        // Backtrace and log
    }
};
```

**Windows**: `src/platform/windows/CrashHandler.cpp`
- Keep original MiniDump implementation

### Step 1.8: Test Phase 1

Create simple test program:
```cpp
// test_phase1.cpp
#include "ConfigManager.h"
#include "CriticalSection.h"
#include "Queue.h"
#include "ConsoleInterface.h"

int main() {
    // Test ConfigManager
    ConfigManager config;
    config.load("test.ini");
    
    // Test Queue
    CQueue queue;
    QUEUE_INFO info = {};
    queue.add_to_queue(info);
    queue.get_from_queue(info);
    
    // Test Console
    ConsoleInterface console;
    console.initialize();
    console.log(Color::GREEN, "Phase 1 test successful!");
    
    return 0;
}
```

## Phase 2: Networking (Week 3-4)

### Step 2.1: Implement ClientSession

**File**: `include/ClientSession.h`
```cpp
#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <queue>
#include <mutex>
#include <chrono>

constexpr size_t MAX_PACKET_SIZE = 2048;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(boost::asio::io_context& io, int index);
    
    void start();
    void async_send(const uint8_t* data, size_t size);
    void close();
    
    boost::asio::ip::tcp::socket& socket() { return socket_; }
    int index() const { return index_; }
    bool is_connected() const;
    bool check_timeout() const;
    
private:
    void start_read();
    void handle_read(const boost::system::error_code& error, size_t bytes);
    bool parse_packets();
    void process_packet(uint8_t head, const uint8_t* data, size_t size);
    
    void start_write();
    void handle_write(const boost::system::error_code& error, size_t bytes);
    
    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    
    std::array<uint8_t, MAX_PACKET_SIZE> recv_buffer_;
    size_t recv_buffer_size_;
    
    std::queue<std::vector<uint8_t>> send_queue_;
    std::mutex send_mutex_;
    bool write_in_progress_;
    
    int index_;
    std::string ip_address_;
    std::chrono::steady_clock::time_point connect_time_;
    std::chrono::steady_clock::time_point last_packet_time_;
};
```

**File**: `src/ClientSession.cpp`
- Implement all methods
- Preserve packet parsing logic from original
- Use strand for synchronization

### Step 2.2: Implement SocketManager

**File**: `include/SocketManager.h`
```cpp
#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include "ClientSession.h"

class SocketManager {
public:
    SocketManager(boost::asio::io_context& io);
    
    bool start(uint16_t port);
    void stop();
    uint32_t get_queue_size() const;
    
private:
    void start_accept();
    void handle_accept(std::shared_ptr<ClientSession> session,
                      const boost::system::error_code& error);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ClientSession>> sessions_;
    std::mutex sessions_mutex_;
};
```

**File**: `src/SocketManager.cpp`
- Implement TCP server
- IP filtering during accept
- Session management

### Step 2.3: Implement SocketManagerUdp

**File**: `include/SocketManagerUdp.h`
```cpp
#pragma once
#include <boost/asio.hpp>
#include <array>

constexpr size_t MAX_UDP_PACKET_SIZE = 4096;

class SocketManagerUdp {
public:
    SocketManagerUdp(boost::asio::io_context& io);
    
    bool start(uint16_t port);
    void stop();
    void async_send(const uint8_t* data, size_t size,
                   const std::string& ip, uint16_t port);
    
private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error, size_t bytes);
    bool parse_udp_packets();
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    
    std::array<uint8_t, MAX_UDP_PACKET_SIZE> recv_buffer_;
    size_t recv_buffer_size_;
};
```

**File**: `src/SocketManagerUdp.cpp`
- Implement UDP server
- Async receive/send
- Packet parsing

### Step 2.4: Implement TimerManager

**File**: `include/TimerManager.h`
```cpp
#pragma once
#include <boost/asio.hpp>
#include <atomic>

class TimerManager {
public:
    TimerManager(boost::asio::io_context& io);
    
    void start();
    void stop();
    
private:
    void schedule_1s_timer();
    void schedule_5s_timer();
    
    boost::asio::io_context& io_context_;
    boost::asio::steady_timer timer_1s_;
    boost::asio::steady_timer timer_5s_;
    std::atomic<bool> running_;
};
```

**File**: `src/TimerManager.cpp`
- 1-second timer for ServerList.MainProc()
- 5-second timer for timeout checks
- Auto-rescheduling

### Step 2.5: Test Phase 2

Create network test:
```cpp
// test_phase2.cpp
#include <boost/asio.hpp>
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "TimerManager.h"

int main() {
    boost::asio::io_context io;
    
    SocketManager tcp_server(io);
    tcp_server.start(44405);
    
    SocketManagerUdp udp_server(io);
    udp_server.start(55557);
    
    TimerManager timers(io);
    timers.start();
    
    io.run();
    return 0;
}
```

## Phase 3: Protocol (Week 5)

### Step 3.1: Port Business Logic Classes

These classes require minimal changes:
- `ServerList.cpp/h` - Change GetTickCount to get_tick_count()
- `IpManager.cpp/h` - No changes needed
- `ConnectServerProtocol.cpp/h` - No changes needed
- `ReadScript.cpp/h` - No changes needed
- `ClientManager.cpp/h` - Remove WSAOVERLAPPED, use ClientSession

### Step 3.2: Integrate Protocol Handler

Connect ClientSession packet processing to protocol handler:
```cpp
void ClientSession::process_packet(uint8_t head, const uint8_t* data, size_t size) {
    last_packet_time_ = std::chrono::steady_clock::now();
    console_protocol_log(CON_PROTO_TCP_RECV, data, size);
    ConnectServerProtocolCore(index_, head, const_cast<uint8_t*>(data), size);
}
```

### Step 3.3: Test Phase 3

Test with actual MuOnline client or packet simulator.

## Phase 4: Polish (Week 6)

### Step 4.1: Complete CLI Commands

Implement all commands in ConsoleInterface:
- `help` - Show commands
- `status` - Display metrics
- `reload` - Reload ServerList.dat
- `log tcp_recv on/off` - Toggle logging
- `log tcp_send on/off` - Toggle logging
- `clear` - Clear screen
- `exit` - Graceful shutdown

### Step 4.2: Implement ConnectServerApp

**File**: `include/ConnectServerApp.h`
```cpp
#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

class ConnectServerApp {
public:
    ConnectServerApp();
    ~ConnectServerApp();
    
    bool load_config(const std::string& filename);
    bool initialize();
    void run();
    void shutdown();
    
private:
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<
        boost::asio::io_context::executor_type> work_guard_;
    boost::asio::signal_set signals_;
    
    std::unique_ptr<SocketManager> socket_manager_;
    std::unique_ptr<SocketManagerUdp> socket_manager_udp_;
    std::unique_ptr<TimerManager> timer_manager_;
    std::unique_ptr<ConsoleInterface> console_;
    
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_;
    
    uint16_t tcp_port_;
    uint16_t udp_port_;
    int max_ip_connection_;
};
```

### Step 4.3: Implement main()

**File**: `src/main.cpp`
```cpp
#include "ConnectServerApp.h"
#include <iostream>
#include <exception>

#ifdef PLATFORM_LINUX
#include "platform/linux/SignalHandler.h"
#elif defined(PLATFORM_WINDOWS)
#include "platform/windows/CrashHandler.h"
#endif

int main(int argc, char* argv[]) {
    try {
        // Install crash handler
        #ifdef PLATFORM_LINUX
        SignalHandler::install();
        #elif defined(PLATFORM_WINDOWS)
        CMiniDump::Start();
        #endif
        
        ConnectServerApp app;
        
        if (!app.load_config("ConnectServer.ini")) {
            std::cerr << "Failed to load configuration\n";
            return 1;
        }
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize server\n";
            return 1;
        }
        
        app.run();
        app.shutdown();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
```

### Step 4.4: Testing

1. **Unit Tests**: Test individual components
2. **Integration Tests**: Test component interaction
3. **Protocol Tests**: Verify packet handling
4. **Load Tests**: Test with many clients
5. **Compatibility Tests**: Test on multiple platforms

### Step 4.5: Documentation

- Update README.md with build instructions
- Document any deviations from original
- Create troubleshooting guide
- Write deployment guide

## Building

### Linux
```bash
cd ConnectServerCross
chmod +x scripts/build.sh
./scripts/build.sh
```

### Windows
```cmd
cd ConnectServerCross
scripts\build.bat
```

## Running

### First Run
```bash
# Copy configuration files
cp config/ConnectServer.ini.example ConnectServer.ini
cp config/ServerList.dat.example ServerList.dat

# Edit configurations
nano ConnectServer.ini
nano ServerList.dat

# Run
./build/ConnectServer
```

## Troubleshooting

### Boost Not Found
```bash
# Ubuntu/Debian
sudo apt install libboost-all-dev

# Or specify Boost location
cmake .. -DBOOST_ROOT=/path/to/boost
```

### Port Already in Use
```bash
# Check what's using the port
sudo lsof -i :44405
sudo lsof -i :55557

# Kill the process or change port in ConnectServer.ini
```

### Permission Denied (Linux)
```bash
# Ports < 1024 require root or capabilities
sudo setcap 'cap_net_bind_service=+ep' ./ConnectServer
```

## Next Steps

After implementation:
1. Performance profiling
2. Memory leak detection (valgrind, ASAN)
3. Security audit
4. Documentation completion
5. Deployment automation

---

**Last Updated**: October 30, 2025
