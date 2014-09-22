/*!**************************************************************
 *@file share.h
 *@brief    �������ݶ���
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

//! ���˽�нӿڣ��������ṩ
#define GPrivate static

//! ��ǹ��нӿڣ������ṩ
#define GPublic extern

//! ��gFile
GPublic void gcmon_debug_fopen();

//! �ر�gFile
GPublic void gcmon_debug_fclose();

//! flush�ļ�����
GPublic void gcmon_debug_flush();

//! ��ӡ������Ϣ
GPublic int gcmon_debug_msg(const char *fmt, ...);

//! ������
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

//! ������Ƿ�Ϊ��
#define GCMON_CHECK_NULL(handle, where)                     \
do                                                          \
{                                                           \
    if (NULL == (handle))                                   \
    {                                                       \
        gcmon_debug_msg("%s is null in %s --> %s : %d \n", #handle, __FUNCTION__, __FILE__, __LINE__); \
        goto where;                                         \
    }                                                       \
} while (0)

//! ��������Ƿ����
#define GCMON_CHECK_COND(condition, where)                  \
do                                                          \
{                                                           \
    if (!(condition))                                       \
    {                                                       \
        gcmon_debug_msg("%s check failure in %s --> %s : %d \n", #condition, __FUNCTION__, __FILE__, __LINE__); \
        goto where;                                         \
    }                                                       \
} while (0)

//! �������ռ�
#define GMALLOC(handle, type, size)                         \
do                                                          \
{                                                           \
    handle = (type *)malloc(size);                          \
    gcmon_debug_msg("%s --> 0x%p = (%s *)malloc(%d) in %s --> %s : %d \n", #handle, handle, #type, size, __FUNCTION__, __FILE__, __LINE__); \
} while (0)

//! �ͷž���ռ�
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

//! �����ض���
#define GASSERT assert

#define KB (1024UL)
#define MB (1024UL * KB)
#define GB (1024UL * MB)
#define TB (1024UL * GB)

#endif
