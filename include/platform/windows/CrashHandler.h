#pragma once

#ifdef _WIN32

class CMiniDump {
public:
    static void Start();
    static void Clean();
};

#endif // _WIN32
