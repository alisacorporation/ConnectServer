# Testing Ports on Windows

The port testing scripts now support Windows! Both Python and Bash scripts automatically detect Windows and handle paths correctly.

## Method 1: PowerShell + Python (Recommended) ⭐

The easiest and most reliable method on Windows:

```powershell
# Navigate to project
cd "C:\Users\root\Desktop\Season 1 - XeoN\Source\MuServer\ConnectServerCross"

# Run the test
python scripts\test_ports.py
```

**Features:**
- ✅ Automatically detects Windows
- ✅ Finds ConnectServer.exe in Release/Debug folders
- ✅ Uses Windows temp directory
- ✅ Shows correct build instructions

## Method 2: Git Bash

If you have Git for Windows installed:

```bash
# Open Git Bash
# Navigate to project
cd "/c/Users/root/Desktop/Season 1 - XeoN/Source/MuServer/ConnectServerCross"

# Run Python test (recommended)
python scripts/test_ports.py

# Or run Bash test
sh scripts/test_ports.sh
```

**Note:** The bash script now detects Git Bash and looks for .exe files automatically!

## Method 3: WSL (Windows Subsystem for Linux)

If you have WSL installed:

```bash
# Open WSL terminal
wsl

# Navigate to project
cd /mnt/c/Users/root/Desktop/Season\ 1\ -\ XeoN/Source/MuServer/ConnectServerCross

# Make scripts executable
chmod +x scripts/test_ports.py scripts/test_ports.sh

# Run Python test (recommended)
./scripts/test_ports.py

# Or run Bash test
./scripts/test_ports.sh
```

## Method 3: Manual Testing with PowerShell

### Check if Ports are in Use

```powershell
# Check TCP port
netstat -an | Select-String "44405"

# Check UDP port
netstat -an | Select-String "55601"

# Or use Test-NetConnection (TCP only)
Test-NetConnection -ComputerName localhost -Port 44405
```

### Start Server and Test

1. **Start ConnectServer:**
   ```powershell
   cd "C:\Users\root\Desktop\Season 1 - XeoN\Source\MuServer\ConnectServerCross"
   .\build\Release\ConnectServer.exe
   ```

2. **In another PowerShell window, check ports:**
   ```powershell
   # Check listening ports
   netstat -an | Select-String "LISTENING" | Select-String "44405"
   netstat -an | Select-String "55601"
   
   # Get process using the port
   Get-NetTCPConnection -LocalPort 44405 | Select-Object LocalPort, State, OwningProcess
   Get-NetUDPEndpoint -LocalPort 55601 | Select-Object LocalPort, OwningProcess
   ```

3. **Test TCP connection:**
   ```powershell
   Test-NetConnection -ComputerName localhost -Port 44405
   ```

4. **Test with telnet (if installed):**
   ```powershell
   telnet localhost 44405
   ```

### PowerShell Test Script

Create a file `test_ports.ps1`:

```powershell
# test_ports.ps1 - Simple port test for Windows
param(
    [int]$TcpPort = 44405,
    [int]$UdpPort = 55601
)

Write-Host "=== ConnectServer Port Test (Windows) ===" -ForegroundColor Cyan
Write-Host ""

# Check if ports are in use
Write-Host "Checking if ports are available..." -ForegroundColor Yellow

$tcpInUse = Get-NetTCPConnection -LocalPort $TcpPort -ErrorAction SilentlyContinue
if ($tcpInUse) {
    Write-Host "[X] TCP port $TcpPort is already in use" -ForegroundColor Red
    exit 1
}
Write-Host "[OK] TCP port $TcpPort is available" -ForegroundColor Green

$udpInUse = Get-NetUDPEndpoint -LocalPort $UdpPort -ErrorAction SilentlyContinue
if ($udpInUse) {
    Write-Host "[X] UDP port $UdpPort is already in use" -ForegroundColor Red
    exit 1
}
Write-Host "[OK] UDP port $UdpPort is available" -ForegroundColor Green

Write-Host ""
Write-Host "Starting ConnectServer..." -ForegroundColor Yellow

# Start server
$serverPath = ".\build\Release\ConnectServer.exe"
if (-not (Test-Path $serverPath)) {
    $serverPath = ".\build\Debug\ConnectServer.exe"
}

if (-not (Test-Path $serverPath)) {
    Write-Host "[X] ConnectServer.exe not found" -ForegroundColor Red
    Write-Host "    Please build the project first" -ForegroundColor Yellow
    exit 1
}

$process = Start-Process -FilePath $serverPath -PassThru -WindowStyle Hidden

Write-Host "[OK] Server started with PID: $($process.Id)" -ForegroundColor Green

# Wait for server to initialize
Start-Sleep -Seconds 3

# Check if process is still running
if ($process.HasExited) {
    Write-Host "[X] Server process died immediately" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Testing TCP port..." -ForegroundColor Yellow

# Test TCP port
$tcpTest = Test-NetConnection -ComputerName localhost -Port $TcpPort -WarningAction SilentlyContinue
if ($tcpTest.TcpTestSucceeded) {
    Write-Host "[OK] TCP port $TcpPort is listening" -ForegroundColor Green
} else {
    Write-Host "[X] TCP port $TcpPort is NOT listening" -ForegroundColor Red
}

Write-Host ""
Write-Host "Testing UDP port..." -ForegroundColor Yellow

# Check UDP port
$udpCheck = Get-NetUDPEndpoint -LocalPort $UdpPort -ErrorAction SilentlyContinue
if ($udpCheck) {
    Write-Host "[OK] UDP port $UdpPort is open" -ForegroundColor Green
} else {
    Write-Host "[X] UDP port $UdpPort is NOT open" -ForegroundColor Red
}

Write-Host ""
Write-Host "Port status:" -ForegroundColor Yellow
netstat -an | Select-String "$TcpPort|$UdpPort"

Write-Host ""
Write-Host "Stopping server..." -ForegroundColor Yellow
Stop-Process -Id $process.Id -Force

Start-Sleep -Seconds 2

Write-Host "[OK] Server stopped" -ForegroundColor Green
Write-Host ""
Write-Host "=== Test Complete ===" -ForegroundColor Cyan
```

Run it:
```powershell
.\scripts\test_ports.ps1
```

## Method 4: Using Python on Windows

If you have Python installed on Windows:

```powershell
# Navigate to project
cd "C:\Users\root\Desktop\Season 1 - XeoN\Source\MuServer\ConnectServerCross"

# Run the Python test script
python scripts\test_ports.py
```

Note: The script will work on Windows but some features may be limited.

## Method 5: Using netcat for Windows

Download netcat for Windows and test:

```powershell
# Test TCP connection
nc -zv localhost 44405

# Send UDP packet
echo "test" | nc -u localhost 55601
```

## Troubleshooting

### Enable Telnet Client (Windows)

```powershell
# Run as Administrator
Enable-WindowsOptionalFeature -Online -FeatureName TelnetClient
```

### Check Windows Firewall

```powershell
# Check firewall rules
Get-NetFirewallRule | Where-Object {$_.LocalPort -eq 44405 -or $_.LocalPort -eq 55601}

# Add firewall rules if needed
New-NetFirewallRule -DisplayName "ConnectServer TCP" -Direction Inbound -LocalPort 44405 -Protocol TCP -Action Allow
New-NetFirewallRule -DisplayName "ConnectServer UDP" -Direction Inbound -LocalPort 55601 -Protocol UDP -Action Allow
```

### View Running Processes on Ports

```powershell
# Find process using TCP port
Get-Process -Id (Get-NetTCPConnection -LocalPort 44405).OwningProcess

# Find process using UDP port
Get-Process -Id (Get-NetUDPEndpoint -LocalPort 55601).OwningProcess
```

## Recommended Approach

For Windows development, we recommend:

1. **Use WSL2** - Best compatibility with Unix scripts
2. **Use Python script** - Works on both Windows and Unix
3. **Use PowerShell script** - Native Windows solution

## Additional Resources

- [WSL Installation Guide](https://docs.microsoft.com/en-us/windows/wsl/install)
- [Git for Windows](https://gitforwindows.org/)
- [Python for Windows](https://www.python.org/downloads/windows/)
