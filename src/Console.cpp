#include "Console.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

CConsole gConsole;

CConsole::CConsole() {
    HideConsole = false;
    
    // Enable general output by default
    EnableOutput[CON_GENERAL] = true;
    EnableOutput[CON_PROTO_TCP_RECV] = false;
    EnableOutput[CON_PROTO_TCP_SEND] = false;
    
    for (int i = 3; i < 5; i++) {
        EnableOutput[i] = false;
    }
}

CConsole::~CConsole() {
}

void CConsole::Output(int type, const char* text, ...) {
    if (type < 0 || type >= MAX_CONSOLE_OUTPUT) {
        return;
    }
    
    if (!EnableOutput[type]) {
        return;
    }
    
    if (HideConsole) {
        return;
    }
    
    char buffer[1024];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);
    
    printf("%s\n", buffer);
}
