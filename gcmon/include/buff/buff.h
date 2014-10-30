/*!**************************************************************
 *@file buff.h
 *@brief    提供对缓存区域的操作接口
 *@author   zhaohm3
 *@date 2014-10-20 15:02
 *@note
 * 
 ****************************************************************/

#ifndef _buff_h__
#define _buff_h__

#include "share/share.h"

typedef struct Buffer Buffer_t, *BufferP_t;

GPublic BufferP_t bf_new(Uint32_t size, Uint32_t reserved);
GPublic void bf_free(BufferP_t pBuffer);
GPublic void bf_reset(BufferP_t pBuffer);
GPublic Bool32_t bf_full(BufferP_t pBuffer);
GPublic Int32_t bf_sprint(BufferP_t pBuffer, String_t fmt, va_list args);
GPublic Int32_t bf_print(BufferP_t pBuffer, String_t fmt, ...);
GPublic void bf_write(BufferP_t pBuffer, FILE *file);

#endif
