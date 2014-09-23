/*!**************************************************************
 *@file os.c
 *@brief    提供与操作系统无关的接口
 *@author   zhaohm3
 *@date 2014-9-20 20:13
 *@note
 * 
 ****************************************************************/

#include "os/os.h"

#ifdef WIN32

#include <windows.h>
#include <psapi.h>
#include <process.h>

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

/*!
*@brief        获取进程的pid
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-23 15:02
*@attention
* 
*/
GPublic Int_t os_getpid()
{
    return (Int_t)_getpid();
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
    Int32_t sdwFileID = -1, sdwReaded = -1, sdwScanf = 0;
    Char_t szMemoryInfo[1024] = { 0 }, szFileName[256] = { 0 };
    String_t szItor = NULL;
    Size64_t lwPhysicalSize = 0, lwVirtualSize = 0;

    sprintf(szFileName, "/proc/%d/status", getpid());

    sdwFileID = open(szFileName, 0, 0);
    GCMON_CHECK_COND(sdwFileID != -1, OSERROR);
    sdwReaded = read(sdwFileID, szMemoryInfo, sizeof(szMemoryInfo));
    GCMON_CHECK_COND(sdwReaded > 0 && sdwReaded < 1024, OSERROR);
    szMemoryInfo[sdwReaded - 1] = '\0';

    szItor = strstr(szMemoryInfo, "VmRSS:\t");
    GCMON_CHECK_NULL(szItor, OSERROR);
    szItor = szItor + strlen("VmRSS:\t");
    sdwScanf = sscanf(szItor, FMTL, &lwPhysicalSize);
    GCMON_CHECK_COND(1 == sdwScanf, OSERROR);

    szItor = strstr(szMemoryInfo, "VmSize:\t");
    GCMON_CHECK_NULL(szItor, OSERROR);
    szItor = szItor + strlen("VmSize:\t");
    sdwScanf = sscanf(szItor, FMTL, &lwVirtualSize);
    GCMON_CHECK_COND(1 == sdwScanf, OSERROR);

    if (pMemoryInfo != NULL)
    {
        pMemoryInfo->lwPhysicalSize = lwPhysicalSize * KB;
        pMemoryInfo->lwVirtualSize = lwVirtualSize * KB;
    }

    bSuccess = TRUE;

OSERROR:
    if (sdwFileID != -1)
    {
        close(sdwFileID);
    }

    return bSuccess;
}

/*!
*@brief        获取进程的pid
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-23 15:02
*@attention
* 
*/
GPublic Int_t os_getpid()
{
    return (Int_t)getpid();
}

#else
#error UnsupportedSystem
#endif

