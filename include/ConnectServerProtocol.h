#pragma once

#include "ProtocolDefines.h"
#include <cstdint>

//**********************************************//
//********** Client -> ConnectServer ***********//
//**********************************************//

struct PMSG_SERVER_LIST_RECV
{
    PSBMSG_HEAD header; // C1:F4:02
};

struct PMSG_SERVER_INFO_RECV
{
    PSBMSG_HEAD header; // C1:F4:03
    uint8_t ServerCode;
};

//**********************************************//
//********** ConnectServer -> Client ***********//
//**********************************************//

struct PMSG_SERVER_INIT_SEND
{
    PBMSG_HEAD header; // C1:00
    uint8_t result;
};

struct PMSG_CUSTOM_SERVER_LIST_SEND
{
    PSWMSG_HEAD header; // C2:F4:01
    uint8_t count[2];
};

struct PMSG_CUSTOM_SERVER_LIST
{
    uint16_t ServerCode;
    char ServerName[32];
};

struct PMSG_SERVER_LIST_SEND
{
    PSWMSG_HEAD header; // C2:F4:02
    uint8_t count;
};

struct PMSG_SERVER_LIST
{
    uint16_t ServerCode;
    uint8_t UserTotal;
};

struct PMSG_SERVER_INFO_SEND
{
    PSBMSG_HEAD header; // C1:F4:03
    char ServerAddress[16];
    uint16_t ServerPort;
};

//**********************************************//
//************** Protocol Core *****************//
//**********************************************//

void ConnectServerProtocolCore(int index, uint8_t head, const uint8_t* lpMsg, int size);

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index);
void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index);
void CCCustomServerListSend(int index);
void CCServerInitSend(int index, int result);
