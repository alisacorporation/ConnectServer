# Testing TCP/UDP Ports on Unix

Quick guide to test if ConnectServer correctly opens TCP and UDP ports on Unix systems (Linux/macOS).

## Prerequisites

1. **Build the project first:**
   ```bash
   cd ConnectServerCross
   mkdir -p build && cd build
   cmake ..
   make
   cd ..
   ```

2. **Ensure configuration files exist:**
   ```bash
   # Copy example configs if needed
   cp config/ConnectServer.ini.example config/ConnectServer.ini
   cp config/ServerList.dat.example config/ServerList.dat
   ```

## Quick Test

### Option 1: Python Script (Recommended)

```bash
# Make executable
chmod +x scripts/test_ports.py

# Run test
./scripts/test_ports.py
```

**Advantages:**
- No additional dependencies (Python 3.6+ only)
- Better cross-platform compatibility
- More detailed diagnostics
- Cleaner output

### Option 2: Bash Script

```bash
# Make executable
chmod +x scripts/test_ports.sh

# Run test
./scripts/test_ports.sh
```

**Requirements:**
- Bash shell
- Network tools: `netstat`, `ss`, or `lsof`

## What Gets Tested

1. ✅ TCP port opens and accepts connections
2. ✅ UDP port opens and receives packets
3. ✅ Ports close properly on shutdown
4. ✅ No port conflicts before starting
5. ✅ Server process starts successfully
6. ✅ Server logs show correct initialization

## Expected Results

### Success Output

```
=== ConnectServer Port Test Script ===

[✓] Binary found: build/ConnectServer
[✓] Config file found
[i] TCP Port: 44405
[i] UDP Port: 55601

[✓] TCP port 44405 is available
[✓] UDP port 55601 is available

[✓] ConnectServer process is running
[✓] TCP port 44405 is LISTENING
[✓] TCP connection test successful
[✓] UDP port 55601 is OPEN

[✓] TCP port 44405 is now closed
[✓] UDP port 55601 is now closed

=== Test Summary ===
[✓] All tests passed! TCP and UDP ports opened and closed correctly.
```

### Common Issues

#### Port Already in Use
```
[✗] TCP port 44405 is already in use (PID: 12345)
```
**Fix:** Stop the existing process or change port in config

#### Binary Not Found
```
[✗] ConnectServer binary not found
```
**Fix:** Build the project first (see Prerequisites)

#### Process Dies Immediately
```
[✗] ConnectServer process died immediately
```
**Fix:** Check `/tmp/connectserver_test.log` for errors

## Manual Testing

If automated scripts don't work, test manually:

### 1. Start Server
```bash
cd ConnectServerCross
./build/ConnectServer
```

### 2. Check Ports (in another terminal)

**Using netstat:**
```bash
netstat -an | grep 44405  # TCP
netstat -an | grep 55601  # UDP
```

**Using ss:**
```bash
ss -ln | grep 44405  # TCP
ss -ln | grep 55601  # UDP
```

**Using lsof:**
```bash
lsof -i tcp:44405
lsof -i udp:55601
```

### 3. Test TCP Connection

**Using telnet:**
```bash
telnet 127.0.0.1 44405
```

**Using netcat:**
```bash
nc -zv 127.0.0.1 44405
```

**Using Python:**
```bash
python3 -c "import socket; s=socket.socket(); s.connect(('127.0.0.1', 44405)); print('Connected')"
```

### 4. Test UDP Port

**Using netcat:**
```bash
echo "test" | nc -u 127.0.0.1 55601
```

**Using Python:**
```bash
python3 -c "import socket; s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.sendto(b'test', ('127.0.0.1', 55601))"
```

## Troubleshooting

### Install Network Tools

**Ubuntu/Debian:**
```bash
sudo apt-get install net-tools netcat
```

**CentOS/RHEL:**
```bash
sudo yum install net-tools nc
```

**macOS:**
```bash
# Usually pre-installed
# If needed: brew install netcat
```

### Check Firewall

**Linux (iptables):**
```bash
sudo iptables -L -n | grep 44405
sudo iptables -L -n | grep 55601
```

**Linux (firewalld):**
```bash
sudo firewall-cmd --list-ports
```

**macOS:**
```bash
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --getglobalstate
```

### View Server Logs

**During test:**
```bash
tail -f /tmp/connectserver_test.log
```

**Normal operation:**
```bash
tail -f ConnectServer.log
```

## Configuration

Default ports from `config/ConnectServer.ini`:
```ini
[ConnectServerInfo]
ConnectServerPortTCP=44405
ConnectServerPortUDP=55601
```

To change ports:
1. Edit `config/ConnectServer.ini`
2. Update both TCP and UDP port values
3. Re-run the test

## CI/CD Integration

Add to your CI pipeline:

```yaml
# .github/workflows/test.yml
- name: Test Ports
  run: |
    cd ConnectServerCross
    ./scripts/test_ports.py
```

Or in a shell script:
```bash
#!/bin/bash
cd ConnectServerCross
./scripts/test_ports.py
if [ $? -ne 0 ]; then
    echo "Port tests failed!"
    exit 1
fi
echo "Port tests passed!"
```

## Additional Resources

- **Detailed Documentation:** [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md)
- **Main README:** [README.md](README.md)
- **Build Instructions:** [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
- **Testing Guide:** [docs/TESTING.md](docs/TESTING.md)

## Support

If tests fail:
1. Check `/tmp/connectserver_test.log` for errors
2. Verify build completed successfully
3. Ensure configuration files are valid
4. Check no other process is using the ports
5. Verify network tools are installed

For more help, see the detailed documentation in `scripts/README_PORT_TESTING.md`.
