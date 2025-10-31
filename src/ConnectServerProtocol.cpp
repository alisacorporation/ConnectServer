#include "ConnectServerProtocol.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "Console.h"
#include "Util.h"
#include <cstring>

void ConnectServerProtocolCore(int index, uint8_t head, const uint8_t* lpMsg, int size)
{
    ConsoleProtocolLog(CON_PROTO_TCP_RECV, lpMsg, size);
    
    LogAdd(2, "[Protocol] Received packet: Index=%d, Head=0x%02X, Size=%d", index, head, size);

    // Update packet time for timeout tracking
    // TODO: Will be implemented when we add timeout tracking to ClientSession

    switch (head)
    {
        case 0xF4:
        {
            uint8_t subhead = lpMsg[3];
            LogAdd(2, "[Protocol] 0xF4 packet, subhead=0x%02X", subhead);
            
            switch (subhead)
            {
                case 0x02:
                {
                    LogAdd(2, "[Protocol] Server list request from client %d", index);
                    CCCustomServerListSend(index);
                    CCServerListRecv((PMSG_SERVER_LIST_RECV*)lpMsg, index);
                    break;
                }

                case 0x03:
                {
                    LogAdd(2, "[Protocol] Server info request from client %d", index);
                    CCServerInfoRecv((PMSG_SERVER_INFO_RECV*)lpMsg, index);
                    break;
                }
                
                default:
                {
                    LogAdd(1, "[Protocol] Unknown 0xF4 subhead: 0x%02X", subhead);
                    break;
                }
            }

            break;
        }
        
        default:
        {
            LogAdd(1, "[Protocol] Unknown packet head: 0x%02X", head);
            break;
        }
    }
}

void CCServerInitSend(int index, int result)
{
    PMSG_SERVER_INIT_SEND pMsg;

    pMsg.header.set(0x00, sizeof(pMsg));
    pMsg.result = result;

    LogAdd(2, "[Protocol] Sending init packet to client %d, result=%d", index, result);

    auto session = g_socket_manager->get_session(index);
    if (session) {
        session->async_send((uint8_t*)&pMsg, pMsg.header.size);
    } else {
        LogAdd(1, "[Protocol] Failed to get session %d for init send", index);
    }
}

void CCCustomServerListSend(int index)
{
    uint8_t send[2048];

    PMSG_CUSTOM_SERVER_LIST_SEND pMsg;

    pMsg.header.set(0xF4, 0x04, 0);

    int size = sizeof(pMsg);

    int count = gServerList.GenerateCustomServerList(send, &size);

    pMsg.count[0] = SET_NUMBERHB(count);
    pMsg.count[1] = SET_NUMBERLB(count);

    pMsg.header.size[0] = SET_NUMBERHB(size);
    pMsg.header.size[1] = SET_NUMBERLB(size);

    memcpy(send, &pMsg, sizeof(pMsg));

    auto session = g_socket_manager->get_session(index);
    if (session) {
        session->async_send(send, size);
    }
}

void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index)
{
    uint8_t send[2048];

    PMSG_SERVER_LIST_SEND pMsg;

    pMsg.header.set(0xF4, 0x02, 0);

    int size = sizeof(pMsg);

    int count = gServerList.GenerateServerList(send, &size);

    LogAdd(2, "[Protocol] Sending server list to client %d: count=%d, size=%d", index, count, size);

    pMsg.count = count;

    pMsg.header.size[0] = SET_NUMBERHB(size);
    pMsg.header.size[1] = SET_NUMBERLB(size);

    memcpy(send, &pMsg, sizeof(pMsg));

    auto session = g_socket_manager->get_session(index);
    if (session) {
        session->async_send(send, size);
    } else {
        LogAdd(1, "[Protocol] Failed to get session %d for server list send", index);
    }
}

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index)
{
    LogAdd(2, "[Protocol] Server info request for ServerCode=%d from client %d", lpMsg->ServerCode, index);

    SERVER_LIST_INFO* lpServerListInfo = gServerList.GetServerListInfo(lpMsg->ServerCode);

    if (lpServerListInfo == nullptr)
    {
        LogAdd(1, "[Protocol] Server code %d not found", lpMsg->ServerCode);
        return;
    }

    // Temporarily allow offline servers for testing
    // TODO: Re-enable these checks when GameServer is running
    // if (lpServerListInfo->ServerShow == 0 || lpServerListInfo->ServerState == 0)
    if (lpServerListInfo->ServerShow == 0)
    {
        LogAdd(1, "[Protocol] Server %d is hidden", lpMsg->ServerCode);
        return;
    }

    PMSG_SERVER_INFO_SEND pMsg;

    pMsg.header.set(0xF4, 0x03, sizeof(pMsg));

    memcpy(pMsg.ServerAddress, lpServerListInfo->ServerAddress, sizeof(pMsg.ServerAddress));

    pMsg.ServerPort = lpServerListInfo->ServerPort;

    LogAdd(2, "[Protocol] Sending server info to client %d: %s:%d", 
           index, pMsg.ServerAddress, pMsg.ServerPort);

    auto session = g_socket_manager->get_session(index);
    if (session) {
        session->async_send((uint8_t*)&pMsg, pMsg.header.size);
    } else {
        LogAdd(1, "[Protocol] Failed to get session %d for server info send", index);
    }
}
