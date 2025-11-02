#!/usr/bin/env python3
"""
Test script to verify TCP/UDP ports are correctly opened by ConnectServer
Cross-platform support: Unix (Linux/macOS) and Windows
"""

import os
import sys
import time
import socket
import subprocess
import signal
import re
import platform
from pathlib import Path

# ANSI color codes
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def print_success(msg):
    print(f"{Colors.GREEN}[✓] {msg}{Colors.NC}")

def print_error(msg):
    print(f"{Colors.RED}[✗] {msg}{Colors.NC}")

def print_warning(msg):
    print(f"{Colors.YELLOW}[!] {msg}{Colors.NC}")

def print_info(msg):
    print(f"{Colors.BLUE}[i] {msg}{Colors.NC}")

def read_config_value(config_file, key, default):
    """Read a value from the INI config file"""
    try:
        with open(config_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith(key + '='):
                    value = line.split('=', 1)[1].strip()
                    return int(value)
    except (FileNotFoundError, ValueError):
        pass
    return default

def is_port_in_use(port, protocol='tcp'):
    """Check if a port is in use"""
    try:
        if protocol == 'tcp':
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex(('127.0.0.1', port))
            sock.close()
            return result == 0
        else:  # UDP
            # For UDP, we try to bind to check if it's available
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            try:
                sock.bind(('127.0.0.1', port))
                sock.close()
                return False  # Port is available
            except OSError:
                return True  # Port is in use
    except Exception as e:
        print_warning(f"Error checking port {port}: {e}")
        return False

def check_port_listening(port, protocol='tcp'):
    """Check if a port is listening using netstat/ss"""
    try:
        # Try ss first (modern)
        result = subprocess.run(['ss', '-ln'], capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            pattern = f"{'tcp' if protocol == 'tcp' else 'udp'}.*:{port}"
            return bool(re.search(pattern, result.stdout))
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    
    try:
        # Try netstat (legacy)
        result = subprocess.run(['netstat', '-an'], capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            pattern = f"{'tcp' if protocol == 'tcp' else 'udp'}.*:{port}"
            return bool(re.search(pattern, result.stdout))
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    
    # Fallback to socket test
    return is_port_in_use(port, protocol)

def wait_for_port(port, protocol='tcp', timeout=10):
    """Wait for a port to start listening"""
    print_info(f"Waiting for {protocol.upper()} port {port} to open (timeout: {timeout}s)...")
    
    start_time = time.time()
    while time.time() - start_time < timeout:
        if check_port_listening(port, protocol):
            return True
        time.sleep(0.5)
    
    return False

def test_tcp_connection(port):
    """Test TCP connection to the port"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        result = sock.connect_ex(('127.0.0.1', port))
        sock.close()
        
        if result == 0:
            print_success("TCP connection test successful")
            return True
        else:
            print_warning(f"TCP connection failed with error code: {result}")
            return False
    except Exception as e:
        print_warning(f"TCP connection test error: {e}")
        return False

def test_udp_send(port):
    """Test sending UDP packet to the port"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(2)
        
        # Send a test packet
        test_data = b"test"
        sock.sendto(test_data, ('127.0.0.1', port))
        
        print_info("UDP packet sent (server may reject invalid protocol)")
        sock.close()
        return True
    except Exception as e:
        print_warning(f"UDP send test error: {e}")
        return False

def get_port_info(port, protocol='tcp'):
    """Get detailed information about what's listening on a port"""
    try:
        # Try lsof
        result = subprocess.run(
            ['lsof', '-i', f'{protocol}:{port}'],
            capture_output=True, text=True, timeout=5
        )
        if result.returncode == 0 and result.stdout:
            return result.stdout.strip()
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    
    try:
        # Try ss with process info
        result = subprocess.run(
            ['ss', '-lnp', f'sport = :{port}'],
            capture_output=True, text=True, timeout=5
        )
        if result.returncode == 0 and result.stdout:
            return result.stdout.strip()
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    
    return None

def main():
    print(f"{Colors.BLUE}=== ConnectServer Port Test Script ==={Colors.NC}\n")
    
    # Detect platform
    is_windows = platform.system() == 'Windows'
    
    # Setup paths
    script_dir = Path(__file__).parent.absolute()
    project_root = script_dir.parent
    build_dir = project_root / 'build'
    config_file = project_root / 'config' / 'ConnectServer.ini'
    
    # Binary path differs on Windows
    if is_windows:
        # Try Release first, then Debug
        binary = build_dir / 'Release' / 'ConnectServer.exe'
        if not binary.exists():
            binary = build_dir / 'Debug' / 'ConnectServer.exe'
        if not binary.exists():
            binary = build_dir / 'ConnectServer.exe'
        log_file = Path(os.environ.get('TEMP', 'C:/Temp')) / 'connectserver_test.log'
    else:
        binary = build_dir / 'ConnectServer'
        log_file = Path('/tmp/connectserver_test.log')
    
    # Default ports
    tcp_port = 44405
    udp_port = 55601
    
    # Step 1: Check prerequisites
    print_info("Step 1: Checking prerequisites...")
    
    if not binary.exists():
        print_error(f"ConnectServer binary not found at: {binary}")
        if is_windows:
            print_info("Please build the project first using: cd build && cmake .. && cmake --build . --config Release")
            print_info("Searched locations:")
            print_info(f"  - {build_dir / 'Release' / 'ConnectServer.exe'}")
            print_info(f"  - {build_dir / 'Debug' / 'ConnectServer.exe'}")
            print_info(f"  - {build_dir / 'ConnectServer.exe'}")
        else:
            print_info("Please build the project first using: cd build && cmake .. && make")
        return 1
    print_success(f"Binary found: {binary}")
    
    if not config_file.exists():
        print_warning(f"Config file not found at: {config_file}")
        print_info(f"Using default ports: TCP={tcp_port}, UDP={udp_port}")
    else:
        tcp_port = read_config_value(config_file, 'ConnectServerPortTCP', tcp_port)
        udp_port = read_config_value(config_file, 'ConnectServerPortUDP', udp_port)
        print_success("Config file found")
        print_info(f"TCP Port: {tcp_port}")
        print_info(f"UDP Port: {udp_port}")
    
    print()
    
    # Step 2: Check if ports are already in use
    print_info("Step 2: Checking if ports are available...")
    
    if check_port_listening(tcp_port, 'tcp'):
        print_error(f"TCP port {tcp_port} is already in use")
        info = get_port_info(tcp_port, 'tcp')
        if info:
            print(info)
        return 1
    print_success(f"TCP port {tcp_port} is available")
    
    if check_port_listening(udp_port, 'udp'):
        print_error(f"UDP port {udp_port} is already in use")
        info = get_port_info(udp_port, 'udp')
        if info:
            print(info)
        return 1
    print_success(f"UDP port {udp_port} is available")
    
    print()
    
    # Step 3: Start ConnectServer
    print_info("Step 3: Starting ConnectServer...")
    
    os.chdir(project_root)
    
    # Start the server
    with open(log_file, 'w') as log:
        process = subprocess.Popen(
            [str(binary)],
            stdout=log,
            stderr=subprocess.STDOUT,
            cwd=project_root
        )
    
    server_pid = process.pid
    print_info(f"ConnectServer started with PID: {server_pid}")
    print_info(f"Log file: {log_file}")
    
    # Give it a moment to initialize
    time.sleep(1)
    
    # Check if process is still running
    if process.poll() is not None:
        print_error("ConnectServer process died immediately")
        print_info(f"Check log file for details: {log_file}")
        with open(log_file, 'r') as f:
            print(f.read())
        return 1
    print_success("ConnectServer process is running")
    
    print()
    
    # Step 4: Test TCP port
    print_info(f"Step 4: Testing TCP port {tcp_port}...")
    
    tcp_success = False
    if wait_for_port(tcp_port, 'tcp', 10):
        print_success(f"TCP port {tcp_port} is LISTENING")
        tcp_success = True
        
        # Try to connect
        print_info("Testing TCP connection...")
        test_tcp_connection(tcp_port)
    else:
        print_error(f"TCP port {tcp_port} is NOT listening after 10 seconds")
        print_info(f"Check log file: {log_file}")
    
    print()
    
    # Step 5: Test UDP port
    print_info(f"Step 5: Testing UDP port {udp_port}...")
    
    udp_success = False
    if wait_for_port(udp_port, 'udp', 10):
        print_success(f"UDP port {udp_port} is OPEN")
        udp_success = True
        
        # Try to send UDP packet
        print_info("Testing UDP communication...")
        test_udp_send(udp_port)
    else:
        print_error(f"UDP port {udp_port} is NOT open after 10 seconds")
        print_info(f"Check log file: {log_file}")
    
    print()
    
    # Step 6: Display port status
    print_info("Step 6: Current port status...")
    print()
    
    tcp_info = get_port_info(tcp_port, 'tcp')
    if tcp_info:
        print("TCP port info:")
        print(tcp_info)
    else:
        print("TCP port info: Not available")
    
    print()
    
    udp_info = get_port_info(udp_port, 'udp')
    if udp_info:
        print("UDP port info:")
        print(udp_info)
    else:
        print("UDP port info: Not available")
    
    print()
    
    # Step 7: Show recent log entries
    print_info("Step 7: Recent log entries...")
    print()
    
    try:
        with open(log_file, 'r') as f:
            lines = f.readlines()
            for line in lines[-20:]:
                print(line.rstrip())
    except Exception as e:
        print_warning(f"Could not read log file: {e}")
    
    print()
    
    # Step 8: Cleanup
    print_info("Step 8: Cleanup...")
    print_warning(f"Stopping ConnectServer (PID: {server_pid})...")
    
    try:
        process.terminate()
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        print_warning("Process did not terminate gracefully, force killing...")
        process.kill()
        process.wait()
    
    # Verify ports are closed
    time.sleep(1)
    
    tcp_closed = not check_port_listening(tcp_port, 'tcp')
    udp_closed = not check_port_listening(udp_port, 'udp')
    
    if tcp_closed:
        print_success(f"TCP port {tcp_port} is now closed")
    else:
        print_warning(f"TCP port {tcp_port} is still in use")
    
    if udp_closed:
        print_success(f"UDP port {udp_port} is now closed")
    else:
        print_warning(f"UDP port {udp_port} is still in use")
    
    print()
    print(f"{Colors.BLUE}=== Test Summary ==={Colors.NC}")
    
    # Determine overall result
    if tcp_success and udp_success and tcp_closed and udp_closed:
        print_success("All tests passed! TCP and UDP ports opened and closed correctly.")
        return 0
    else:
        print_error("Some tests failed. Check the output above for details.")
        if not tcp_success:
            print_error("  - TCP port did not open correctly")
        if not udp_success:
            print_error("  - UDP port did not open correctly")
        if not tcp_closed:
            print_error("  - TCP port did not close correctly")
        if not udp_closed:
            print_error("  - UDP port did not close correctly")
        return 1

if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        sys.exit(130)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
