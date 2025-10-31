#include "ServerList.h"
#include "ConnectServerProtocol.h"
#include "ReadScript.h"
#include "Util.h"
#include <cstring>

CServerList gServerList;

CServerList::CServerList()
{
    this->m_JoinServerState = false;
    this->m_JoinServerStateTime = 0;
    this->m_JoinServerQueueSize = 0;
    this->m_ServerListInfo.clear();
}

CServerList::~CServerList()
{
}

void CServerList::Load(const char* path)
{
    CReadScript* lpReadScript = new CReadScript;

    if (lpReadScript == nullptr)
    {
        ErrorMessageBox(READ_SCRIPT_ALLOC_ERROR, path);
        return;
    }

    if (!lpReadScript->Load(path))
    {
        ErrorMessageBox(READ_SCRIPT_FILE_ERROR, path);
        delete lpReadScript;
        return;
    }

    this->m_ServerListInfo.clear();

    try
    {
        eTokenResult token;

        while (true)
        {
            token = lpReadScript->GetToken();

            if (token == TOKEN_END || token == TOKEN_END_SECTION)
            {
                break;
            }

            SERVER_LIST_INFO info;

            info.ServerCode = lpReadScript->GetNumber();

            strncpy(info.ServerName, lpReadScript->GetAsString(), sizeof(info.ServerName) - 1);
            info.ServerName[sizeof(info.ServerName) - 1] = '\0';

            strncpy(info.ServerAddress, lpReadScript->GetAsString(), sizeof(info.ServerAddress) - 1);
            info.ServerAddress[sizeof(info.ServerAddress) - 1] = '\0';

            info.ServerPort = lpReadScript->GetAsNumber();

            info.ServerShow = (strcmp(lpReadScript->GetAsString(), "SHOW") == 0);

            info.ServerState = false;
            info.ServerStateTime = 0;
            info.UserTotal = 0;
            info.UserCount = 0;
            info.AccountCount = 0;
            info.MaxUserCount = 0;

            this->m_ServerListInfo.insert(std::pair<int, SERVER_LIST_INFO>(info.ServerCode, info));
        }
    }
    catch (...)
    {
        ErrorMessageBox(lpReadScript->GetError());
    }

    delete lpReadScript;

    LogAdd(3, "[ServerList] ServerList loaded successfully (%d servers)", 
           static_cast<int>(this->m_ServerListInfo.size()));
}

void CServerList::MainProc()
{
    // Check JoinServer timeout (10 seconds)
    if (this->m_JoinServerState != false && (GetTickCountCross() - this->m_JoinServerStateTime) > 10000)
    {
        this->m_JoinServerState = false;
        this->m_JoinServerStateTime = 0;
        LogAdd(1, "[ServerList] JoinServer offline");
    }

    // Check GameServer timeouts (10 seconds)
    for (auto it = this->m_ServerListInfo.begin(); it != this->m_ServerListInfo.end(); it++)
    {
        if (it->second.ServerState != false && (GetTickCountCross() - it->second.ServerStateTime) > 10000)
        {
            it->second.ServerState = false;
            it->second.ServerStateTime = 0;
            LogAdd(0, "[ServerList] GameServer offline (%s) (%d)", 
                   it->second.ServerName, it->second.ServerCode);
        }
    }
}

bool CServerList::CheckJoinServerState()
{
    // TODO: For testing without JoinServer, always return true
    // Re-enable this check when JoinServer is running
    return true;
    
    /*
    if (this->m_JoinServerState == false)
    {
        return false;
    }

    if (this->m_JoinServerQueueSize > MAX_JOIN_SERVER_QUEUE_SIZE)
    {
        return false;
    }

    return true;
    */
}

long CServerList::GenerateCustomServerList(uint8_t* lpMsg, int* size)
{
    int count = 0;

    PMSG_CUSTOM_SERVER_LIST info;

    if (this->CheckJoinServerState() != 0)
    {
        for (auto it = this->m_ServerListInfo.begin(); it != this->m_ServerListInfo.end(); it++)
        {
            // Temporarily show all servers marked as SHOW, even if offline (for testing)
            // TODO: Re-enable ServerState check when GameServer is running
            if (it->second.ServerShow != 0) // && it->second.ServerState != 0)
            {
                info.ServerCode = it->second.ServerCode;

                strncpy(info.ServerName, it->second.ServerName, sizeof(info.ServerName) - 1);
                info.ServerName[sizeof(info.ServerName) - 1] = '\0';

                memcpy(&lpMsg[(*size)], &info, sizeof(info));

                (*size) += sizeof(info);

                count++;
            }
        }
    }

    return count;
}

long CServerList::GenerateServerList(uint8_t* lpMsg, int* size)
{
    int count = 0;

    PMSG_SERVER_LIST info;

    if (this->CheckJoinServerState() != false)
    {
        for (auto it = this->m_ServerListInfo.begin(); it != this->m_ServerListInfo.end(); it++)
        {
            // Temporarily show all servers marked as SHOW, even if offline (for testing)
            // TODO: Re-enable ServerState check when GameServer is running
            if (it->second.ServerShow != false) // && it->second.ServerState != false)
            {
                info.ServerCode = it->second.ServerCode;
                info.UserTotal = it->second.UserTotal;

                memcpy(&lpMsg[(*size)], &info, sizeof(info));

                (*size) += sizeof(info);

                count++;
            }
        }
    }

    return count;
}

SERVER_LIST_INFO* CServerList::GetServerListInfo(int ServerCode)
{
    auto it = this->m_ServerListInfo.find(ServerCode);

    if (it == this->m_ServerListInfo.end())
    {
        return nullptr;
    }
    else
    {
        return &it->second;
    }
}

void CServerList::ServerProtocolCore(uint8_t head, uint8_t* lpMsg, int size)
{
    switch (head)
    {
        case 0x01:
        {
            this->GCGameServerLiveRecv((SDHP_GAME_SERVER_LIVE_RECV*)lpMsg);
            break;
        }

        case 0x02:
        {
            this->JCJoinServerLiveRecv((SDHP_JOIN_SERVER_LIVE_RECV*)lpMsg);
            break;
        }
    }
}

void CServerList::GCGameServerLiveRecv(SDHP_GAME_SERVER_LIVE_RECV* lpMsg)
{
    SERVER_LIST_INFO* lpServerListInfo = this->GetServerListInfo(lpMsg->ServerCode);

    if (lpServerListInfo == nullptr)
    {
        return;
    }

    if (lpServerListInfo->ServerState == false)
    {
        LogAdd(0, "[ServerList] GameServer online (%s) (%d)", 
               lpServerListInfo->ServerName, lpServerListInfo->ServerCode);
    }

    lpServerListInfo->ServerState = true;
    lpServerListInfo->ServerStateTime = GetTickCountCross();
    lpServerListInfo->UserTotal = lpMsg->UserTotal;
    lpServerListInfo->UserCount = lpMsg->UserCount;
    lpServerListInfo->AccountCount = lpMsg->AccountCount;
    lpServerListInfo->MaxUserCount = lpMsg->MaxUserCount;
}

void CServerList::JCJoinServerLiveRecv(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg)
{
    if (this->m_JoinServerState == false)
    {
        LogAdd(2, "[ServerList] JoinServer online");
    }

    this->m_JoinServerState = true;
    this->m_JoinServerStateTime = GetTickCountCross();
    this->m_JoinServerQueueSize = lpMsg->QueueSize;
}
