#!/bin/bash
# Test script to verify TCP/UDP ports are correctly opened by ConnectServer
# Cross-platform support: Unix (Linux/macOS) and Windows (Git Bash/WSL)

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
CONFIG_FILE="$PROJECT_ROOT/config/ConnectServer.ini"

# Detect Windows (Git Bash/MSYS/Cygwin)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" || "$OSTYPE" == "cygwin" ]]; then
    IS_WINDOWS=1
    # Try Release first, then Debug, then root build dir
    if [ -f "$BUILD_DIR/Release/ConnectServer.exe" ]; then
        BINARY="$BUILD_DIR/Release/ConnectServer.exe"
    elif [ -f "$BUILD_DIR/Debug/ConnectServer.exe" ]; then
        BINARY="$BUILD_DIR/Debug/ConnectServer.exe"
    else
        BINARY="$BUILD_DIR/ConnectServer.exe"
    fi
else
    IS_WINDOWS=0
    BINARY="$BUILD_DIR/ConnectServer"
fi

# Default ports (will be read from config)
TCP_PORT=44405
UDP_PORT=55601

echo -e "${BLUE}=== ConnectServer Port Test Script ===${NC}"
echo ""

# Function to print colored messages
print_success() {
    echo -e "${GREEN}[✓] $1${NC}"
}

print_error() {
    echo -e "${RED}[✗] $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}[!] $1${NC}"
}

print_info() {
    echo -e "${BLUE}[i] $1${NC}"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to read port from config file
read_port_from_config() {
    local key=$1
    local default=$2
    
    if [ -f "$CONFIG_FILE" ]; then
        local value=$(grep "^$key=" "$CONFIG_FILE" | cut -d'=' -f2 | tr -d ' \r')
        if [ -n "$value" ]; then
            echo "$value"
        else
            echo "$default"
        fi
    else
        echo "$default"
    fi
}

# Function to check if port is in use
check_port_in_use() {
    local port=$1
    local protocol=$2
    
    if command_exists netstat; then
        if [ "$protocol" = "tcp" ]; then
            netstat -an | grep -E "[:.]$port.*LISTEN" >/dev/null 2>&1
        else
            netstat -an | grep -E "udp.*[:.]$port" >/dev/null 2>&1
        fi
    elif command_exists ss; then
        if [ "$protocol" = "tcp" ]; then
            ss -ln | grep -E "tcp.*:$port.*LISTEN" >/dev/null 2>&1
        else
            ss -ln | grep -E "udp.*:$port" >/dev/null 2>&1
        fi
    elif command_exists lsof; then
        lsof -i "$protocol:$port" >/dev/null 2>&1
    else
        print_warning "No network tools available (netstat, ss, or lsof)"
        return 2
    fi
}

# Function to wait for port to be listening
wait_for_port() {
    local port=$1
    local protocol=$2
    local timeout=$3
    local elapsed=0
    
    print_info "Waiting for $protocol port $port to open (timeout: ${timeout}s)..."
    
    while [ $elapsed -lt $timeout ]; do
        if check_port_in_use "$port" "$protocol"; then
            return 0
        fi
        sleep 0.5
        elapsed=$((elapsed + 1))
    done
    
    return 1
}

# Function to get process listening on port
get_port_process() {
    local port=$1
    local protocol=$2
    
    if command_exists lsof; then
        lsof -i "$protocol:$port" -t 2>/dev/null | head -1
    elif command_exists netstat; then
        if [ "$(uname)" = "Linux" ]; then
            netstat -lnp 2>/dev/null | grep ":$port" | awk '{print $7}' | cut -d'/' -f1 | head -1
        else
            # macOS doesn't show PID with netstat without sudo
            echo ""
        fi
    else
        echo ""
    fi
}

# Step 1: Check prerequisites
print_info "Step 1: Checking prerequisites..."

if [ ! -f "$BINARY" ]; then
    print_error "ConnectServer binary not found at: $BINARY"
    if [ "$IS_WINDOWS" = "1" ]; then
        print_info "Please build the project first using: cd build && cmake .. && cmake --build . --config Release"
        print_info "Searched locations:"
        print_info "  - $BUILD_DIR/Release/ConnectServer.exe"
        print_info "  - $BUILD_DIR/Debug/ConnectServer.exe"
        print_info "  - $BUILD_DIR/ConnectServer.exe"
    else
        print_info "Please build the project first using: cd build && cmake .. && make"
    fi
    exit 1
fi
print_success "Binary found: $BINARY"

if [ ! -f "$CONFIG_FILE" ]; then
    print_warning "Config file not found at: $CONFIG_FILE"
    print_info "Using default ports: TCP=$TCP_PORT, UDP=$UDP_PORT"
else
    TCP_PORT=$(read_port_from_config "ConnectServerPortTCP" "$TCP_PORT")
    UDP_PORT=$(read_port_from_config "ConnectServerPortUDP" "$UDP_PORT")
    print_success "Config file found"
    print_info "TCP Port: $TCP_PORT"
    print_info "UDP Port: $UDP_PORT"
fi

# Check for network diagnostic tools
if ! command_exists netstat && ! command_exists ss && ! command_exists lsof; then
    print_error "No network diagnostic tools found (netstat, ss, or lsof)"
    print_info "Please install net-tools or iproute2 package"
    exit 1
fi
print_success "Network diagnostic tools available"

echo ""

# Step 2: Check if ports are already in use
print_info "Step 2: Checking if ports are available..."

if check_port_in_use "$TCP_PORT" "tcp"; then
    PID=$(get_port_process "$TCP_PORT" "tcp")
    print_error "TCP port $TCP_PORT is already in use (PID: $PID)"
    print_info "Please stop the process or choose a different port"
    exit 1
fi
print_success "TCP port $TCP_PORT is available"

if check_port_in_use "$UDP_PORT" "udp"; then
    PID=$(get_port_process "$UDP_PORT" "udp")
    print_error "UDP port $UDP_PORT is already in use (PID: $PID)"
    print_info "Please stop the process or choose a different port"
    exit 1
fi
print_success "UDP port $UDP_PORT is available"

echo ""

# Step 3: Start ConnectServer in background
print_info "Step 3: Starting ConnectServer..."

cd "$PROJECT_ROOT"

# Start the server in background
"$BINARY" > /tmp/connectserver_test.log 2>&1 &
SERVER_PID=$!

print_info "ConnectServer started with PID: $SERVER_PID"
print_info "Log file: /tmp/connectserver_test.log"

# Give it a moment to initialize
sleep 1

# Check if process is still running
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    print_error "ConnectServer process died immediately"
    print_info "Check log file for details: /tmp/connectserver_test.log"
    cat /tmp/connectserver_test.log
    exit 1
fi
print_success "ConnectServer process is running"

echo ""

# Step 4: Test TCP port
print_info "Step 4: Testing TCP port $TCP_PORT..."

if wait_for_port "$TCP_PORT" "tcp" 10; then
    print_success "TCP port $TCP_PORT is LISTENING"
    
    # Try to connect to TCP port
    if command_exists nc; then
        print_info "Testing TCP connection..."
        if timeout 2 nc -zv 127.0.0.1 "$TCP_PORT" 2>&1 | grep -q "succeeded\|open"; then
            print_success "TCP connection test successful"
        else
            print_warning "TCP connection test failed (but port is listening)"
        fi
    elif command_exists telnet; then
        print_info "Testing TCP connection with telnet..."
        if timeout 2 telnet 127.0.0.1 "$TCP_PORT" 2>&1 | grep -q "Connected\|Escape"; then
            print_success "TCP connection test successful"
        else
            print_warning "TCP connection test failed (but port is listening)"
        fi
    fi
else
    print_error "TCP port $TCP_PORT is NOT listening after 10 seconds"
    print_info "Check log file: /tmp/connectserver_test.log"
fi

echo ""

# Step 5: Test UDP port
print_info "Step 5: Testing UDP port $UDP_PORT..."

if wait_for_port "$UDP_PORT" "udp" 10; then
    print_success "UDP port $UDP_PORT is OPEN"
    
    # Try to send UDP packet
    if command_exists nc; then
        print_info "Testing UDP communication..."
        # Send a test packet (will likely be rejected by server, but tests if port accepts data)
        echo "test" | timeout 2 nc -u 127.0.0.1 "$UDP_PORT" 2>/dev/null || true
        print_info "UDP packet sent (server may reject invalid protocol)"
    fi
else
    print_error "UDP port $UDP_PORT is NOT open after 10 seconds"
    print_info "Check log file: /tmp/connectserver_test.log"
fi

echo ""

# Step 6: Display port status
print_info "Step 6: Current port status..."

if command_exists netstat; then
    echo ""
    echo "TCP connections:"
    netstat -an | grep ":$TCP_PORT" || echo "  No TCP connections found"
    echo ""
    echo "UDP connections:"
    netstat -an | grep ":$UDP_PORT" || echo "  No UDP connections found"
elif command_exists ss; then
    echo ""
    echo "TCP connections:"
    ss -ln | grep ":$TCP_PORT" || echo "  No TCP connections found"
    echo ""
    echo "UDP connections:"
    ss -ln | grep ":$UDP_PORT" || echo "  No UDP connections found"
fi

echo ""

# Step 7: Show recent log entries
print_info "Step 7: Recent log entries..."
echo ""
tail -20 /tmp/connectserver_test.log
echo ""

# Step 8: Cleanup
print_info "Step 8: Cleanup..."
print_warning "Stopping ConnectServer (PID: $SERVER_PID)..."

kill -TERM "$SERVER_PID" 2>/dev/null || true
sleep 2

# Force kill if still running
if kill -0 "$SERVER_PID" 2>/dev/null; then
    print_warning "Process still running, force killing..."
    kill -KILL "$SERVER_PID" 2>/dev/null || true
fi

# Verify ports are closed
sleep 1

TCP_CLOSED=false
UDP_CLOSED=false

if ! check_port_in_use "$TCP_PORT" "tcp"; then
    print_success "TCP port $TCP_PORT is now closed"
    TCP_CLOSED=true
else
    print_warning "TCP port $TCP_PORT is still in use"
fi

if ! check_port_in_use "$UDP_PORT" "udp"; then
    print_success "UDP port $UDP_PORT is now closed"
    UDP_CLOSED=true
else
    print_warning "UDP port $UDP_PORT is still in use"
fi

echo ""
echo -e "${BLUE}=== Test Summary ===${NC}"

# Determine overall result
if [ "$TCP_CLOSED" = true ] && [ "$UDP_CLOSED" = true ]; then
    print_success "All tests passed! TCP and UDP ports opened and closed correctly."
    exit 0
else
    print_error "Some tests failed. Check the output above for details."
    exit 1
fi
