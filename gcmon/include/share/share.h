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
#include <stdarg.h>

#include "hotspot/jvmti.h"
#include "os/type.h"

//! 标记私有接口，不对外提供
#define GPrivate static

//! 标记公有接口，对外提供
#define GPublic extern

//! 打开gFile
GPublic void gcmon_debug_fopen();

//! 关闭gFile
GPublic void gcmon_debug_fclose();

//! flush文件缓存
GPublic void gcmon_debug_flush();

//! 打印调试信息
GPublic int gcmon_debug_msg(const char *fmt, ...);

//! 错误检测
#define GCMON_CHECK_ERROR(error, msg, where)                \
do                                                          \
{                                                           \
    jvmtiError e = (error);                                 \
    if (e != JVMTI_ERROR_NONE)                              \
    {                                                       \
        gcmon_debug_msg("JVMTI Error No. = %d : %s in %s --> %s : %d \n", e, msg, __FUNCTION__, __FILE__, __LINE__); \
        printf("JVMTI Error No. = %d : %s \n", e, msg); \
        goto where;                                         \
    }                                                       \
} while (0)

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
#define GCMON_CHECK_COND(condition, where)                  \
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

#ifdef DEBUG
    #define GCMON_PRINT_FUNC() printf("%s\n", __FUNCTION__)
#else
    #define GCMON_PRINT_FUNC()
#endif

//! 断言重定义
#define GASSERT assert

#define KB (1024UL)
#define MB (1024UL * KB)
#define GB (1024UL * MB)
#define TB (1024UL * GB)

#endif
