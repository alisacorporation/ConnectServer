#include "IpManager.h"
#include <cstring>

CIpManager gIpManager;
int MaxIpConnection = 0;  // Will be set from configuration

CIpManager::CIpManager()
{
}

CIpManager::~CIpManager()
{
}

bool CIpManager::CheckIpAddress(const char* IpAddress)
{
    auto it = this->m_IpAddressInfo.find(std::string(IpAddress));

    if (it == this->m_IpAddressInfo.end())
    {
        return ((MaxIpConnection == 0) ? true : true);  // Allow if not in map
    }
    else
    {
        return ((it->second.IpAddressCount >= MaxIpConnection) ? false : true);
    }
}

void CIpManager::InsertIpAddress(const char* IpAddress)
{
    IP_ADDRESS_INFO info;

    strncpy(info.IpAddress, IpAddress, sizeof(info.IpAddress) - 1);
    info.IpAddress[sizeof(info.IpAddress) - 1] = '\0';

    info.IpAddressCount = 1;

    auto it = this->m_IpAddressInfo.find(std::string(IpAddress));

    if (it == this->m_IpAddressInfo.end())
    {
        this->m_IpAddressInfo.insert(std::pair<std::string, IP_ADDRESS_INFO>(std::string(IpAddress), info));
    }
    else
    {
        it->second.IpAddressCount++;
    }
}

void CIpManager::RemoveIpAddress(const char* IpAddress)
{
    auto it = this->m_IpAddressInfo.find(std::string(IpAddress));

    if (it != this->m_IpAddressInfo.end())
    {
        if ((--it->second.IpAddressCount) == 0)
        {
            this->m_IpAddressInfo.erase(it);
        }
    }
}
