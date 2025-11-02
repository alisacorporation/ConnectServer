# Testing Documentation Index

Quick navigation to all testing-related documentation.

## 🚀 Start Here

**New to port testing?** Start with the Quick Start guide:
- **[scripts/QUICK_START.md](scripts/QUICK_START.md)** - 3-step guide to run your first test

**Just want to run the test?**
```bash
chmod +x RUN_PORT_TEST.sh
./RUN_PORT_TEST.sh
```

## 📚 Documentation by Purpose

### For Quick Testing
1. **[scripts/QUICK_START.md](scripts/QUICK_START.md)**
   - 3-step quick start
   - Visual test flow
   - Common errors and fixes
   - **Read time:** 3 minutes

2. **[RUN_PORT_TEST.sh](RUN_PORT_TEST.sh)**
   - One-command test runner
   - Auto-builds if needed
   - Auto-creates configs
   - **Just run it!**

### For Unix/Linux Users
1. **[TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md)**
   - Prerequisites checklist
   - Quick test commands
   - Manual testing methods
   - Troubleshooting guide
   - **Read time:** 10 minutes

2. **[scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md)**
   - Comprehensive testing guide
   - Detailed explanations
   - All test scenarios
   - CI/CD integration
   - **Read time:** 20 minutes

### For Windows Users
1. **[scripts/test_ports_windows.md](scripts/test_ports_windows.md)**
   - WSL usage guide
   - Git Bash methods
   - PowerShell scripts
   - Native Windows tools
   - **Read time:** 15 minutes

### For Technical Details
1. **[PORT_TESTING_SUMMARY.md](PORT_TESTING_SUMMARY.md)**
   - Implementation details
   - Test flow documentation
   - Technical specifications
   - Compatibility matrix
   - **Read time:** 15 minutes

2. **[TEST_IMPLEMENTATION_COMPLETE.md](TEST_IMPLEMENTATION_COMPLETE.md)**
   - Complete deliverables list
   - Success criteria
   - Verification checklist
   - Example outputs
   - **Read time:** 10 minutes

## 🛠️ Test Scripts

### Python Script (Recommended)
- **File:** [scripts/test_ports.py](scripts/test_ports.py)
- **Language:** Python 3.6+
- **Dependencies:** None
- **Usage:** `./scripts/test_ports.py`
- **Best for:** All platforms with Python

### Bash Script
- **File:** [scripts/test_ports.sh](scripts/test_ports.sh)
- **Language:** Bash 4.0+
- **Dependencies:** netstat/ss/lsof
- **Usage:** `./scripts/test_ports.sh`
- **Best for:** Unix/Linux systems

### Test Runner
- **File:** [RUN_PORT_TEST.sh](RUN_PORT_TEST.sh)
- **Language:** Bash
- **Usage:** `./RUN_PORT_TEST.sh`
- **Features:** Auto-build, auto-config, auto-test

## 📖 By User Type

### I'm a Developer
1. Read: [scripts/QUICK_START.md](scripts/QUICK_START.md)
2. Run: `./RUN_PORT_TEST.sh`
3. Reference: [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md)

### I'm a DevOps Engineer
1. Read: [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md)
2. Check: CI/CD integration section
3. Reference: [PORT_TESTING_SUMMARY.md](PORT_TESTING_SUMMARY.md)

### I'm on Windows
1. Read: [scripts/test_ports_windows.md](scripts/test_ports_windows.md)
2. Choose: WSL, Git Bash, or PowerShell method
3. Run: Appropriate test method

### I Need Technical Details
1. Read: [PORT_TESTING_SUMMARY.md](PORT_TESTING_SUMMARY.md)
2. Review: [TEST_IMPLEMENTATION_COMPLETE.md](TEST_IMPLEMENTATION_COMPLETE.md)
3. Check: Source code in scripts/

## 🎯 By Task

### "I want to test ports now"
```bash
./RUN_PORT_TEST.sh
```
Or: [scripts/QUICK_START.md](scripts/QUICK_START.md)

### "I need to understand what's tested"
→ [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md) - Section: "What Gets Tested"

### "I have an error"
→ [scripts/QUICK_START.md](scripts/QUICK_START.md) - Section: "Common Errors"
→ [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md) - Section: "Troubleshooting"

### "I want to integrate with CI/CD"
→ [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md) - Section: "CI/CD Integration"

### "I'm on Windows"
→ [scripts/test_ports_windows.md](scripts/test_ports_windows.md)

### "I need manual testing steps"
→ [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md) - Section: "Manual Testing"

### "I want technical implementation details"
→ [PORT_TESTING_SUMMARY.md](PORT_TESTING_SUMMARY.md)

## 📊 Documentation Map

```
TESTING_INDEX.md (You are here)
    │
    ├─── Quick Start
    │    ├─ scripts/QUICK_START.md ⭐ Start here!
    │    └─ RUN_PORT_TEST.sh (One-command runner)
    │
    ├─── User Guides
    │    ├─ TESTING_PORTS_UNIX.md (Unix/Linux)
    │    └─ scripts/test_ports_windows.md (Windows)
    │
    ├─── Detailed Documentation
    │    └─ scripts/README_PORT_TESTING.md (Complete guide)
    │
    └─── Technical Documentation
         ├─ PORT_TESTING_SUMMARY.md (Implementation)
         └─ TEST_IMPLEMENTATION_COMPLETE.md (Deliverables)
```

## 🔍 Quick Reference

### Test Commands
```bash
# Easiest way (auto-everything)
./RUN_PORT_TEST.sh

# Python script (recommended)
./scripts/test_ports.py

# Bash script (alternative)
./scripts/test_ports.sh
```

### Manual Testing
```bash
# Start server
./build/ConnectServer

# Check ports (another terminal)
netstat -an | grep 44405
netstat -an | grep 55601

# Test TCP
telnet localhost 44405

# Test UDP
echo "test" | nc -u localhost 55601
```

### Configuration
```bash
# Config location
config/ConnectServer.ini

# Default ports
TCP: 44405
UDP: 55601
```

### Log Files
```bash
# Test log
/tmp/connectserver_test.log

# Server log
ConnectServer.log
```

## 📞 Getting Help

### Quick Issues
1. Check: [scripts/QUICK_START.md](scripts/QUICK_START.md) - "Common Errors"
2. Check: Test log at `/tmp/connectserver_test.log`

### Build Issues
1. Check: [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
2. Verify: CMake and dependencies installed

### Port Issues
1. Check: Port not in use: `lsof -i tcp:44405`
2. Check: Firewall settings
3. Try: Different ports in config

### Platform Issues
- **Linux:** [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md)
- **Windows:** [scripts/test_ports_windows.md](scripts/test_ports_windows.md)
- **macOS:** [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md)

## ✅ Checklist Before Testing

- [ ] Project built (`build/ConnectServer` exists)
- [ ] Config files exist (`config/ConnectServer.ini`)
- [ ] Python 3.6+ or Bash 4.0+ installed
- [ ] Network tools available (netstat/ss/lsof)
- [ ] Ports not in use (44405, 55601)
- [ ] Test scripts executable (`chmod +x`)

## 🎓 Learning Path

1. **Beginner** (5 min)
   - Read: [scripts/QUICK_START.md](scripts/QUICK_START.md)
   - Run: `./RUN_PORT_TEST.sh`

2. **Intermediate** (15 min)
   - Read: [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md)
   - Try: Manual testing methods

3. **Advanced** (30 min)
   - Read: [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md)
   - Read: [PORT_TESTING_SUMMARY.md](PORT_TESTING_SUMMARY.md)
   - Setup: CI/CD integration

4. **Expert** (60 min)
   - Review: All documentation
   - Study: Test script source code
   - Customize: For your needs

## 🔗 External Resources

- **Main README:** [README.md](README.md)
- **Build Guide:** [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
- **Project Status:** [PROJECT_STATUS.md](PROJECT_STATUS.md)

## 📝 Summary

| Document | Purpose | Time | Audience |
|----------|---------|------|----------|
| scripts/QUICK_START.md | Quick start | 3 min | Everyone |
| RUN_PORT_TEST.sh | One-command test | 0 min | Everyone |
| TESTING_PORTS_UNIX.md | Unix guide | 10 min | Unix users |
| scripts/test_ports_windows.md | Windows guide | 15 min | Windows users |
| scripts/README_PORT_TESTING.md | Complete guide | 20 min | All users |
| PORT_TESTING_SUMMARY.md | Technical details | 15 min | Developers |
| TEST_IMPLEMENTATION_COMPLETE.md | Deliverables | 10 min | Reviewers |

---

**Quick Start:** Run `./RUN_PORT_TEST.sh` or read [scripts/QUICK_START.md](scripts/QUICK_START.md)

**Need Help?** Start with [scripts/QUICK_START.md](scripts/QUICK_START.md) → [TESTING_PORTS_UNIX.md](TESTING_PORTS_UNIX.md) → [scripts/README_PORT_TESTING.md](scripts/README_PORT_TESTING.md)
