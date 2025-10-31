#pragma once

#include "ProtocolDefines.h"
#include <map>
#include <cstdint>

#define MAX_JOIN_SERVER_QUEUE_SIZE 100

//**********************************************//
//********** UDP Protocol Structures ***********//
//**********************************************//

struct SDHP_GAME_SERVER_LIVE_RECV
{
    PBMSG_HEAD header; // C1:01
    uint16_t ServerCode;
    uint8_t UserTotal;
    uint16_t UserCount;
    uint16_t AccountCount;
    uint16_t MaxUserCount;
};

struct SDHP_JOIN_SERVER_LIVE_RECV
{
    PBMSG_HEAD header; // C1:02
    uint32_t QueueSize;
};

//**********************************************//
//************ Server List Info ****************//
//**********************************************//

struct SERVER_LIST_INFO
{
    uint16_t ServerCode;
    char ServerName[32];
    char ServerAddress[16];
    uint16_t ServerPort;
    bool ServerShow;
    bool ServerState;
    uint32_t ServerStateTime;
    uint8_t UserTotal;
    uint16_t UserCount;
    uint16_t AccountCount;
    uint16_t MaxUserCount;
};

class CServerList
{
public:
    CServerList();
    ~CServerList();

    void Load(const char* path);
    void MainProc();
    bool CheckJoinServerState();
    
    long GenerateCustomServerList(uint8_t* lpMsg, int* size);
    long GenerateServerList(uint8_t* lpMsg, int* size);
    
    SERVER_LIST_INFO* GetServerListInfo(int ServerCode);
    
    void ServerProtocolCore(uint8_t head, uint8_t* lpMsg, int size);
    void GCGameServerLiveRecv(SDHP_GAME_SERVER_LIVE_RECV* lpMsg);
    void JCJoinServerLiveRecv(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg);

private:
    bool m_JoinServerState;
    uint32_t m_JoinServerStateTime;
    uint32_t m_JoinServerQueueSize;
    std::map<int, SERVER_LIST_INFO> m_ServerListInfo;
};

extern CServerList gServerList;
