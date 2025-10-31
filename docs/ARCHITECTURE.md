# ConnectServer Architecture Documentation

## Overview

This document describes the architecture of the cross-platform ConnectServer implementation using Boost.Asio and modern C++17.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  ConnectServerApp                        │
│  - Configuration Management                              │
│  - Component Lifecycle                                   │
│  - Signal Handling (SIGINT/SIGTERM)                     │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────┴───────────┐
         │   io_context (Boost)  │
         │   - Event Loop        │
         │   - Worker Threads    │
         │   - Async Operations  │
         └───────────┬───────────┘
                     │
      ┌──────────────┼──────────────┐
      │              │              │
┌─────▼─────┐  ┌────▼────┐  ┌──────▼──────┐
│ Socket    │  │  Socket │  │   Timer     │
│ Manager   │  │ Manager │  │   Manager   │
│ (TCP)     │  │ (UDP)   │  │             │
└─────┬─────┘  └────┬────┘  └──────┬──────┘
      │             │              │
      │             │              │
┌─────▼─────────────▼──────────────▼──────┐
│      Client Session Manager              │
│  - Session Pool (10,000 max)             │
│  - Connection State Tracking             │
│  - Timeout Management                    │
└──────────────┬───────────────────────────┘
               │
        ┌──────┴──────┐
        │   Protocol  │
        │   Handler   │
        └─────┬───────┘
              │
        ┌─────▼──────┐
        │  Server    │
        │  List      │
        │  Manager   │
        └────────────┘
```

## Core Components

### 1. ConnectServerApp

**Responsibility**: Application lifecycle management

**Key Features**:
- Configuration loading from INI file
- Component initialization and cleanup
- Signal handling for graceful shutdown
- Worker thread pool management
- Main event loop coordination

**Interface**:
```cpp
class ConnectServerApp {
public:
    bool load_config(const std::string& filename);
    bool initialize();
    void run();
    void shutdown();
};
```

### 2. SocketManager (TCP)

**Responsibility**: TCP client connection management

**Key Features**:
- Async accept with IP filtering
- Connection pool (10,000 max)
- Per-client session management
- Graceful disconnect handling

**Architecture Pattern**: Proactor with strand-based synchronization

**Interface**:
```cpp
class SocketManager {
public:
    SocketManager(boost::asio::io_context& io);
    bool start(uint16_t port);
    void stop();
    
private:
    void start_accept();
    void handle_accept(std::shared_ptr<ClientSession> session,
                      const boost::system::error_code& error);
};
```

### 3. ClientSession

**Responsibility**: Individual client connection handling

**Key Features**:
- Async read/write operations
- Packet parsing and validation
- Send queue management
- Timeout tracking

**Lifetime Management**: `shared_ptr` with `enable_shared_from_this`

**Interface**:
```cpp
class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    void start();
    void async_send(const uint8_t* data, size_t size);
    void close();
    bool check_timeout() const;
    
private:
    void start_read();
    void handle_read(const boost::system::error_code& error, size_t bytes);
    bool parse_packets();
    void process_packet(uint8_t head, const uint8_t* data, size_t size);
};
```

### 4. SocketManagerUdp

**Responsibility**: UDP inter-server communication

**Key Features**:
- Async receive from GameServers/JoinServer
- Async send to remote servers
- Packet parsing and routing

**Interface**:
```cpp
class SocketManagerUdp {
public:
    SocketManagerUdp(boost::asio::io_context& io);
    bool start(uint16_t port);
    void async_send(const uint8_t* data, size_t size,
                   const std::string& ip, uint16_t port);
    
private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error, size_t bytes);
};
```

### 5. TimerManager

**Responsibility**: Periodic task scheduling

**Key Features**:
- 1-second timer for server list maintenance
- 5-second timer for client timeout checks
- Auto-rescheduling timers

**Interface**:
```cpp
class TimerManager {
public:
    TimerManager(boost::asio::io_context& io);
    void start();
    void stop();
    
private:
    void schedule_1s_timer();
    void schedule_5s_timer();
};
```

### 6. ServerList

**Responsibility**: Game server list management

**Key Features**:
- Load server list from file
- Track server online/offline state
- Generate server list for clients
- Monitor JoinServer connectivity

**Interface**:
```cpp
class CServerList {
public:
    void load(const std::string& path);
    void main_proc();
    bool check_join_server_state();
    long generate_custom_server_list(uint8_t* buffer, int* size);
    long generate_server_list(uint8_t* buffer, int* size);
    SERVER_LIST_INFO* get_server_list_info(int server_code);
};
```

### 7. IpManager

**Responsibility**: IP-based connection limiting

**Key Features**:
- Track connections per IP
- Configurable max connections per IP
- Automatic cleanup on disconnect

**Interface**:
```cpp
class CIpManager {
public:
    bool check_ip_address(const std::string& ip);
    void insert_ip_address(const std::string& ip);
    void remove_ip_address(const std::string& ip);
};
```

### 8. ConsoleInterface

**Responsibility**: CLI user interface

**Key Features**:
- ANSI color-coded output
- Command input processing
- Status display updates
- Log message display

**Interface**:
```cpp
class ConsoleInterface {
public:
    void initialize();
    void log(Color color, const std::string& message);
    void update_status(const std::string& status, size_t queue_size);
    void start_input_loop();
    
private:
    void process_command(const std::string& cmd);
    void show_help();
    void show_status();
};
```

## Threading Model

### Worker Thread Pool

```cpp
// Main thread
main() → ConnectServerApp::run() → console input loop

// Worker threads (CPU count, max 8)
Thread 1-N: io_context.run() → async operation handlers
```

### Synchronization Strategy

1. **Per-Session Strand**: Each ClientSession uses a strand for serialized execution
2. **Shared Data Mutex**: Global data protected by std::mutex
3. **Lock-Free Where Possible**: Atomic operations for flags and counters

### Thread Safety Guarantees

- **ClientSession**: All operations serialized via strand
- **ServerList**: Protected by mutex for state updates
- **IpManager**: Protected by mutex for map operations
- **Queue**: Thread-safe with condition variable

## Data Flow

### Client Connection Flow

```
1. Client connects
   ↓
2. acceptor_.async_accept()
   ↓
3. IP filtering check
   ↓
4. Create ClientSession
   ↓
5. session->start()
   ↓
6. Send init packet (0x00)
   ↓
7. Start async_read
```

### Packet Processing Flow

```
1. async_read completes
   ↓
2. handle_read() called
   ↓
3. parse_packets() - validate headers
   ↓
4. process_packet() - route by head
   ↓
5. ConnectServerProtocolCore()
   ↓
6. Generate response
   ↓
7. async_send()
```

### Server List Request Flow

```
Client sends 0xF4:0x02
   ↓
CCCustomServerListSend() - send custom list (0xF4:0x04)
   ↓
CCServerListRecv() - send server list (0xF4:0x02 response)
```

### Server Info Request Flow

```
Client sends 0xF4:0x03 with ServerCode
   ↓
Check JoinServer state
   ↓
Lookup server in ServerList
   ↓
Send server address/port (0xF4:0x03 response)
```

## Protocol Specification

### Packet Structure

**Type C1 (Small Packet)**:
```
[0xC1] [Size] [Head] [Data...]
```

**Type C2 (Large Packet)**:
```
[0xC2] [Size_H] [Size_L] [Head] [Data...]
```

### Supported Packets

| Head | SubHead | Direction | Description |
|------|---------|-----------|-------------|
| 0x00 | - | S→C | Server Init |
| 0xF4 | 0x02 | C→S | Server List Request |
| 0xF4 | 0x02 | S→C | Server List Response |
| 0xF4 | 0x03 | C→S | Server Info Request |
| 0xF4 | 0x03 | S→C | Server Info Response |
| 0xF4 | 0x04 | S→C | Custom Server List |
| 0x01 | - | GS→CS | Game Server Live (UDP) |
| 0x02 | - | JS→CS | Join Server Live (UDP) |

## Configuration

### ConnectServer.ini Format

```ini
[ConnectServerInfo]
ConnectServerPortTCP=44405
ConnectServerPortUDP=55557
MaxIpConnection=5

[Log]
LOG=1
```

### ServerList.dat Format

```
# ServerCode ServerName ServerAddress ServerPort ShowFlag
0 "TestServer" 127.0.0.1 55901 SHOW
1 "MainServer" 192.168.1.100 55901 SHOW
2 "TestServer2" 127.0.0.1 55902 HIDE
```

## Error Handling

### Strategy

1. **Exceptions**: Used for initialization failures
2. **Error Codes**: Used for async operations (Boost.Asio)
3. **Logging**: All errors logged with context
4. **Graceful Degradation**: Continue operation when possible

### Error Categories

- **Network Errors**: Connection failures, timeouts
- **Protocol Errors**: Invalid packets, malformed data
- **Configuration Errors**: Missing files, invalid values
- **Resource Errors**: Memory allocation, file I/O

## Performance Optimization

### Key Optimizations

1. **Zero-Copy**: Use `boost::asio::buffer()` with existing buffers
2. **Connection Pooling**: Reuse ClientSession objects
3. **Strand-Based Sync**: Avoid mutex contention per session
4. **Efficient Packet Parsing**: Single-pass parsing with minimal copies

### Scalability Limits

- **Max Clients**: 10,000 (configurable via MAX_CLIENT)
- **Max Packet Size**: 2048 bytes (TCP), 4096 bytes (UDP)
- **Worker Threads**: CPU count (max 8)
- **Queue Size**: 2048 packets

## Platform-Specific Code

### Linux

- Signal handlers (SIGINT, SIGTERM)
- Backtrace for crash dumps
- ANSI terminal colors

### Windows

- SetUnhandledExceptionFilter
- MiniDumpWriteDump
- Console API for colors (optional)

### Abstraction Layer

Platform-specific code isolated in `platform/` directory:
- `platform/linux/SignalHandler.cpp`
- `platform/windows/CrashHandler.cpp`

## Build System

### CMake Configuration

- C++17 standard required
- Boost 1.70+ required
- Platform detection (WIN32, UNIX)
- Optional ncurses support
- Install targets for binaries and configs

### Compiler Support

- GCC 7+
- Clang 5+
- MSVC 2019+

## Monitoring and Diagnostics

### Logging Levels

- **ERROR**: Critical errors requiring attention
- **WARNING**: Non-critical issues
- **INFO**: General information
- **DEBUG**: Detailed diagnostic information

### Metrics

- Active connections count
- Packet queue size
- Bytes sent/received
- Server list state
- Timeout events

### CLI Commands

- `status` - Display current metrics
- `log tcp_recv on/off` - Toggle receive logging
- `log tcp_send on/off` - Toggle send logging
- `reload` - Reload server list
- `exit` - Graceful shutdown

---

**Last Updated**: October 30, 2025
