# ConnectServer Cross-Platform Migration Plan

## Executive Summary

Complete rewrite of MuOnline ConnectServer for Linux compatibility using Boost.Asio and CLI while maintaining 100% functionality parity with Windows version.

## Current Architecture

### Core Components
- **SocketManager** - TCP server with IOCP (10,000 max clients)
- **SocketManagerUdp** - UDP server for inter-server communication  
- **ClientManager** - Connection management with 5-minute timeout
- **ServerList** - Game server list management
- **ConnectServerProtocol** - Packet handling (0xF4:0x02, 0xF4:0x03, 0xF4:0x04)
- **IpManager** - IP-based connection limiting
- **Queue** - Thread-safe packet queue with semaphore
- **Win32 GUI** - Window with GDI rendering and menu system

## Windows to Cross-Platform Mapping

| Windows API | Replacement |
|-------------|-------------|
| IOCP | Boost.Asio io_context |
| WSAAccept | async_accept + IP filter |
| WSARecv/WSASend | async_read/async_write |
| CreateThread | std::thread |
| CRITICAL_SECTION | std::recursive_mutex |
| Semaphore | std::condition_variable |
| SetTimer | boost::asio::steady_timer |
| Win32 GUI | CLI with ANSI colors |
| GetPrivateProfileInt | Custom INI parser |
| GetTickCount | std::chrono::steady_clock |
| MiniDumpWriteDump | signal handlers + backtrace |

## Implementation Phases

### Phase 1: Foundation (Week 1-2)
- CMake build system
- ConfigManager (INI parser)
- CriticalSection → std::mutex
- Queue → std::condition_variable  
- ConsoleInterface with ANSI colors
- Log system with std::fstream
- Signal handlers (Linux) / MiniDump (Windows)
- Util functions → std::chrono

### Phase 2: Networking (Week 3-4)
- ClientSession with Boost.Asio
- SocketManager → async_accept/read/write
- SocketManagerUdp → async_receive_from/send_to
- Thread pool with io_context
- ClientManager (remove WSAOVERLAPPED)
- TimerManager with steady_timer

### Phase 3: Protocol (Week 5)
- ConnectServerProtocol (no changes)
- ServerList (no changes)
- ReadScript (no changes)
- Packet queue integration
- Timeout handling

### Phase 4: Polish (Week 6)
- Complete CLI commands
- ServerDisplayer → ConsoleInterface
- Status display updates
- Graceful shutdown
- Testing and documentation

## Critical Implementation Details

### Packet Protocol Preservation
```cpp
// MUST preserve exact parsing logic
if (header == 0xC1) {
    packet_size = buffer[1];
    head = buffer[2];
} else if (header == 0xC2) {
    packet_size = MAKEWORD(buffer[2], buffer[1]);
    head = buffer[3];
}
```

### Connection Limit
```cpp
constexpr size_t MAX_CLIENT = 10000; // Exact same limit
```

### Timeout Handling
```cpp
constexpr auto MAX_ONLINE_TIME = std::chrono::milliseconds(300000); // 5 minutes
```

### IP Limiting
```cpp
// If MaxIpConnection == 0, allow all IPs
// Otherwise, limit connections per IP (same behavior)
```

## Testing Strategy

### Unit Tests
- Packet parsing validation
- IP limiting logic
- Queue operations
- Configuration loading

### Integration Tests
- 100 concurrent clients
- Protocol request/response
- Timeout behavior
- UDP communication

### Compatibility Tests
- Windows 10/11
- Ubuntu 20.04/22.04
- CentOS 8/Rocky Linux
- Protocol byte-for-byte validation

## Performance Targets

- **Concurrent Connections**: 10,000 clients
- **Packet Latency**: <10ms average
- **Memory Usage**: <500MB with 10,000 clients
- **CPU Usage**: <50% on 4-core system

## Risk Mitigation

### High-Risk Areas
1. IOCP → Boost.Asio conversion (performance)
2. Packet parsing edge cases (protocol compatibility)
3. Timing-dependent logic (timer precision)
4. Thread safety (race conditions)

### Mitigation
- Early benchmarking
- Extensive unit tests
- Packet capture validation
- Thread sanitizer testing
- Maintain Windows version in parallel

## File Structure

```
ConnectServerCross/
├── CMakeLists.txt
├── README.md
├── config/
│   ├── ConnectServer.ini.example
│   └── ServerList.dat.example
├── docs/
│   ├── MIGRATION_PLAN.md (this file)
│   ├── ARCHITECTURE.md
│   ├── API_REFERENCE.md
│   ├── PROTOCOL.md
│   └── BUILD_GUIDE.md
├── include/
│   ├── ConnectServerApp.h
│   ├── SocketManager.h
│   ├── ClientSession.h
│   ├── SocketManagerUdp.h
│   ├── ClientManager.h
│   ├── ServerList.h
│   ├── ConnectServerProtocol.h
│   ├── IpManager.h
│   ├── Queue.h
│   ├── CriticalSection.h
│   ├── Log.h
│   ├── Console.h
│   ├── ConsoleInterface.h
│   ├── TimerManager.h
│   ├── ConfigManager.h
│   ├── ReadScript.h
│   ├── Util.h
│   ├── ProtocolDefines.h
│   └── platform/
│       ├── linux/SignalHandler.h
│       └── windows/CrashHandler.h
└── src/
    ├── main.cpp
    ├── ConnectServerApp.cpp
    ├── SocketManager.cpp
    ├── ClientSession.cpp
    ├── SocketManagerUdp.cpp
    ├── ClientManager.cpp
    ├── ServerList.cpp
    ├── ConnectServerProtocol.cpp
    ├── IpManager.cpp
    ├── Queue.cpp
    ├── CriticalSection.cpp
    ├── Log.cpp
    ├── Console.cpp
    ├── ConsoleInterface.cpp
    ├── TimerManager.cpp
    ├── ConfigManager.cpp
    ├── ReadScript.cpp
    ├── Util.cpp
    └── platform/
        ├── linux/SignalHandler.cpp
        └── windows/CrashHandler.cpp
```

## Dependencies

- Boost ≥1.70 (Asio, System)
- C++17 compiler (g++, clang++, MSVC)
- CMake ≥3.15
- pthread (Linux)
- ncurses (optional, for TUI)

## Next Steps

1. Review this migration plan
2. Set up development environment
3. Begin Phase 1 implementation
4. Follow test-driven development approach
5. Maintain protocol compatibility at every step

---

**Status**: Planning Complete - Ready for Implementation
**Last Updated**: October 30, 2025
