# Quick Start: Port Testing

Test TCP/UDP ports in 3 simple steps!

## 🚀 Quick Test

### Easiest Way (Any Platform)
```bash
# One command does everything!
sh RUN_PORT_TEST.sh
```
This script:
- Auto-detects Windows/Unix
- Auto-builds if needed
- Auto-creates configs
- Runs the test

### Manual Way
```bash
# 1. Make executable (first time only, Unix)
chmod +x scripts/test_ports.py

# 2. Run test
./scripts/test_ports.py

# Done! ✅
```

## 📋 Prerequisites

Before running the test, ensure:

1. **Project is built:**
   ```bash
   mkdir -p build && cd build
   cmake .. && make
   cd ..
   ```

2. **Config files exist:**
   ```bash
   # Check if configs exist
   ls config/ConnectServer.ini
   ls config/ServerList.dat
   
   # If not, copy examples:
   cp config/ConnectServer.ini.example config/ConnectServer.ini
   cp config/ServerList.dat.example config/ServerList.dat
   ```

3. **Python 3.6+ installed:**
   ```bash
   python3 --version
   ```

## 🎯 What Gets Tested

```
┌─────────────────────────────────────────┐
│  1. Check Prerequisites                 │
│     ├─ Binary exists?                   │
│     ├─ Config readable?                 │
│     └─ Network tools available?         │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  2. Check Port Availability             │
│     ├─ TCP port 44405 free?             │
│     └─ UDP port 55601 free?             │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  3. Start ConnectServer                 │
│     ├─ Launch process                   │
│     ├─ Verify running                   │
│     └─ Log to /tmp/connectserver_test.log│
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  4. Test TCP Port (44405)               │
│     ├─ Wait for LISTEN (10s timeout)    │
│     ├─ Verify listening                 │
│     └─ Test connection                  │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  5. Test UDP Port (55601)               │
│     ├─ Wait for OPEN (10s timeout)      │
│     ├─ Verify open                      │
│     └─ Send test packet                 │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  6. Show Diagnostics                    │
│     ├─ Port status                      │
│     ├─ Process info                     │
│     └─ Recent logs                      │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  7. Cleanup                             │
│     ├─ Stop server                      │
│     ├─ Verify ports closed              │
│     └─ Report results                   │
└─────────────────────────────────────────┘
```

## ✅ Success Output

```
=== ConnectServer Port Test Script ===

[i] Step 1: Checking prerequisites...
[✓] Binary found: build/ConnectServer
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
[✓] TCP port 44405 is LISTENING
[✓] TCP connection test successful

[i] Step 5: Testing UDP port 55601...
[✓] UDP port 55601 is OPEN

[i] Step 8: Cleanup...
[✓] TCP port 44405 is now closed
[✓] UDP port 55601 is now closed

=== Test Summary ===
[✓] All tests passed! TCP and UDP ports opened and closed correctly.
```

## ❌ Common Errors

### Error: Binary not found
```
[✗] ConnectServer binary not found at: build/ConnectServer
```
**Fix:**
```bash
mkdir -p build && cd build
cmake .. && make
cd ..
```

### Error: Port already in use
```
[✗] TCP port 44405 is already in use (PID: 12345)
```
**Fix:**
```bash
# Find and kill process
lsof -i tcp:44405
kill -9 <PID>

# Or change port in config
nano config/ConnectServer.ini
```

### Error: Config not found
```
[!] Config file not found at: config/ConnectServer.ini
```
**Fix:**
```bash
cp config/ConnectServer.ini.example config/ConnectServer.ini
cp config/ServerList.dat.example config/ServerList.dat
```

## 🔧 Alternative Methods

### Using Bash Script
```bash
chmod +x scripts/test_ports.sh
./scripts/test_ports.sh
```

### Manual Testing
```bash
# Terminal 1: Start server
./build/ConnectServer

# Terminal 2: Check ports
netstat -an | grep 44405
netstat -an | grep 55601

# Test TCP connection
telnet localhost 44405
```

## 📖 More Information

- **Detailed Guide:** [README_PORT_TESTING.md](README_PORT_TESTING.md)
- **Unix Testing:** [../TESTING_PORTS_UNIX.md](../TESTING_PORTS_UNIX.md)
- **Windows Testing:** [test_ports_windows.md](test_ports_windows.md)
- **Full Summary:** [../PORT_TESTING_SUMMARY.md](../PORT_TESTING_SUMMARY.md)

## 💡 Tips

1. **Run from project root:**
   ```bash
   cd ConnectServerCross
   ./scripts/test_ports.py
   ```

2. **Check logs if test fails:**
   ```bash
   cat /tmp/connectserver_test.log
   ```

3. **Test specific ports:**
   Edit `config/ConnectServer.ini` before running test

4. **Clean up stuck processes:**
   ```bash
   pkill -9 ConnectServer
   ```

5. **Install missing tools:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install net-tools netcat python3
   
   # macOS
   brew install netcat
   ```

## 🎓 Understanding the Output

| Symbol | Meaning |
|--------|---------|
| `[✓]` | Success - test passed |
| `[✗]` | Error - test failed |
| `[!]` | Warning - non-critical issue |
| `[i]` | Info - status message |

## 🚨 Emergency Stop

If server doesn't stop properly:
```bash
# Find process
ps aux | grep ConnectServer

# Force kill
pkill -9 ConnectServer

# Or by PID
kill -9 <PID>
```

## 📞 Need Help?

1. Check `/tmp/connectserver_test.log`
2. Verify build: `ls -la build/ConnectServer`
3. Check config: `cat config/ConnectServer.ini`
4. Test manually: `./build/ConnectServer`
5. Review detailed docs in `scripts/README_PORT_TESTING.md`

---

**Ready to test?** Run: `./scripts/test_ports.py` 🚀
