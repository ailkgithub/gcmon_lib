/*!**************************************************************
 *@file mem.c
 *@brief    提供与操作系统无关的内存信息获取接口
 *@author   zhaohm3
 *@date 2014-9-20 20:13
 *@note
 * 
 ****************************************************************/

#include "os/mem.h"

#ifdef WIN32

#include <windows.h>
#include <psapi.h>

/*!
*@brief        获取物理主机的内存信息
*@author       zhaohm3
*@param[in]    pMemoryInfo
*@retval
*@note
* 
*@since    2014-9-20 23:17
*@attention
* 
*/
GPublic Bool32_t os_get_physical_memory_info(PhysicalMemoryInfoP_t pMemoryInfo)
{
    Bool32_t bSuccess = FALSE;
    MEMORYSTATUSEX mstatus = { 0 };

    mstatus.dwLength = sizeof(mstatus);
    bSuccess = (Bool32_t)(GlobalMemoryStatusEx(&mstatus) != 0);

    if (bSuccess && pMemoryInfo != NULL)
    {
        pMemoryInfo->lwTotalPhys = mstatus.ullTotalPhys;
        pMemoryInfo->lwAvailPhys = mstatus.ullAvailPhys;
    }

    return bSuccess;
}

/*!
*@brief        获取当前进程的内存使用信息
*@author       zhaohm3
*@param[in]    pMemoryInfo
*@retval
*@note
* 
*@since    2014-9-20 23:18
*@attention
* 
*/
GPublic Bool32_t os_get_process_memory_info(ProccessMemoryInfoP_t pMemoryInfo)
{
    Bool32_t bSuccess = FALSE;
    PROCESS_MEMORY_COUNTERS pmc = { 0 };

    bSuccess = (Bool32_t)(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) != 0);

    if (bSuccess && pMemoryInfo != NULL)
    {
        pMemoryInfo->lwPhysicalSize = pmc.WorkingSetSize;
        pMemoryInfo->lwVirtualSize = pmc.PagefileUsage;
    }

    return bSuccess;
}

#elif defined(LINUX) || defined(SOLARIS)

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*!
*@brief        获取物理主机的内存信息
*@author       zhaohm3
*@param[in]    pMemoryInfo
*@retval
*@note
* 
*@since    2014-9-20 23:18
*@attention
* 
*/
GPublic Bool32_t os_get_physical_memory_info(PhysicalMemoryInfoP_t pMemoryInfo)
{
    Bool32_t bSuccess = FALSE;
    struct sysinfo meminfo = { 0 };

    bSuccess = (Bool32_t)(0 == sysinfo(&meminfo));

    if (bSuccess && pMemoryInfo != NULL)
    {
        pMemoryInfo->lwTotalPhys = meminfo.totalram;
        pMemoryInfo->lwAvailPhys = meminfo.freeram;
    }

    return bSuccess;
}

/*!
*@brief        获取当前进程的内存使用信息
*@author       zhaohm3
*@param[in]    pMemoryInfo
*@retval
*@note
* 
*@since    2014-9-20 23:18
*@attention
* 
*/
GPublic Bool32_t os_get_process_memory_info(ProccessMemoryInfoP_t pMemoryInfo)
{
    Bool32_t bSuccess = FALSE;
    Int32_t sdwFileID = -1, sdwReaded = -1;
    Char_t szMemoryInfo[1024] = { 0 }, szFileName[256] = { 0 };
    Addr_t pHead = NULL, pTail = NULL;
    Size64_t lwPhysicalSize = 0, lwVirtualSize = 0;

    sprintf(szFileName, "/proc/%d/status", getpid());

    sdwFileID = open(szFileName, 0, 0);
    GCMON_CHECK_COND(sdwFileID != -1, OSERROR);

    sdwReaded = read(sdwFileID, szMemoryInfo, sizeof(szMemoryInfo));
    GCMON_CHECK_COND(sdwReaded > 0 && sdwReaded < 1024, OSERROR);
    szMemoryInfo[sdwReaded - 1] = '\0';

    pHead = strstr(szMemoryInfo, "VmRSS:\t");
    GCMON_CHECK_NULL(pHead, OSERROR);
    pHead = pHead + strlen("VmRSS:\t");

    pTail = strstr(pHead, " kB");
    GCMON_CHECK_NULL(pTail, OSERROR);
    pTail[0] = '\0';

    lwPhysicalSize = atoll(pHead);

    pHead = strstr(szMemoryInfo, "VmSize:\t");
    GCMON_CHECK_NULL(pHead, OSERROR);
    pHead = pHead + strlen("VmSize:\t");

    pTail = strstr(pHead, " kB");
    GCMON_CHECK_NULL(pTail, OSERROR);
    pTail[0] = '\0';

    lwVirtualSize = atoll(pHead);

    if (pMemoryInfo != NULL)
    {
        pMemoryInfo->lwPhysicalSize = lwPhysicalSize * 1024L;
        pMemoryInfo->lwVirtualSize = lwVirtualSize * 1024L;
    }

    bSuccess = TRUE;

OSERROR:
    if (sdwFileID != -1)
    {
        close(sdwFileID);
    }

    return bSuccess;
}

#else
#error UnsupportedSystem
#endif
