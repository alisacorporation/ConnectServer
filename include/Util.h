#pragma once

#include <cstdint>
#include <chrono>

// Forward declarations
enum class Color;

// Error message box (cross-platform)
void ErrorMessageBox(const char* message, ...);

// Logging with color
void LogAdd(int color, const char* text, ...);

// Console protocol logging
void ConsoleProtocolLog(int type, const uint8_t* lpMsg, int size);

// Timeout processing
void ConnectServerTimeoutProc();

// Get free client index
int GetFreeClientIndex();

// Search for free client index
int SearchFreeClientIndex(int* index, int MinIndex, int MaxIndex, uint32_t MinTime);

// Cross-platform GetTickCount replacement
inline uint32_t GetTickCountCross() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return static_cast<uint32_t>(ms.count());
}

// Compatibility macro for original code
#ifndef _WIN32
#define GetTickCount GetTickCountCross
#endif

// Global client count
extern int gClientCount;
