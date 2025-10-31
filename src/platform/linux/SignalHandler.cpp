#ifdef __linux__

#include "platform/linux/SignalHandler.h"
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <fcntl.h>

void SignalHandler::install() {
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGBUS, signal_handler);
}

void SignalHandler::cleanup() {
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
}

void SignalHandler::signal_handler(int sig) {
    // Create crash log filename with timestamp
    char filename[256];
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    strftime(filename, sizeof(filename), "crash_%Y%m%d_%H%M%S.log", tm_info);
    
    // Open crash log file
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        // Write signal information
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "Fatal signal %d received\n", sig);
        write(fd, buffer, len);
        
        const char* sig_name = "UNKNOWN";
        switch (sig) {
            case SIGSEGV: sig_name = "SIGSEGV (Segmentation fault)"; break;
            case SIGABRT: sig_name = "SIGABRT (Abort)"; break;
            case SIGFPE: sig_name = "SIGFPE (Floating point exception)"; break;
            case SIGILL: sig_name = "SIGILL (Illegal instruction)"; break;
            case SIGBUS: sig_name = "SIGBUS (Bus error)"; break;
        }
        
        len = snprintf(buffer, sizeof(buffer), "Signal: %s\n\n", sig_name);
        write(fd, buffer, len);
        
        // Write backtrace
        const char* header = "Backtrace:\n";
        write(fd, header, strlen(header));
        
        void* array[50];
        size_t size = backtrace(array, 50);
        backtrace_symbols_fd(array, size, fd);
        
        close(fd);
        
        fprintf(stderr, "Crash log written to: %s\n", filename);
    }
    
    // Also print to stderr
    fprintf(stderr, "\n=== FATAL ERROR ===\n");
    fprintf(stderr, "Signal %d received\n", sig);
    fprintf(stderr, "Backtrace:\n");
    
    void* array[50];
    size_t size = backtrace(array, 50);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    
    // Exit
    _exit(1);
}

void SignalHandler::write_backtrace() {
    void* array[50];
    size_t size = backtrace(array, 50);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

#endif // __linux__
