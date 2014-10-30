/*!**************************************************************
*@file buff.h
*@brief    提供对缓存区域的操作接口
*@author   zhaohm3
*@date 2014-10-20 15:02
*@note
*
****************************************************************/

#include "os/os.h"
#include "buff/buff.h"

//! 缓存结构体定义
struct Buffer
{
    Addr_t start;           //!< 起始地址
    Addr_t seek;            //!< 当前能够被写入的起始地址
    Uint32_t size;          //!< 缓存大小
    Uint32_t free;          //!< 空闲空间
    Uint32_t reserved;      //!< 为防止溢出，当free < reserved，认为缓存已满
};

#define BUFMENT_SIZE sizeof(Buffer_t)

GPublic BufferP_t bf_new(Uint32_t size, Uint32_t reserved)
{
    BufferP_t pBuffer = NULL;
    Addr_t pStart = NULL;
    Uint32_t udwSize = size + reserved;

    GMALLOC(pBuffer, Buffer_t, BUFMENT_SIZE);
    GCMON_CHECK_NULL(pBuffer, ERROR);

    GMALLOC(pStart, Char_t, udwSize);
    GCMON_CHECK_NULL(pStart, ERROR);
    pStart[0] = '\0';

    pBuffer->start = pStart;
    pBuffer->seek = pStart;
    pBuffer->size = udwSize;
    pBuffer->free = udwSize;
    pBuffer->reserved = reserved;

    return pBuffer;

ERROR:
    GFREE(pStart);
    GFREE(pBuffer);

    return NULL;
}

GPublic void bf_free(BufferP_t pBuffer)
{
    GCMON_CHECK_NULL(pBuffer, ERROR);

    GFREE(pBuffer->start);
    pBuffer->start = NULL;
    pBuffer->seek = NULL;
    pBuffer->size = 0;
    pBuffer->free = 0;
    pBuffer->reserved = 0;

    GFREE(pBuffer);

ERROR:
    return;
}

GPublic void bf_reset(BufferP_t pBuffer)
{
    GCMON_CHECK_NULL(pBuffer, ERROR);

    pBuffer->seek = pBuffer->start;
    pBuffer->seek[0] = '\0';
    pBuffer->free = pBuffer->size;

ERROR:
    return;
}

GPublic Bool32_t bf_full(BufferP_t pBuffer)
{
    Bool32_t bFull = FALSE;

    GCMON_CHECK_NULL(pBuffer, ERROR);
    bFull = (Bool32_t)(pBuffer->free < pBuffer->reserved);

ERROR:
    return bFull;
}

GPublic Int32_t bf_sprint(BufferP_t pBuffer, String_t fmt, va_list args)
{
    Int32_t size = 0;

    GCMON_CHECK_NULL(pBuffer, ERROR);
    size = os_vsprintf(pBuffer->seek, fmt, args);
    GCMON_CHECK_COND(size >= 0, ERROR);

    pBuffer->seek += size;
    pBuffer->seek[0] = '\0';
    pBuffer->free -= size;

ERROR:
    return size;
}

GPublic Int32_t bf_print(BufferP_t pBuffer, String_t fmt, ... )
{
    Int32_t size = 0;
    va_list args;

    GCMON_CHECK_NULL(pBuffer, ERROR);

    va_start(args, fmt);
    size = bf_sprint(pBuffer, fmt, args);
    va_end(args);

ERROR:
    return size;
}

GPublic void bf_write(BufferP_t pBuffer, FILE *file)
{
    GCMON_CHECK_NULL(pBuffer, ERROR);
    GCMON_CHECK_NULL(file, ERROR);

    pBuffer->seek[0] = '\0';
    os_fprintf(file, "%s\n", pBuffer->start);

ERROR:
    return;
}