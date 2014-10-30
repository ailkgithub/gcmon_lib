/*!**************************************************************
 *@file thread.h
 *@brief    线程模块实现
 *@author   zhaohm3
 *@date 2014-10-15 15:45
 *@note
 * 
 ****************************************************************/

#ifndef _thread_h__
#define _thread_h__

#include "share/share.h"

typedef struct OSThread OSThread_t, *OSThreadP_t;

GPublic OSThreadP_t os_thread_create(Int32_t (JNICALL *pfnStart)(void *), void *pArgs);
GPublic OSThreadP_t os_thread_start(OSThreadP_t pThread);
GPublic Int32_t os_thread_exit(OSThreadP_t pThread);

#endif