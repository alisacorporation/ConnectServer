#pragma once

#ifdef __linux__

class SignalHandler {
public:
    static void install();
    static void cleanup();

private:
    static void signal_handler(int sig);
    static void write_backtrace();
};

#endif // __linux__
