#pragma once

#include <cstdint>

// Byte manipulation macros
#define SET_NUMBERHB(x) ((uint8_t)((uint32_t)(x)>>(uint32_t)8))
#define SET_NUMBERLB(x) ((uint8_t)((uint32_t)(x)&0xFF))
#define SET_NUMBERHW(x) ((uint16_t)((uint32_t)(x)>>(uint32_t)16))
#define SET_NUMBERLW(x) ((uint16_t)((uint32_t)(x)&0xFFFF))
#define SET_NUMBERHDW(x) ((uint32_t)((uint64_t)(x)>>(uint64_t)32))
#define SET_NUMBERLDW(x) ((uint32_t)((uint64_t)(x)&0xFFFFFFFF))

#define MAKE_NUMBERW(x,y) ((uint16_t)(((uint8_t)((y)&0xFF))|((uint16_t)((uint8_t)((x)&0xFF))<<8)))
#define MAKE_NUMBERDW(x,y) ((uint32_t)(((uint16_t)((y)&0xFFFF))|((uint32_t)((uint16_t)((x)&0xFFFF))<<16)))
#define MAKE_NUMBERQW(x,y) ((uint64_t)(((uint32_t)((y)&0xFFFFFFFF))|((uint64_t)((uint32_t)((x)&0xFFFFFFFF))<<32)))

// Compatibility with original code
#define MAKEWORD(low, high) MAKE_NUMBERW(high, low)

//**********************************************//
//************ Packet Base *********************//
//**********************************************//

struct PBMSG_HEAD
{
    void set(uint8_t head, uint8_t size)
    {
        this->type = 0xC1;
        this->size = size;
        this->head = head;
    }

    void setE(uint8_t head, uint8_t size)
    {
        this->type = 0xC3;
        this->size = size;
        this->head = head;
    }

    uint8_t type;
    uint8_t size;
    uint8_t head;
};

struct PSBMSG_HEAD
{
    void set(uint8_t head, uint8_t subh, uint8_t size)
    {
        this->type = 0xC1;
        this->size = size;
        this->head = head;
        this->subh = subh;
    }

    void setE(uint8_t head, uint8_t subh, uint8_t size)
    {
        this->type = 0xC3;
        this->size = size;
        this->head = head;
        this->subh = subh;
    }

    uint8_t type;
    uint8_t size;
    uint8_t head;
    uint8_t subh;
};

struct PWMSG_HEAD
{
    void set(uint8_t head, uint16_t size)
    {
        this->type = 0xC2;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
    }

    void setE(uint8_t head, uint16_t size)
    {
        this->type = 0xC4;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
    }

    uint8_t type;
    uint8_t size[2];
    uint8_t head;
};

struct PSWMSG_HEAD
{
    void set(uint8_t head, uint8_t subh, uint16_t size)
    {
        this->type = 0xC2;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
        this->subh = subh;
    }

    void setE(uint8_t head, uint8_t subh, uint16_t size)
    {
        this->type = 0xC4;
        this->size[0] = SET_NUMBERHB(size);
        this->size[1] = SET_NUMBERLB(size);
        this->head = head;
        this->subh = subh;
    }

    uint8_t type;
    uint8_t size[2];
    uint8_t head;
    uint8_t subh;
};
