#ifdef _WIN32

#include "platform/windows/CrashHandler.h"
#include <windows.h>
#include <dbghelp.h>
#include <cstdio>

static LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter = nullptr;

LONG WINAPI DumpExceptionFilter(EXCEPTION_POINTERS* info) {
    char path[MAX_PATH];
    
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    
    sprintf_s(path, sizeof(path), "%d-%d-%d_%dh%dm%ds.dmp", 
             SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
             SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
    
    HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    
    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = info;
        mdei.ClientPointers = FALSE;
        
        MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
            MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);
        
        if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                             file, dump_type, &mdei, nullptr, nullptr)) {
            CloseHandle(file);
            fprintf(stderr, "Crash dump written to: %s\n", path);
            return EXCEPTION_EXECUTE_HANDLER;
        }
        
        CloseHandle(file);
    }
    
    return EXCEPTION_CONTINUE_SEARCH;
}

void CMiniDump::Start() {
    SetErrorMode(SEM_FAILCRITICALERRORS);
    PreviousExceptionFilter = SetUnhandledExceptionFilter(DumpExceptionFilter);
}

void CMiniDump::Clean() {
    SetUnhandledExceptionFilter(PreviousExceptionFilter);
}

#endif // _WIN32
