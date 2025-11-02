# Windows Port Testing Fix

## Issue
The port testing scripts were looking for Unix-style binary paths and didn't account for Windows-specific differences:
- Binary name: `ConnectServer` vs `ConnectServer.exe`
- Build location: `build/` vs `build/Release/` or `build/Debug/`
- Temp directory: `/tmp/` vs `%TEMP%`

## Solution

### Python Script (`scripts/test_ports.py`)
Added Windows detection and path handling:

```python
# Detect platform
is_windows = platform.system() == 'Windows'

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
```

### Bash Script (`scripts/test_ports.sh`)
Added Windows (Git Bash/MSYS) detection:

```bash
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
```

## Usage on Windows

### Option 1: Python (Recommended)
```powershell
# PowerShell
python scripts\test_ports.py
```

### Option 2: Git Bash
```bash
# Git Bash
sh scripts/test_ports.sh
```

### Option 3: WSL
```bash
# WSL
./scripts/test_ports.py
```

## Build Locations Searched

The scripts now search in this order:
1. `build/Release/ConnectServer.exe` (CMake Release build)
2. `build/Debug/ConnectServer.exe` (CMake Debug build)
3. `build/ConnectServer.exe` (Direct build)

## Error Messages

### Before (Confusing)
```
[✗] ConnectServer binary not found at: /c/Users/.../build/ConnectServer
[i] Please build the project first using: cd build && cmake .. && make
```

### After (Clear)
```
[✗] ConnectServer binary not found at: C:\Users\...\build\Release\ConnectServer.exe
[i] Please build the project first using: cd build && cmake .. && cmake --build . --config Release
[i] Searched locations:
[i]   - C:\Users\...\build\Release\ConnectServer.exe
[i]   - C:\Users\...\build\Debug\ConnectServer.exe
[i]   - C:\Users\...\build\ConnectServer.exe
```

## Log File Location

### Unix/Linux
```
/tmp/connectserver_test.log
```

### Windows
```
%TEMP%\connectserver_test.log
```
(Usually `C:\Users\<username>\AppData\Local\Temp\connectserver_test.log`)

## Testing

To verify the fix works:

1. **Build the project:**
   ```powershell
   cd build
   cmake ..
   cmake --build . --config Release
   cd ..
   ```

2. **Run the test:**
   ```powershell
   python scripts\test_ports.py
   ```

3. **Expected output:**
   ```
   === ConnectServer Port Test Script ===

   [i] Step 1: Checking prerequisites...
   [✓] Binary found: C:\Users\...\build\Release\ConnectServer.exe
   [✓] Config file found
   [i] TCP Port: 44405
   [i] UDP Port: 55601
   ...
   ```

## Platform Detection

### Python
```python
import platform
is_windows = platform.system() == 'Windows'
```

### Bash
```bash
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" || "$OSTYPE" == "cygwin" ]]; then
    IS_WINDOWS=1
fi
```

## Compatibility Matrix

| Platform | Python Script | Bash Script | Status |
|----------|--------------|-------------|--------|
| Windows PowerShell | ✅ | ❌ | Python works |
| Windows Git Bash | ✅ | ✅ | Both work |
| Windows WSL | ✅ | ✅ | Both work |
| Linux | ✅ | ✅ | Both work |
| macOS | ✅ | ✅ | Both work |

## Changes Made

### Files Modified
1. `scripts/test_ports.py` - Added Windows detection and path handling
2. `scripts/test_ports.sh` - Added Windows (Git Bash) detection

### Lines Changed
- **Python:** ~15 lines added/modified
- **Bash:** ~20 lines added/modified

## Testing Checklist

- [x] Python script detects Windows
- [x] Python script finds Release build
- [x] Python script finds Debug build
- [x] Python script uses correct temp directory
- [x] Bash script detects Git Bash/MSYS
- [x] Bash script finds .exe files
- [x] Error messages show correct paths
- [x] Error messages show searched locations

## Known Limitations

1. **PowerShell native:** Bash script won't work in PowerShell (use Python)
2. **CMD.exe:** Neither script works well in CMD (use PowerShell + Python)
3. **Cygwin:** Should work but not extensively tested

## Recommendations

For Windows users:
1. **Best:** Use Python script with PowerShell
2. **Alternative:** Use Git Bash with either script
3. **WSL:** Use either script (full Unix environment)

---

**Status:** ✅ Fixed
**Date:** November 2, 2024
**Tested:** Windows 10/11 with PowerShell and Git Bash
