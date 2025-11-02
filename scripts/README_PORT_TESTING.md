# Port Testing Scripts

This directory contains scripts to test if TCP/UDP ports are correctly opened when starting ConnectServer on Unix systems.

## Available Scripts

### 1. `test_ports.py` (Recommended)
Python-based test script with better cross-platform compatibility and detailed diagnostics.

**Requirements:**
- Python 3.6+
- No additional dependencies required

**Usage:**
```bash
# Make executable (first time only)
chmod +x scripts/test_ports.py

# Run the test
./scripts/test_ports.py

# Or run with python directly
python3 scripts/test_ports.py
```

### 2. `test_ports.sh`
Bash-based test script for Unix systems.

**Requirements:**
- Bash shell
- Network tools: `netstat`, `ss`, or `lsof`
- Optional: `nc` (netcat) or `telnet` for connection testing

**Usage:**
```bash
# Make executable (first time only)
chmod +x scripts/test_ports.sh

# Run the test
./scripts/test_ports.sh
```

## What the Tests Do

Both scripts perform the following steps:

1. **Check Prerequisites**
   - Verify ConnectServer binary exists
   - Read configuration file to get port numbers
   - Check for required network diagnostic tools

2. **Check Port Availability**
   - Verify TCP and UDP ports are not already in use
   - Display which process is using the port if occupied

3. **Start ConnectServer**
   - Launch the server in the background
   - Redirect output to a log file
   - Verify the process starts successfully

4. **Test TCP Port**
   - Wait for TCP port to start listening (up to 10 seconds)
   - Attempt to connect to the port
   - Report success or failure

5. **Test UDP Port**
   - Wait for UDP port to open (up to 10 seconds)
   - Send a test UDP packet
   - Report success or failure

6. **Display Port Status**
   - Show current network connections
   - Display process information for the ports

7. **Show Log Entries**
   - Display recent log output from ConnectServer
   - Help diagnose any startup issues

8. **Cleanup**
   - Gracefully stop ConnectServer
   - Verify ports are closed
   - Report final test results

## Configuration

The scripts automatically read port numbers from:
```
config/ConnectServer.ini
```

Default ports if config is not found:
- TCP: 44405
- UDP: 55601

## Expected Output

### Successful Test
```
=== ConnectServer Port Test Script ===

[i] Step 1: Checking prerequisites...
[✓] Binary found: /path/to/build/ConnectServer
[✓] Config file found
[i] TCP Port: 44405
[i] UDP Port: 55601

[i] Step 2: Checking if ports are available...
[✓] TCP port 44405 is available
[✓] UDP port 55601 is available

[i] Step 3: Starting ConnectServer...
[i] ConnectServer started with PID: 12345
[✓] ConnectServer process is running

[i] Step 4: Testing TCP port 44405...
[i] Waiting for TCP port 44405 to open (timeout: 10s)...
[✓] TCP port 44405 is LISTENING
[✓] TCP connection test successful

[i] Step 5: Testing UDP port 55601...
[i] Waiting for UDP port 55601 to open (timeout: 10s)...
[✓] UDP port 55601 is OPEN
[i] UDP packet sent (server may reject invalid protocol)

[i] Step 6: Current port status...
...

[i] Step 7: Recent log entries...
...

[i] Step 8: Cleanup...
[✓] TCP port 44405 is now closed
[✓] UDP port 55601 is now closed

=== Test Summary ===
[✓] All tests passed! TCP and UDP ports opened and closed correctly.
```

### Failed Test
If a test fails, you'll see error messages like:
```
[✗] TCP port 44405 is NOT listening after 10 seconds
[i] Check log file: /tmp/connectserver_test.log
```

## Troubleshooting

### Port Already in Use
```
[✗] TCP port 44405 is already in use (PID: 12345)
```
**Solution:** Stop the existing process or change the port in `config/ConnectServer.ini`

### Binary Not Found
```
[✗] ConnectServer binary not found at: /path/to/build/ConnectServer
```
**Solution:** Build the project first:
```bash
cd build
cmake ..
make
```

### Process Dies Immediately
```
[✗] ConnectServer process died immediately
```
**Solution:** Check the log file for error messages. Common causes:
- Missing configuration files
- Permission issues
- Missing dependencies

### Network Tools Not Available
```
[!] No network diagnostic tools found (netstat, ss, or lsof)
```
**Solution:** Install network tools:
- **Debian/Ubuntu:** `sudo apt-get install net-tools`
- **RHEL/CentOS:** `sudo yum install net-tools`
- **macOS:** Tools are usually pre-installed

## Manual Port Testing

If the automated scripts don't work, you can manually test ports:

### Check if Port is Listening
```bash
# Using netstat
netstat -an | grep 44405

# Using ss
ss -ln | grep 44405

# Using lsof
lsof -i tcp:44405
lsof -i udp:55601
```

### Test TCP Connection
```bash
# Using telnet
telnet 127.0.0.1 44405

# Using netcat
nc -zv 127.0.0.1 44405

# Using Python
python3 -c "import socket; s=socket.socket(); s.connect(('127.0.0.1', 44405)); print('Connected')"
```

### Test UDP Port
```bash
# Using netcat
echo "test" | nc -u 127.0.0.1 55601

# Using Python
python3 -c "import socket; s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.sendto(b'test', ('127.0.0.1', 55601))"
```

## Log Files

Test logs are written to:
- `/tmp/connectserver_test.log` - Server output during test

Server logs (if configured) are written to:
- `ConnectServer.log` - Main server log (in project directory)

## Exit Codes

- `0` - All tests passed
- `1` - One or more tests failed
- `130` - Test interrupted by user (Ctrl+C)

## Integration with CI/CD

These scripts can be integrated into CI/CD pipelines:

```bash
# In your CI script
cd /path/to/ConnectServerCross
./scripts/test_ports.py
if [ $? -eq 0 ]; then
    echo "Port tests passed"
else
    echo "Port tests failed"
    exit 1
fi
```

## Notes

- Tests are designed to be non-destructive and clean up after themselves
- The server is only run for a short time during testing
- Ports are verified to be closed after the test completes
- Root/sudo privileges are NOT required for these tests
- Tests work on both Linux and macOS (with appropriate tools installed)
