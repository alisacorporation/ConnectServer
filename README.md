# ConnectServer Cross-Platform Edition

A complete rewrite of the MuOnline ConnectServer for full cross-platform compatibility (Windows/Linux) using modern C++17, Boost.Asio, and CLI interface.

## 🎯 Project Goals

- **Full Linux Compatibility**: Native Linux deployment without Wine or compatibility layers
- **Windows Compatibility**: Maintain full Windows support
- **Zero Functionality Loss**: 100% feature parity with original Windows version
- **Modern Architecture**: Clean C++17 codebase with Boost.Asio networking
- **Identical Protocol**: Byte-for-byte protocol compatibility with original

## 📋 Features

### Core Functionality
- ✅ TCP server for client connections (max 10,000 concurrent)
- ✅ UDP server for inter-server communication
- ✅ Server list management and distribution
- ✅ IP-based connection limiting
- ✅ Client timeout handling (5-minute idle timeout)
- ✅ Protocol handling (server list requests, server info)
- ✅ Configuration file support (INI format)
- ✅ File logging with rotation
- ✅ Console output with color coding

### Cross-Platform Features
- 🔄 Boost.Asio async networking (replaces IOCP)
- 🔄 CLI interface with ANSI colors (replaces Win32 GUI)
- 🔄 std::thread and std::mutex (replaces Win32 threading)
- 🔄 CMake build system (replaces Visual Studio project)
- 🔄 Signal handlers for graceful shutdown
- 🔄 Cross-platform crash handling

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│         ConnectServerApp (Main)             │
│  - Configuration loading                    │
│  - Signal handling (SIGINT/SIGTERM)         │
│  - Component initialization                 │
└────────────────┬────────────────────────────┘
                 │
     ┌───────────┴───────────┐
     │   io_context (Boost)  │
     │   - Worker threads    │
     │   - Async I/O         │
     └───────────┬───────────┘
                 │
  ┌──────────────┼──────────────┐
  │              │              │
┌─▼────────┐ ┌──▼─────┐ ┌──────▼──────┐
│ TCP      │ │  UDP   │ │   Timers    │
│ Acceptor │ │ Socket │ │  - 1s tick  │
│          │ │        │ │  - 5s tick  │
└──────────┘ └────────┘ └─────────────┘
```

## 📦 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Boost | ≥1.70 | Asio networking, system utilities |
| C++ Compiler | C++17 | g++, clang++, or MSVC |
| CMake | ≥3.15 | Build system |
| pthread | System | Threading support |
| ncurses (optional) | ≥6.0 | Terminal UI |

### Linux Installation
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake libboost-all-dev

# CentOS/RHEL
sudo yum install gcc-c++ cmake boost-devel

# Arch Linux
sudo pacman -S base-devel cmake boost
```

### Windows Installation
- Visual Studio 2019+ with C++ workload
- CMake 3.15+
- Boost 1.70+ (via vcpkg or manual installation)

## 🚀 Quick Start

### Building

```bash
# Clone or navigate to ConnectServerCross directory
cd ConnectServerCross

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Install (optional)
sudo cmake --install .
```

### Configuration

**Note:** Configuration files are automatically copied from `config/` folder during build. You can skip step 1 if using the build scripts or CMake.

1. (Optional) Manually copy example configuration files:
```bash
cp config/ConnectServer.ini.example ConnectServer.ini
cp config/ServerList.dat.example ServerList.dat
```

2. Edit `ConnectServer.ini`:
```ini
[ConnectServerInfo]
ConnectServerPortTCP=44405
ConnectServerPortUDP=55601
MaxIpConnection=5

[Log]
LOG=1
```

3. Edit `ServerList.dat`:
```
# ServerCode ServerName ServerAddress ServerPort ShowFlag
0 "TestServer" 127.0.0.1 55901 SHOW
```

### Running

```bash
# Linux
./ConnectServer

# Windows
ConnectServer.exe
```

### CLI Commands

Once running, use these commands:
- `help` - Show available commands
- `status` - Display server status
- `reload` - Reload ServerList.dat
- `log tcp_recv on/off` - Toggle TCP receive logging
- `log tcp_send on/off` - Toggle TCP send logging
- `clear` - Clear screen
- `exit` or `quit` - Shutdown server

## 📚 Documentation

Detailed documentation is available in the `docs/` directory:

- **[MIGRATION_PLAN.md](docs/MIGRATION_PLAN.md)** - Complete migration strategy and implementation plan
- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Detailed architecture and design decisions
- **[API_REFERENCE.md](docs/API_REFERENCE.md)** - Class and function reference
- **[PROTOCOL.md](docs/PROTOCOL.md)** - Network protocol specification
- **[WINDOWS_TO_LINUX.md](docs/WINDOWS_TO_LINUX.md)** - Windows API to Linux equivalents mapping
- **[BUILD_GUIDE.md](docs/BUILD_GUIDE.md)** - Comprehensive build instructions
- **[TESTING.md](docs/TESTING.md)** - Testing strategy and procedures

## 🔧 Development

### Project Structure

```
ConnectServerCross/
├── CMakeLists.txt           # Main build configuration
├── README.md                # This file
├── LICENSE                  # License information
├── .gitignore              # Git ignore rules
│
├── config/                  # Configuration examples
│   ├── ConnectServer.ini.example
│   └── ServerList.dat.example
│
├── docs/                    # Documentation
│   ├── MIGRATION_PLAN.md
│   ├── ARCHITECTURE.md
│   ├── API_REFERENCE.md
│   ├── PROTOCOL.md
│   ├── WINDOWS_TO_LINUX.md
│   ├── BUILD_GUIDE.md
│   └── TESTING.md
│
├── include/                 # Header files
│   ├── ConnectServerApp.h
│   ├── SocketManager.h
│   ├── ClientSession.h
│   └── ... (see docs for full list)
│
├── src/                     # Source files
│   ├── main.cpp
│   ├── ConnectServerApp.cpp
│   ├── SocketManager.cpp
│   └── ... (see docs for full list)
│
└── scripts/                 # Build and deployment scripts
    ├── build.sh             # Linux build script
    ├── build.bat            # Windows build script
    ├── test_ports.py        # Port testing (Python)
    ├── test_ports.sh        # Port testing (Bash)
    └── README_PORT_TESTING.md
```

### Implementation Phases

1. **Phase 1: Foundation** (Week 1-2)
   - Cross-platform infrastructure
   - Configuration management
   - Basic CLI interface

2. **Phase 2: Networking** (Week 3-4)
   - Boost.Asio integration
   - TCP/UDP socket managers
   - Client session management

3. **Phase 3: Protocol** (Week 5)
   - Protocol handling
   - Server list management
   - Business logic

4. **Phase 4: Polish** (Week 6)
   - Complete CLI interface
   - Testing and validation
   - Documentation

See [MIGRATION_PLAN.md](docs/MIGRATION_PLAN.md) for detailed implementation roadmap.

## 🧪 Testing

### Port Testing (Unix/Linux)

Test if TCP/UDP ports are correctly opened on Unix systems:

```bash
# Using Python script (recommended)
./scripts/test_ports.py

# Or using Bash script
./scripts/test_ports.sh
```

See [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md) for detailed documentation.

### Unit Tests

```bash
# Run unit tests
cd build
ctest --output-on-failure

# Run integration tests
./tests/integration_tests

# Run load tests
./tests/load_test --clients 1000
```

## 📊 Performance

Target performance metrics:
- **Concurrent Connections**: 10,000 clients
- **Packet Processing Latency**: <10ms average
- **Memory Usage**: <500MB with 10,000 clients
- **CPU Usage**: <50% on 4-core system with 10,000 clients

## 🔒 Security

- IP-based connection limiting
- Packet validation and sanitization
- Buffer overflow protection
- Graceful error handling
- No hardcoded credentials

## 🤝 Contributing

This is a migration project from the original Windows-only ConnectServer. Contributions should:
1. Maintain protocol compatibility
2. Preserve all original functionality
3. Follow modern C++ best practices
4. Include tests for new features
5. Update documentation

## 📝 License

[Specify your license here]

## 🙏 Acknowledgments

- Original MuOnline ConnectServer developers
- Boost.Asio library maintainers
- Community contributors

## 📞 Support

For issues, questions, or contributions:
- Check documentation in `docs/` directory
- Review existing issues
- Create detailed bug reports with logs

## 🗺️ Roadmap

- [x] Complete migration plan
- [ ] Phase 1: Foundation implementation
- [ ] Phase 2: Networking implementation
- [ ] Phase 3: Protocol implementation
- [ ] Phase 4: Testing and polish
- [ ] Phase 5: Optional ncurses TUI
- [ ] Docker support
- [ ] Systemd service files
- [ ] Performance optimization
- [ ] Monitoring/metrics endpoints

---

**Status**: 📋 Planning Complete - Ready for Implementation

**Last Updated**: October 30, 2025
