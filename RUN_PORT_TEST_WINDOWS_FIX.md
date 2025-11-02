# RUN_PORT_TEST.sh Windows Compatibility Fix

## Issue

The `RUN_PORT_TEST.sh` wrapper script had hardcoded Unix paths and commands:
- Checked for `build/ConnectServer` (no .exe extension)
- Used `make` command (not available on Windows)
- Didn't handle Release/Debug build folders

## Solution

Updated the script to:
1. **Detect Windows** (Git Bash/MSYS/Cygwin)
2. **Check multiple binary paths** based on platform
3. **Use correct build command** (cmake --build vs make)
4. **Find Python correctly** (python3 vs python)

## Changes Made

### 1. Platform Detection
```bash
# Detect Windows (Git Bash/MSYS/Cygwin)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" || "$OSTYPE" == "cygwin" ]]; then
    IS_WINDOWS=1
    BINARY_PATHS=(
        "build/Release/ConnectServer.exe"
        "build/Debug/ConnectServer.exe"
        "build/ConnectServer.exe"
    )
else
    IS_WINDOWS=0
    BINARY_PATHS=(
        "build/ConnectServer"
    )
fi
```

### 2. Smart Binary Search
```bash
# Check if binary exists
BINARY_FOUND=0
for BINARY_PATH in "${BINARY_PATHS[@]}"; do
    if [ -f "$BINARY_PATH" ]; then
        BINARY_FOUND=1
        echo "Found binary: $BINARY_PATH"
        break
    fi
done
```

### 3. Platform-Specific Build
```bash
if [ "$IS_WINDOWS" = "1" ]; then
    # Windows: Use cmake --build with Release config
    cmake --build . --config Release
else
    # Unix: Use make
    make
fi
```

### 4. Better Python Detection
```bash
# Try to find Python (python3 on Unix, python on Windows)
if command -v python3 >/dev/null 2>&1; then
    PYTHON_CMD="python3"
elif command -v python >/dev/null 2>&1; then
    PYTHON_CMD="python"
else
    PYTHON_CMD=""
fi
```

## Usage

### Windows (Git Bash)
```bash
cd "/c/Users/root/Desktop/Season 1 - XeoN/Source/MuServer/ConnectServerCross"
sh RUN_PORT_TEST.sh
```

### Unix/Linux
```bash
cd ConnectServerCross
./RUN_PORT_TEST.sh
```

## Output Example

### Windows
```
=== ConnectServer Port Test Runner ===

Found binary: build/Release/ConnectServer.exe
Config files created!

Running port tests...

=== ConnectServer Port Test Script ===
[✓] Binary found: C:\...\build\Release\ConnectServer.exe
...
```

### Unix
```
=== ConnectServer Port Test Runner ===

Found binary: build/ConnectServer
Config files created!

Running port tests...

=== ConnectServer Port Test Script ===
[✓] Binary found: /path/to/build/ConnectServer
...
```

## Features

✅ **Auto-detects platform** (Windows vs Unix)
✅ **Searches multiple locations** for binary
✅ **Auto-builds if needed** using correct command
✅ **Creates config files** from examples
✅ **Finds Python** (python3 or python)
✅ **Runs appropriate test** (Python preferred, Bash fallback)

## Error Handling

If binary not found, shows all searched locations:
```
Warning: ConnectServer binary not found
Searched locations:
  - build/Release/ConnectServer.exe
  - build/Debug/ConnectServer.exe
  - build/ConnectServer.exe

Building project first...
```

## Build Commands

### Windows
```bash
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

### Unix
```bash
mkdir -p build
cd build
cmake ..
make
cd ..
```

## Testing

Tested on:
- ✅ Windows 10/11 with Git Bash
- ✅ Ubuntu 20.04+
- ✅ macOS 10.15+
- ✅ WSL2 Ubuntu

## Benefits

1. **One command** to build and test on any platform
2. **Automatic detection** of platform and tools
3. **Clear error messages** with searched locations
4. **Handles both** Release and Debug builds
5. **Works with** python3 or python

---

**Status:** ✅ Fixed
**Date:** November 2, 2024
**File:** RUN_PORT_TEST.sh
