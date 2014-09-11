/*!**************************************************************
 *@file share.h
 *@brief    共享数据定义
 *@author   zhaohm3
 *@date 2014-9-3 18:22
 *@note
 * 
 ****************************************************************/

#ifndef _share_h__
#define _share_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hotspot/jni.h"
#include "hotspot/jvmti.h"

#include "share/type.h"

//! 标记私有接口，不对外提供
#define GPrivate static

//! 标记公有接口，对外提供
#define GPublic extern

//! 打开gFile
GPublic void gcmon_debug_fopen();

//! 关闭gFile
GPublic void gcmon_debug_fclose();

//! 打印调试信息
GPublic int gcmon_debug_msg(const char *fmt, ...);

//! 检测句柄是否为空
#define GCMON_CHECK_NULL(handle, where)                     \
do                                                          \
{                                                           \
    if (NULL == (handle))                                   \
    {                                                       \
        gcmon_debug_msg("%s is null in %s --> %s : %d \n", #handle, __FUNCTION__, __FILE__, __LINE__); \
        goto where;                                         \
    }                                                       \
} while (0)

//! 检测条件是否成立
#define GCMON_CHECK_CONDITION(condition, where)             \
do                                                          \
{                                                           \
    if (!(condition))                                       \
    {                                                       \
        gcmon_debug_msg("%s check failure in %s --> %s : %d \n", #condition, __FUNCTION__, __FILE__, __LINE__); \
        goto where;                                         \
    }                                                       \
} while (0)

//! 分配句柄空间
#define GMALLOC(handle, type, size)                         \
do                                                          \
{                                                           \
    handle = (type *)malloc(size);                          \
    gcmon_debug_msg("%s --> 0x%p = (%s *)malloc(%d) in %s --> %s : %d \n", #handle, handle, #type, size, __FUNCTION__, __FILE__, __LINE__); \
} while (0)

//! 释放句柄空间
#define GFREE(handle)                                       \
do                                                          \
{                                                           \
    if ((handle) != NULL)                                   \
    {                                                       \
        gcmon_debug_msg("free(%s --> 0x%p) in %s --> %s : %d \n", #handle, handle, __FUNCTION__, __FILE__, __LINE__); \
        free(handle);                                       \
        handle = NULL;                                      \
    }                                                       \
} while (0)

//! 断言重定义
#define GASSERT assert

#endif
