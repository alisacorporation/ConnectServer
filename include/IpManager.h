#pragma once

#include <map>
#include <string>
#include <cstdint>

struct IP_ADDRESS_INFO
{
    char IpAddress[16];
    uint16_t IpAddressCount;
};

class CIpManager
{
public:
    CIpManager();
    ~CIpManager();

    bool CheckIpAddress(const char* IpAddress);
    void InsertIpAddress(const char* IpAddress);
    void RemoveIpAddress(const char* IpAddress);

private:
    std::map<std::string, IP_ADDRESS_INFO> m_IpAddressInfo;
};

extern CIpManager gIpManager;
extern int MaxIpConnection;  // From configuration
