# ConnectServer Cross-Platform - Quick Start Guide

## 🚀 Quick Start (5 Minutes)

### Prerequisites Check

**Linux**:
```bash
# Check if tools are installed
which cmake g++ make
cmake --version  # Need 3.15+
g++ --version    # Need C++17 support

# Install if missing (Ubuntu/Debian)
sudo apt install build-essential cmake libboost-all-dev
```

**Windows**:
- Visual Studio 2019+ with C++ workload
- CMake 3.15+
- Boost 1.70+ (via vcpkg recommended)

### Build in 3 Steps

**Linux**:
```bash
cd ConnectServerCross
chmod +x scripts/build.sh
./scripts/build.sh
```

**Windows**:
```cmd
cd ConnectServerCross
scripts\build.bat
```

### Configure and Run

```bash
# Copy configuration files
cp config/ConnectServer.ini.example ConnectServer.ini
cp config/ServerList.dat.example ServerList.dat

# Edit as needed
nano ConnectServer.ini
nano ServerList.dat

# Run
cd build
./ConnectServer  # Linux
# or
cd build\Release
ConnectServer.exe  # Windows
```

---

## 📚 Documentation Overview

| Document | Purpose | When to Read |
|----------|---------|--------------|
| **README.md** | Project overview and features | Start here |
| **QUICK_START.md** | This file - get running fast | First time setup |
| **PROJECT_STATUS.md** | Implementation progress tracking | Check status |
| **docs/MIGRATION_PLAN.md** | Complete migration strategy | Understanding approach |
| **docs/ARCHITECTURE.md** | System design and components | Understanding code |
| **docs/PROTOCOL.md** | Network protocol specification | Protocol work |
| **docs/WINDOWS_TO_LINUX.md** | API mapping reference | During implementation |
| **docs/IMPLEMENTATION_GUIDE.md** | Step-by-step implementation | During development |

---

## 🗂️ Project Structure

```
ConnectServerCross/
├── 📄 README.md                    # Main documentation
├── 📄 QUICK_START.md               # This file
├── 📄 PROJECT_STATUS.md            # Progress tracking
├── 📄 CMakeLists.txt               # Build configuration
├── 📄 .gitignore                   # Git ignore rules
│
├── 📁 config/                      # Configuration examples
│   ├── ConnectServer.ini.example   # Server config template
│   └── ServerList.dat.example      # Server list template
│
├── 📁 docs/                        # Detailed documentation
│   ├── MIGRATION_PLAN.md           # Migration strategy
│   ├── ARCHITECTURE.md             # System architecture
│   ├── PROTOCOL.md                 # Protocol specification
│   ├── WINDOWS_TO_LINUX.md         # API mapping
│   └── IMPLEMENTATION_GUIDE.md     # Implementation steps
│
├── 📁 include/                     # Header files (to be implemented)
│   ├── ConnectServerApp.h
│   ├── SocketManager.h
│   ├── ClientSession.h
│   └── ... (see docs for full list)
│
├── 📁 src/                         # Source files (to be implemented)
│   ├── main.cpp
│   ├── ConnectServerApp.cpp
│   ├── SocketManager.cpp
│   └── ... (see docs for full list)
│
└── 📁 scripts/                     # Build scripts
    ├── build.sh                    # Linux build script
    └── build.bat                   # Windows build script
```

---

## 🎯 Current Status

**Phase**: Planning Complete ✅  
**Next**: Begin Phase 1 Implementation  
**Progress**: 0% (Ready to start)

See **PROJECT_STATUS.md** for detailed progress tracking.

---

## 🔧 Common Commands

### Build Commands

```bash
# Clean build
./scripts/build.sh --clean

# Debug build
./scripts/build.sh --debug

# With ncurses TUI
./scripts/build.sh --ncurses

# With tests
./scripts/build.sh --tests
```

### Runtime Commands

Once server is running, use these commands:

```
help                - Show available commands
status              - Display server status
reload              - Reload ServerList.dat
log tcp_recv on     - Enable TCP receive logging
log tcp_recv off    - Disable TCP receive logging
log tcp_send on     - Enable TCP send logging
log tcp_send off    - Disable TCP send logging
clear               - Clear screen
exit / quit         - Shutdown server
```

---

## 📝 Configuration Quick Reference

### ConnectServer.ini

```ini
[ConnectServerInfo]
ConnectServerPortTCP=44405    # Client connection port
ConnectServerPortUDP=55601    # Inter-server communication port
MaxIpConnection=5             # Max connections per IP (0=unlimited)

[Log]
LOG=1                         # Enable file logging

[Console]
EnableGeneralOutput=1         # General messages
EnableTCPRecvOutput=0         # TCP receive packets
EnableTCPSendOutput=0         # TCP send packets
```

### ServerList.dat

```
# ServerCode ServerName ServerAddress ServerPort ShowFlag
0 "TestServer" 127.0.0.1 55901 SHOW
1 "MainServer" 192.168.1.100 55901 SHOW
```

---

## 🐛 Troubleshooting

### Build Errors

**"Boost not found"**:
```bash
# Ubuntu/Debian
sudo apt install libboost-all-dev

# Or specify location
cmake .. -DBOOST_ROOT=/path/to/boost
```

**"C++17 required"**:
```bash
# Update compiler
sudo apt install g++-9  # or newer
```

### Runtime Errors

**"Port already in use"**:
```bash
# Check what's using the port
sudo lsof -i :44405

# Change port in ConnectServer.ini
```

**"Permission denied" (Linux)**:
```bash
# For ports < 1024, grant capability
sudo setcap 'cap_net_bind_service=+ep' ./ConnectServer
```

**"Cannot load ServerList.dat"**:
```bash
# Ensure file exists in same directory as executable
cp config/ServerList.dat.example ServerList.dat
```

---

## 🎓 Learning Path

### For Developers New to the Project

1. **Day 1**: Read README.md and QUICK_START.md
2. **Day 2**: Read MIGRATION_PLAN.md and ARCHITECTURE.md
3. **Day 3**: Read PROTOCOL.md and WINDOWS_TO_LINUX.md
4. **Day 4**: Study IMPLEMENTATION_GUIDE.md
5. **Day 5**: Set up development environment
6. **Week 2+**: Begin Phase 1 implementation

### For Experienced Developers

1. Read README.md (5 min)
2. Skim ARCHITECTURE.md (10 min)
3. Review IMPLEMENTATION_GUIDE.md (15 min)
4. Start coding (Phase 1)

---

## 📞 Getting Help

### Documentation

- Check **docs/** directory for detailed information
- Review **PROJECT_STATUS.md** for current progress
- See **IMPLEMENTATION_GUIDE.md** for step-by-step instructions

### Common Issues

Most issues are covered in:
- **IMPLEMENTATION_GUIDE.md** - Troubleshooting section
- **WINDOWS_TO_LINUX.md** - API mapping issues
- **docs/** - Detailed technical documentation

---

## ✅ Pre-Implementation Checklist

Before starting implementation:

- [ ] Read README.md
- [ ] Read MIGRATION_PLAN.md
- [ ] Review ARCHITECTURE.md
- [ ] Study PROTOCOL.md
- [ ] Check IMPLEMENTATION_GUIDE.md
- [ ] Development environment set up
- [ ] Boost library installed
- [ ] CMake installed
- [ ] C++17 compiler available
- [ ] Git repository initialized (optional)
- [ ] Team members briefed (if applicable)

---

## 🎯 Success Criteria

The project is successful when:

- ✅ Compiles on Windows and Linux
- ✅ All original functionality preserved
- ✅ Protocol byte-for-byte compatible
- ✅ Handles 10,000 concurrent clients
- ✅ Performance matches or exceeds original
- ✅ No memory leaks
- ✅ Graceful shutdown works
- ✅ All tests pass
- ✅ Documentation complete

---

## 🚀 Ready to Start?

1. **Read**: README.md (if you haven't)
2. **Review**: PROJECT_STATUS.md
3. **Follow**: docs/IMPLEMENTATION_GUIDE.md
4. **Build**: Start with Phase 1
5. **Test**: After each phase
6. **Document**: Update PROJECT_STATUS.md

**Good luck with the implementation!** 🎉

---

**Last Updated**: October 30, 2025
