/*!**************************************************************
 *@file thread.c
 *@brief    线程模块实现
 *@author   zhaohm3
 *@date 2014-10-15 15:46
 *@note
 * 
 ****************************************************************/

#include "os/thread.h"
#include "os/os.h"

#ifdef WIN32
#include <windows.h>
#include <process.h>
typedef HANDLE Thread_t, *ThreadP_t;
#elif defined(LINUX) || defined(SOLARIS)
#include <pthread.h>
typedef pthread_t Thread_t, *ThreadP_t;
#else
#error UnsupportedSystem
#endif

struct OSThread
{
    Thread_t hThread;
    Int32_t (JNICALL *pfnStart)(void *);
    void *pArgs;
};

#define OSTHREAD_SIZE sizeof(OSThread_t)

GPrivate void os_thread_clear(OSThreadP_t pThread)
{
    os_memset(pThread, 0, OSTHREAD_SIZE);
}

GPrivate OSThreadP_t os_thread_new()
{
    OSThreadP_t pThread = NULL;
    GMALLOC(pThread, OSThread_t, OSTHREAD_SIZE);
    GCMON_CHECK_NULL(pThread, OSERROR);
    os_thread_clear(pThread);

OSERROR:
    return pThread;
}

GPrivate void os_thread_free(OSThreadP_t pThread)
{
    GCMON_CHECK_NULL(pThread, OSERROR);
    /*os_thread_clear(pThread);*/
    GFREE(pThread);

OSERROR:
    return;
}

#ifdef WIN32

GPublic OSThreadP_t os_thread_create(Int32_t (JNICALL *pfnStart)(void *), void *pArgs)
{
    Int32_t sdwId = -1;
    OSThreadP_t pThread = NULL;

    pThread = os_thread_new();
    GCMON_CHECK_NULL(pThread, OSERROR);

    pThread->hThread = (Thread_t)_beginthreadex(NULL,
        0,
        pfnStart,
        pArgs,
        CREATE_SUSPENDED,
        &sdwId);

    GCMON_CHECK_NULL(pThread->hThread, OSERROR);
    pThread->pfnStart = pfnStart; 
    pThread->pArgs = pArgs;

    return pThread;

OSERROR:
    os_thread_free(pThread);

    return NULL;
}

GPublic OSThreadP_t os_thread_start(OSThreadP_t pThread)
{
    GCMON_CHECK_NULL(pThread, OSERROR);
    ResumeThread(pThread->hThread);

OSERROR:
    return pThread;
}

GPublic Int32_t os_thread_exit(OSThreadP_t pThread)
{
    Int32_t sdwResult = -1;

    GCMON_CHECK_NULL(pThread, OSERROR);
    GetExitCodeThread(pThread->hThread, &sdwResult);
    TerminateThread(pThread->hThread, sdwResult);
    os_sleep(500);
    os_thread_free(pThread);

OSERROR:
    return sdwResult;
}

#elif defined(LINUX) || defined(SOLARIS)

GPublic OSThreadP_t os_thread_create(Int32_t (JNICALL *pfnStart)(void *), void *pArgs)
{
    Int32_t sdwRet = 0;
    OSThreadP_t pThread = NULL;

    pThread = os_thread_new();
    GCMON_CHECK_NULL(pThread, OSERROR);

    sdwRet = pthread_create(&pThread->hThread, NULL, (void *(*)(void*))pfnStart, pArgs);
    GCMON_CHECK_COND(0 == sdwRet, OSERROR);

    pThread->pfnStart = pfnStart;
    pThread->pArgs = pArgs;

    return pThread;

OSERROR:
    os_thread_free(pThread);

    return NULL;
}

GPublic OSThreadP_t os_thread_start(OSThreadP_t pThread)
{
    GCMON_CHECK_NULL(pThread, OSERROR);
OSERROR:
    return pThread;
}

GPublic Int32_t os_thread_exit(OSThreadP_t pThread)
{
    Int32_t sdwResult = -1;

    GCMON_CHECK_NULL(pThread, OSERROR);
    sdwResult = pthread_cancel(pThread->hThread);
    os_sleep(500);
    os_thread_free(pThread);

OSERROR:
    return sdwResult;
}

#else
#error UnsupportedSystem
#endif
