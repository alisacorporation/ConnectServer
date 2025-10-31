# ConnectServer Protocol Specification

## Overview

This document describes the network protocol used by the ConnectServer for communication with game clients and other server components.

## Packet Structure

### Header Types

**Type C1 - Small Packet (up to 255 bytes)**
```
Offset | Size | Field       | Description
-------|------|-------------|---------------------------
0x00   | 1    | Type        | Always 0xC1
0x01   | 1    | Size        | Total packet size (3-255)
0x02   | 1    | Head        | Packet identifier
0x03   | N    | Data        | Packet payload
```

**Type C2 - Large Packet (up to 65535 bytes)**
```
Offset | Size | Field       | Description
-------|------|-------------|---------------------------
0x00   | 1    | Type        | Always 0xC2
0x01   | 2    | Size        | Total packet size (big-endian)
0x03   | 1    | Head        | Packet identifier
0x04   | N    | Data        | Packet payload
```

**Type C3 - Small Encrypted Packet**
```
Same as C1 but payload is encrypted
```

**Type C4 - Large Encrypted Packet**
```
Same as C2 but payload is encrypted
```

### Sub-Header Format

Some packets use a sub-header for categorization:

```
Offset | Size | Field       | Description
-------|------|-------------|---------------------------
0x00   | 1    | Type        | 0xC1 or 0xC2
0x01   | 1/2  | Size        | Packet size
0x02/3 | 1    | Head        | Main category (e.g., 0xF4)
0x03/4 | 1    | SubHead     | Sub-category
0x04/5 | N    | Data        | Packet payload
```

## Client ↔ ConnectServer Protocol

### 1. Server Init (0x00)

**Direction**: Server → Client  
**When**: Immediately after connection accepted  
**Purpose**: Notify client of successful connection

**Structure**:
```cpp
struct PMSG_SERVER_INIT_SEND {
    PBMSG_HEAD header;  // C1:00
    BYTE result;        // 1 = success
};
```

**Example**:
```
C1 04 00 01
```

---

### 2. Custom Server List Request (0xF4:0x04)

**Direction**: Server → Client (automatic)  
**When**: Before server list request  
**Purpose**: Send custom server names to client

**Structure**:
```cpp
struct PMSG_CUSTOM_SERVER_LIST_SEND {
    PSWMSG_HEAD header;  // C2:F4:04
    BYTE count[2];       // Server count (big-endian)
};

struct PMSG_CUSTOM_SERVER_LIST {
    WORD ServerCode;     // Server identifier
    char ServerName[32]; // Server display name
};
```

**Example**:
```
C2 00 2A F4 04 00 02  // Header + 2 servers
00 00                 // Server 0
"TestServer\0"        // Name (32 bytes)
00 01                 // Server 1
"MainServer\0"        // Name (32 bytes)
```

---

### 3. Server List Request (0xF4:0x02)

**Direction**: Client → Server  
**Purpose**: Request list of available game servers

**Request Structure**:
```cpp
struct PMSG_SERVER_LIST_RECV {
    PSBMSG_HEAD header;  // C1:F4:02
};
```

**Request Example**:
```
C1 04 F4 02
```

**Response Structure**:
```cpp
struct PMSG_SERVER_LIST_SEND {
    PSWMSG_HEAD header;  // C2:F4:02
    BYTE count;          // Number of servers
};

struct PMSG_SERVER_LIST {
    WORD ServerCode;     // Server identifier
    BYTE UserTotal;      // Load percentage (0-100)
};
```

**Response Example**:
```
C2 00 0C F4 02 02     // Header + 2 servers
00 00 32              // Server 0, 50% load
00 01 64              // Server 1, 100% load
```

---

### 4. Server Info Request (0xF4:0x03)

**Direction**: Client → Server  
**Purpose**: Request connection details for specific server

**Request Structure**:
```cpp
struct PMSG_SERVER_INFO_RECV {
    PSBMSG_HEAD header;  // C1:F4:03
    BYTE ServerCode;     // Requested server
};
```

**Request Example**:
```
C1 05 F4 03 00  // Request server 0
```

**Response Structure**:
```cpp
struct PMSG_SERVER_INFO_SEND {
    PSBMSG_HEAD header;      // C1:F4:03
    char ServerAddress[16];  // IP address (null-terminated)
    WORD ServerPort;         // Port number
};
```

**Response Example**:
```
C1 16 F4 03
"127.0.0.1\0"     // IP (16 bytes)
D9 8D             // Port 55901 (little-endian)
```

---

## Inter-Server Protocol (UDP)

### 1. Game Server Live Packet (0x01)

**Direction**: GameServer → ConnectServer  
**When**: Every 5 seconds  
**Purpose**: Heartbeat and status update

**Structure**:
```cpp
struct SDHP_GAME_SERVER_LIVE_RECV {
    PBMSG_HEAD header;    // C1:01
    WORD ServerCode;      // Server identifier
    BYTE UserTotal;       // Load percentage (0-100)
    WORD UserCount;       // Current users
    WORD AccountCount;    // Logged in accounts
    WORD MaxUserCount;    // Maximum capacity
};
```

**Example**:
```
C1 0C 01
00 00        // ServerCode 0
32           // 50% load
01 F4        // 500 users
01 2C        // 300 accounts
03 E8        // 1000 max users
```

**Behavior**:
- If not received for 10 seconds, server marked offline
- Updates server state in ServerList
- Affects server list generation for clients

---

### 2. Join Server Live Packet (0x02)

**Direction**: JoinServer → ConnectServer  
**When**: Every 5 seconds  
**Purpose**: Heartbeat and queue status

**Structure**:
```cpp
struct SDHP_JOIN_SERVER_LIVE_RECV {
    PBMSG_HEAD header;  // C1:02
    DWORD QueueSize;    // Current queue size
};
```

**Example**:
```
C1 07 02
00 00 00 64  // Queue size: 100
```

**Behavior**:
- If not received for 10 seconds, JoinServer marked offline
- If queue size > 100, server list requests rejected
- Affects STANDBY/ACTIVE mode display

---

## Protocol Helpers

### Macros

```cpp
// Byte manipulation
#define SET_NUMBERHB(x) ((BYTE)((DWORD)(x)>>(DWORD)8))
#define SET_NUMBERLB(x) ((BYTE)((DWORD)(x)&0xFF))
#define MAKE_NUMBERW(x,y) ((WORD)(((BYTE)((y)&0xFF))|((BYTE)((x)&0xFF)<<8)))

// Packet size calculation
#define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
```

### Header Structures

```cpp
// Basic header (C1/C3)
struct PBMSG_HEAD {
    void set(BYTE head, BYTE size) {
        this->type = 0xC1;
        this->size = size;
        this->head = head;
    }
    BYTE type;
    BYTE size;
    BYTE head;
};

// Sub-header (C1/C3)
struct PSBMSG_HEAD {
    void set(BYTE head, BYTE subh, BYTE size) {
        this->type = 0xC1;
        this->size = size;
        this->head = head;
        this->subh = subh;
    }
    BYTE type;
    BYTE size;
    BYTE head;
    BYTE subh;
};

// Large header (C2/C4)
struct PWMSG_HEAD {
    void set(BYTE head, WORD size) {
        this->type = 0xC2;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
    }
    BYTE type;
    BYTE size[2];
    BYTE head;
};

// Large sub-header (C2/C4)
struct PSWMSG_HEAD {
    void set(BYTE head, BYTE subh, WORD size) {
        this->type = 0xC2;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
        this->subh = subh;
    }
    BYTE type;
    BYTE size[2];
    BYTE head;
    BYTE subh;
};
```

## Packet Validation

### Size Validation

```cpp
bool validate_packet_size(int size) {
    return size >= 3 && size <= MAX_PACKET_SIZE;
}
```

### Header Validation

```cpp
bool validate_packet_header(BYTE header) {
    return header == 0xC1 || header == 0xC2 || 
           header == 0xC3 || header == 0xC4;
}
```

### Complete Validation

```cpp
bool validate_packet(const BYTE* buffer, size_t buffer_size) {
    if (buffer_size < 3) return false;
    
    BYTE header = buffer[0];
    if (!validate_packet_header(header)) return false;
    
    int packet_size = 0;
    if (header == 0xC1 || header == 0xC3) {
        packet_size = buffer[1];
    } else {
        packet_size = MAKEWORD(buffer[2], buffer[1]);
    }
    
    if (!validate_packet_size(packet_size)) return false;
    if (packet_size > buffer_size) return false;
    
    return true;
}
```

## Error Handling

### Invalid Header

**Condition**: First byte is not 0xC1, 0xC2, 0xC3, or 0xC4  
**Action**: Disconnect client, log error with IP

### Invalid Size

**Condition**: Size < 3 or Size > MAX_PACKET_SIZE  
**Action**: Disconnect client, log error with details

### Incomplete Packet

**Condition**: Buffer contains partial packet  
**Action**: Keep data in buffer, wait for more data

### Unknown Packet

**Condition**: Valid header/size but unknown head/subhead  
**Action**: Ignore packet, log warning

## Timing and Timeouts

### Client Timeout

- **Idle Timeout**: 300 seconds (5 minutes)
- **Check Interval**: 5 seconds (via timer)
- **Action**: Disconnect idle clients

### Server Timeout

- **Heartbeat Interval**: 5 seconds (GameServer/JoinServer)
- **Timeout**: 10 seconds without heartbeat
- **Action**: Mark server offline

## Security Considerations

### IP-Based Limiting

- Configurable max connections per IP
- Checked during accept
- Tracked in IpManager

### Packet Validation

- All packets validated before processing
- Size limits enforced
- Invalid packets cause disconnect

### Buffer Overflow Protection

- Fixed-size buffers
- Size validation before copy
- Bounds checking in parsing

## Testing

### Test Cases

1. **Valid C1 Packet**: `C1 04 00 01`
2. **Valid C2 Packet**: `C2 00 04 00`
3. **Invalid Header**: `FF 04 00 01`
4. **Invalid Size**: `C1 FF 00 01`
5. **Incomplete Packet**: `C1 10 00` (only 3 bytes of 16)
6. **Multiple Packets**: `C1 04 00 01 C1 04 00 01`

### Validation Tools

```cpp
// Hex dump for debugging
void hex_dump(const BYTE* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

// Packet logger
void log_packet(const char* direction, const BYTE* data, size_t size) {
    printf("[%s] ", direction);
    hex_dump(data, size);
}
```

---

**Last Updated**: October 30, 2025
