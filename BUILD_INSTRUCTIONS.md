# Build Instructions - Phase 1

## Quick Build Guide

### Prerequisites

**Linux (Ubuntu/Debian)**:
```bash
sudo apt update
sudo apt install build-essential cmake libboost-all-dev
```

**Linux (CentOS/RHEL)**:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake boost-devel
```

**Windows**:
- Visual Studio 2019+ with C++ workload
- CMake 3.15+
- Boost 1.70+ (install via vcpkg or manually)

---

## Build Steps

### Linux

```bash
# 1. Navigate to project directory
cd ConnectServerCross

# 2. Copy configuration file
cp config/ConnectServer.ini.example ConnectServer.ini

# 3. Make build script executable
chmod +x scripts/build.sh

# 4. Build
./scripts/build.sh

# 5. Run
cd build
./ConnectServer
```

### Windows

```cmd
REM 1. Navigate to project directory
cd ConnectServerCross

REM 2. Copy configuration file
copy config\ConnectServer.ini.example ConnectServer.ini

REM 3. Build
scripts\build.bat

REM 4. Run
cd build\Release
ConnectServer.exe
```

---

## Manual Build (CMake)

### Linux

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
./ConnectServer
```

### Windows

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd Release
ConnectServer.exe
```

---

## Build Options

### Debug Build

**Linux**:
```bash
./scripts/build.sh --debug
```

**Windows**:
```cmd
scripts\build.bat --debug
```

### Clean Build

**Linux**:
```bash
./scripts/build.sh --clean
```

**Windows**:
```cmd
scripts\build.bat --clean
```

---

## Troubleshooting

### "Boost not found"

**Linux**:
```bash
# Install Boost
sudo apt install libboost-all-dev

# Or specify Boost location
cmake .. -DBOOST_ROOT=/path/to/boost
```

**Windows**:
```cmd
REM Install via vcpkg
vcpkg install boost:x64-windows

REM Or specify Boost location
cmake .. -DBOOST_ROOT=C:\path\to\boost
```

### "C++17 required"

**Linux**:
```bash
# Update compiler
sudo apt install g++-9  # or newer

# Or use clang
sudo apt install clang-10
```

**Windows**:
- Update Visual Studio to 2019 or later

### "CMake version too old"

**Linux**:
```bash
# Add Kitware APT repository for latest CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake
```

**Windows**:
- Download latest CMake from cmake.org

---

## Verification

After building, run the program. You should see:

```
=== ConnectServer Phase 1 Test ===

[OK] Signal handler installed
[OK] Configuration loaded
[OK] Critical section tests passed
[OK] Queue tests passed
[OK] Console output working
[OK] ConsoleInterface initialized

=== All Phase 1 Tests Passed ===
```

Then you can use interactive commands:
- `help` - Show available commands
- `status` - Show server status
- `exit` - Quit program

---

## Next Steps

Once Phase 1 builds successfully:
1. Verify all tests pass
2. Test on both Windows and Linux (if possible)
3. Proceed to Phase 2 implementation

---

**Last Updated**: October 30, 2025
