#!/bin/bash
# Simple wrapper to run port tests
# Usage: ./RUN_PORT_TEST.sh

set -e

echo "=== ConnectServer Port Test Runner ==="
echo ""

# Check if we're in the right directory
if [ ! -f "scripts/test_ports.py" ]; then
    echo "Error: Must run from ConnectServerCross directory"
    exit 1
fi

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

# Make scripts executable if needed (Unix only)
if [ "$IS_WINDOWS" = "0" ] && [ ! -x "scripts/test_ports.py" ]; then
    echo "Making test scripts executable..."
    chmod +x scripts/test_ports.py scripts/test_ports.sh
fi

# Check if binary exists
BINARY_FOUND=0
for BINARY_PATH in "${BINARY_PATHS[@]}"; do
    if [ -f "$BINARY_PATH" ]; then
        BINARY_FOUND=1
        echo "Found binary: $BINARY_PATH"
        break
    fi
done

if [ "$BINARY_FOUND" = "0" ]; then
    echo "Warning: ConnectServer binary not found"
    echo "Searched locations:"
    for BINARY_PATH in "${BINARY_PATHS[@]}"; do
        echo "  - $BINARY_PATH"
    done
    echo ""
    echo "Building project first..."
    echo ""
    
    mkdir -p build
    cd build
    cmake ..
    
    if [ "$IS_WINDOWS" = "1" ]; then
        # Windows: Use cmake --build with Release config
        cmake --build . --config Release
    else
        # Unix: Use make
        make
    fi
    
    cd ..
    
    echo ""
    echo "Build complete!"
    echo ""
fi

# Check if config exists
if [ ! -f "config/ConnectServer.ini" ]; then
    echo "Creating config files from examples..."
    cp config/ConnectServer.ini.example config/ConnectServer.ini
    cp config/ServerList.dat.example config/ServerList.dat
    echo "Config files created!"
    echo ""
fi

# Run the test
echo "Running port tests..."
echo ""

# Try to find Python (python3 on Unix, python on Windows)
if command -v python3 >/dev/null 2>&1; then
    PYTHON_CMD="python3"
elif command -v python >/dev/null 2>&1; then
    PYTHON_CMD="python"
else
    PYTHON_CMD=""
fi

if [ -n "$PYTHON_CMD" ]; then
    # Use Python script (preferred)
    if [ "$IS_WINDOWS" = "1" ]; then
        $PYTHON_CMD scripts/test_ports.py
    else
        ./scripts/test_ports.py
    fi
else
    # Fallback to Bash script
    echo "Python not found, using Bash script..."
    ./scripts/test_ports.sh
fi

exit $?
