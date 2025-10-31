#include "Util.h"
#include "Console.h"
#include "ConsoleInterface.h"
#include <cstdio>
#include <cstdarg>
#include <iostream>

int gClientCount = 0;

void ErrorMessageBox(const char* message, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    
    std::cerr << "ERROR: " << buffer << std::endl;
    
    if (g_console_interface) {
        g_console_interface->log(Color::RED, std::string("ERROR: ") + buffer);
    }
}

void LogAdd(int color, const char* text, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);
    
    // Map color codes to Color enum
    Color console_color = Color::WHITE;
    switch (color) {
        case 0: console_color = Color::WHITE; break;   // LOG_BLACK -> WHITE
        case 1: console_color = Color::RED; break;     // LOG_RED
        case 2: console_color = Color::GREEN; break;   // LOG_GREEN
        case 3: console_color = Color::BLUE; break;    // LOG_BLUE
        default: console_color = Color::WHITE; break;
    }
    
    if (g_console_interface) {
        g_console_interface->log(console_color, buffer);
    } else {
        // Fallback to stdout
        printf("%s\n", buffer);
    }
}

void ConsoleProtocolLog(int type, const uint8_t* lpMsg, int size) {
    if (!gConsole.EnableOutput[type]) {
        return;
    }
    
    char buffer[4096];
    int offset = 0;
    
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[Protocol] ");
    
    for (int i = 0; i < size && i < 32; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", lpMsg[i]);
    }
    
    if (size > 32) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "... (%d bytes)", size);
    }
    
    gConsole.Output(type, "%s", buffer);
}

void ConnectServerTimeoutProc() {
    // This will be implemented when ClientManager is ready
    // For now, it's a placeholder
}

int GetFreeClientIndex() {
    // This will be implemented when ClientManager is ready
    // For now, return -1 (no free index)
    return -1;
}

int SearchFreeClientIndex(int* index, int MinIndex, int MaxIndex, uint32_t MinTime) {
    // This will be implemented when ClientManager is ready
    // For now, return -1 (no free index)
    return -1;
}
