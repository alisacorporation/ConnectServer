# Windows API to Linux/Cross-Platform Mapping Guide

## Overview

This document provides detailed mappings from Windows-specific APIs used in the original ConnectServer to cross-platform alternatives using C++17 standard library and Boost.

## Networking

### Socket Creation

**Windows**:
```cpp
SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::ip::tcp::socket socket(io_context);
```

---

### Socket Binding

**Windows**:
```cpp
SOCKADDR_IN addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = htonl(INADDR_ANY);
addr.sin_port = htons(port);
bind(socket, (sockaddr*)&addr, sizeof(addr));
listen(socket, 5);
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
acceptor.open(endpoint.protocol());
acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
acceptor.bind(endpoint);
acceptor.listen();
```

---

### Async Accept with Condition

**Windows**:
```cpp
int CALLBACK AcceptCondition(LPWSABUF lpCallerId, ...) {
    SOCKADDR_IN* addr = (SOCKADDR_IN*)lpCallerId->buf;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
    return check_ip(ip) ? CF_ACCEPT : CF_REJECT;
}

SOCKET client = WSAAccept(listen_socket, ..., AcceptCondition, ...);
```

**Cross-Platform (Boost.Asio)**:
```cpp
acceptor.async_accept(socket, [](const boost::system::error_code& error) {
    if (!error) {
        std::string ip = socket.remote_endpoint().address().to_string();
        if (check_ip(ip)) {
            // Accept connection
        } else {
            socket.close();
        }
    }
});
```

---

### Async Receive (IOCP)

**Windows**:
```cpp
WSAOVERLAPPED overlapped = {};
WSABUF wsabuf;
wsabuf.buf = (char*)buffer;
wsabuf.len = size;
DWORD flags = 0;
WSARecv(socket, &wsabuf, 1, NULL, &flags, &overlapped, NULL);

// In worker thread
GetQueuedCompletionStatus(iocp, &bytes, &key, &overlapped, INFINITE);
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::async_read(socket,
    boost::asio::buffer(buffer, size),
    boost::asio::transfer_at_least(1),
    [](const boost::system::error_code& error, size_t bytes_transferred) {
        // Handle completion
    });
```

---

### Async Send (IOCP)

**Windows**:
```cpp
WSAOVERLAPPED overlapped = {};
WSABUF wsabuf;
wsabuf.buf = (char*)buffer;
wsabuf.len = size;
WSASend(socket, &wsabuf, 1, NULL, 0, &overlapped, NULL);
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::async_write(socket,
    boost::asio::buffer(buffer, size),
    [](const boost::system::error_code& error, size_t bytes_transferred) {
        // Handle completion
    });
```

---

### UDP Socket

**Windows**:
```cpp
SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
SOCKADDR_IN addr = {};
addr.sin_family = AF_INET;
addr.sin_port = htons(port);
bind(sock, (sockaddr*)&addr, sizeof(addr));

// Receive
SOCKADDR_IN from;
int fromlen = sizeof(from);
recvfrom(sock, buffer, size, 0, (sockaddr*)&from, &fromlen);

// Send
sendto(sock, buffer, size, 0, (sockaddr*)&to_addr, sizeof(to_addr));
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::ip::udp::socket socket(io_context);
boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
socket.open(endpoint.protocol());
socket.bind(endpoint);

// Async receive
socket.async_receive_from(
    boost::asio::buffer(buffer, size),
    remote_endpoint,
    [](const boost::system::error_code& error, size_t bytes) {});

// Async send
socket.async_send_to(
    boost::asio::buffer(buffer, size),
    remote_endpoint,
    [](const boost::system::error_code& error, size_t bytes) {});
```

---

## Threading

### Thread Creation

**Windows**:
```cpp
HANDLE thread = CreateThread(NULL, 0, ThreadFunc, param, 0, NULL);
SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
```

**Cross-Platform (C++11)**:
```cpp
std::thread thread(ThreadFunc, param);

// Priority setting (platform-specific)
#ifdef __linux__
    sched_param sch;
    sch.sched_priority = 20;
    pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &sch);
#endif
```

---

### Thread Termination

**Windows**:
```cpp
TerminateThread(thread, 0);
CloseHandle(thread);
```

**Cross-Platform (C++11)**:
```cpp
// Use flag for graceful shutdown
std::atomic<bool> running{true};

void thread_func() {
    while (running) {
        // Work
    }
}

// To stop
running = false;
if (thread.joinable()) {
    thread.join();
}
```

---

## Synchronization

### Critical Section

**Windows**:
```cpp
CRITICAL_SECTION cs;
InitializeCriticalSection(&cs);
EnterCriticalSection(&cs);
// Critical code
LeaveCriticalSection(&cs);
DeleteCriticalSection(&cs);
```

**Cross-Platform (C++11)**:
```cpp
std::recursive_mutex mutex;
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // Critical code
} // Automatic unlock
```

---

### Semaphore

**Windows**:
```cpp
HANDLE semaphore = CreateSemaphore(NULL, 0, MAX_COUNT, NULL);
ReleaseSemaphore(semaphore, 1, NULL);
WaitForSingleObject(semaphore, INFINITE);
CloseHandle(semaphore);
```

**Cross-Platform (C++11)**:
```cpp
std::mutex mutex;
std::condition_variable cv;
int count = 0;

// Signal
{
    std::lock_guard<std::mutex> lock(mutex);
    ++count;
}
cv.notify_one();

// Wait
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&]{ return count > 0; });
    --count;
}
```

---

### Event

**Windows**:
```cpp
HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
SetEvent(event);
WaitForSingleObject(event, INFINITE);
CloseHandle(event);
```

**Cross-Platform (C++11)**:
```cpp
std::mutex mutex;
std::condition_variable cv;
bool signaled = false;

// Set
{
    std::lock_guard<std::mutex> lock(mutex);
    signaled = true;
}
cv.notify_one();

// Wait
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&]{ return signaled; });
    signaled = false; // Auto-reset
}
```

---

## Time Functions

### GetTickCount

**Windows**:
```cpp
DWORD tick = GetTickCount(); // Milliseconds since boot
```

**Cross-Platform (C++11)**:
```cpp
auto now = std::chrono::steady_clock::now();
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()).count();
```

---

### GetLocalTime

**Windows**:
```cpp
SYSTEMTIME st;
GetLocalTime(&st);
```

**Cross-Platform (C++11)**:
```cpp
auto now = std::chrono::system_clock::now();
std::time_t time = std::chrono::system_clock::to_time_t(now);
std::tm* tm = std::localtime(&time);
```

---

### Sleep

**Windows**:
```cpp
Sleep(1000); // Milliseconds
```

**Cross-Platform (C++11)**:
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
```

---

## Timers

### SetTimer

**Windows**:
```cpp
SetTimer(hWnd, TIMER_ID, 1000, NULL);

// In WndProc
case WM_TIMER:
    if (wParam == TIMER_ID) {
        // Timer callback
    }
    break;
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::steady_timer timer(io_context);

void schedule_timer() {
    timer.expires_after(std::chrono::seconds(1));
    timer.async_wait([](const boost::system::error_code& error) {
        if (!error) {
            // Timer callback
            schedule_timer(); // Reschedule
        }
    });
}
```

---

## File I/O

### CreateFile

**Windows**:
```cpp
HANDLE file = CreateFile("file.txt", GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
DWORD written;
WriteFile(file, data, size, &written, NULL);
CloseHandle(file);
```

**Cross-Platform (C++11)**:
```cpp
std::ofstream file("file.txt", std::ios::binary);
file.write(reinterpret_cast<const char*>(data), size);
file.close();
```

---

## Configuration Files

### GetPrivateProfileInt

**Windows**:
```cpp
int value = GetPrivateProfileInt("Section", "Key", 0, "config.ini");
```

**Cross-Platform (Custom)**:
```cpp
class ConfigManager {
    std::map<std::string, std::map<std::string, std::string>> data;
    
public:
    bool load(const std::string& filename) {
        std::ifstream file(filename);
        std::string line, section;
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';') continue;
            
            if (line[0] == '[') {
                section = line.substr(1, line.find(']') - 1);
            } else {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    data[section][key] = value;
                }
            }
        }
        return true;
    }
    
    int get_int(const std::string& section, const std::string& key, int def) {
        try {
            return std::stoi(data[section][key]);
        } catch (...) {
            return def;
        }
    }
};
```

---

## System Information

### GetSystemInfo

**Windows**:
```cpp
SYSTEM_INFO si;
GetSystemInfo(&si);
DWORD cpu_count = si.dwNumberOfProcessors;
```

**Cross-Platform (C++11)**:
```cpp
unsigned int cpu_count = std::thread::hardware_concurrency();
```

---

## GUI to CLI

### Window Creation

**Windows**:
```cpp
HWND hwnd = CreateWindow("ClassName", "Title", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, 0, 600, 600, NULL, NULL, hInst, NULL);
ShowWindow(hwnd, SW_SHOW);
UpdateWindow(hwnd);
```

**Cross-Platform (CLI)**:
```cpp
// Terminal title
std::cout << "\033]0;ConnectServer\007" << std::flush;

// Clear screen
std::cout << "\033[2J\033[H" << std::flush;
```

---

### Message Loop

**Windows**:
```cpp
MSG msg;
while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

**Cross-Platform (Boost.Asio)**:
```cpp
boost::asio::io_context io_context;
io_context.run(); // Blocks until stopped
```

---

### Text Output (GDI)

**Windows**:
```cpp
HDC hdc = GetDC(hwnd);
SetTextColor(hdc, RGB(255, 0, 0));
TextOut(hdc, 10, 10, "Hello", 5);
ReleaseDC(hwnd, hdc);
```

**Cross-Platform (ANSI)**:
```cpp
// Red text
std::cout << "\033[31mHello\033[0m" << std::endl;

// Color codes
// 30=Black, 31=Red, 32=Green, 33=Yellow, 34=Blue, 35=Magenta, 36=Cyan, 37=White
```

---

## Error Handling

### GetLastError

**Windows**:
```cpp
DWORD error = GetLastError();
```

**Cross-Platform**:
```cpp
// For system calls
int error = errno;

// For Boost.Asio
boost::system::error_code ec;
socket.connect(endpoint, ec);
if (ec) {
    std::cerr << ec.message() << std::endl;
}
```

---

### WSAGetLastError

**Windows**:
```cpp
int error = WSAGetLastError();
```

**Cross-Platform (Boost.Asio)**:
```cpp
// Error codes passed to handlers
[](const boost::system::error_code& error, size_t bytes) {
    if (error) {
        std::cerr << error.message() << std::endl;
    }
}
```

---

## Crash Handling

### MiniDump

**Windows**:
```cpp
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* info) {
    HANDLE file = CreateFile("crash.dmp", ...);
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                     file, MiniDumpNormal, info, NULL, NULL);
    CloseHandle(file);
    return EXCEPTION_EXECUTE_HANDLER;
}

SetUnhandledExceptionFilter(ExceptionFilter);
```

**Linux**:
```cpp
#include <signal.h>
#include <execinfo.h>

void signal_handler(int sig) {
    void* array[50];
    size_t size = backtrace(array, 50);
    
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

signal(SIGSEGV, signal_handler);
signal(SIGABRT, signal_handler);
```

---

## String Functions

### strcpy_s

**Windows**:
```cpp
strcpy_s(dest, sizeof(dest), src);
```

**Cross-Platform**:
```cpp
std::strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

// Or better, use std::string
std::string dest = src;
```

---

### sprintf_s

**Windows**:
```cpp
sprintf_s(buffer, sizeof(buffer), "Value: %d", value);
```

**Cross-Platform**:
```cpp
std::snprintf(buffer, sizeof(buffer), "Value: %d", value);

// Or better
std::string str = std::to_string(value);
```

---

## Summary Table

| Windows API | Cross-Platform Alternative | Library |
|-------------|---------------------------|---------|
| WSASocketW | boost::asio::ip::tcp::socket | Boost.Asio |
| WSAAccept | async_accept | Boost.Asio |
| WSARecv | async_read | Boost.Asio |
| WSASend | async_write | Boost.Asio |
| CreateThread | std::thread | C++11 |
| CRITICAL_SECTION | std::recursive_mutex | C++11 |
| CreateSemaphore | std::condition_variable | C++11 |
| GetTickCount | std::chrono::steady_clock | C++11 |
| Sleep | std::this_thread::sleep_for | C++11 |
| SetTimer | boost::asio::steady_timer | Boost.Asio |
| CreateFile | std::fstream | C++11 |
| GetPrivateProfileInt | Custom parser | Custom |
| GetSystemInfo | std::thread::hardware_concurrency | C++11 |
| Win32 GUI | CLI with ANSI codes | Standard |
| MiniDumpWriteDump | signal + backtrace | POSIX |

---

**Last Updated**: October 30, 2025
