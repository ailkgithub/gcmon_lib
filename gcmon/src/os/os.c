/*!**************************************************************
 *@file os.c
 *@brief    提供与操作系统无关的接口
 *@author   zhaohm3
 *@date 2014-9-20 20:13
 *@note
 * 
 ****************************************************************/

#include "os/os.h"

/*!
*@brief        关闭文件
*@author       zhaohm3
*@param[in]    pFile
*@retval
*@note
*
*@since    2014-9-24 11:16
*@attention
*
*/
GPublic void os_fclose(FILE *file)
{
    if (file != NULL && file != stdout)
    {
        fclose(file);
    }
}

/*!
*@brief        刷新文件内容
*@author       zhaohm3
*@param[in]    pFile
*@retval
*@note
*
*@since    2014-9-24 11:16
*@attention
*
*/
GPublic void os_fflush(FILE *file)
{
    if (file != NULL)
    {
        fflush(file);
    }
}

#ifdef WIN32

#include <windows.h>
#include <psapi.h>
#include <process.h>
#include <io.h>
#include <direct.h>

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
GPublic Int32_t os_getpid()
{
    return _getpid();
}

/*!
*@brief        打开文件
*@author       zhaohm3
*@param[in]    filename
*@param[in]    mode
*@retval
*@note
* 
*@since    2014-9-24 11:07
*@attention
* 
*/
GPublic FILE *os_fopen(String_t filename, String_t mode)
{
    FILE *file = NULL;
    errno_t e = fopen_s(&file, filename, mode);

    return file;
}

/*!
*@brief        打开文件
*@author       zhaohm3
*@param[in]    path
*@param[in]    oflag
*@param[in]    pmode
*@retval
*@note
* 
*@since    2014-9-24 17:14
*@attention
* 
*/
GPublic Int32_t os_open(String_t path, Int32_t oflag, Int32_t pmode)
{
    return _open(path, oflag, pmode);
}

/*!
*@brief        关闭文件
*@author       zhaohm3
*@param[in]    fd
*@retval
*@note
* 
*@since    2014-9-24 17:14
*@attention
* 
*/
GPublic Int32_t os_close(Int32_t fd)
{
    return _close(fd);
}

/*!
*@brief        读取文件
*@author       zhaohm3
*@param[in]    fd
*@param[in]    buffer
*@param[in]    count
*@retval
*@note
* 
*@since    2014-9-24 17:15
*@attention
* 
*/
GPublic Int32_t os_read(Int32_t fd, Addr_t buffer, Count32_t count)
{
    return _read(fd, buffer, count);
}


/*!
*@brief        判断文件的访问属性
*@author       zhaohm3
*@param[in]    path
*@param[in]    mode
*@retval
*@note
* 
*@since    2014-9-24 17:15
*@attention
* 
*/
GPublic Int32_t os_access(String_t path, Int32_t mode)
{
    return _access(path, mode);
}

/*!
*@brief        删除文件或者目录
*@author       zhaohm3
*@param[in]    filename
*@retval
*@note
* 
*@since    2014-9-24 17:17
*@attention
* 
*/
GPublic Int32_t os_unlink(String_t filename)
{
    return _unlink(filename);
}

/*!
*@brief        创建目录
*@author       zhaohm3
*@param[in]    path
*@retval
*@note
* 
*@since    2014-9-24 17:15
*@attention
* 
*/
GPublic Int32_t os_mkdir(String_t path)
{
    Int32_t ret = 0;
    Char_t command[256] = { 0 };
    os_sprintf(command, "mkdir %s", path);
    ret = system(command);
    return ret;
}

/*!
*@brief        当前线程进入睡眠期
*@author       zhaohm3
*@param[in]    msec     睡眠的毫秒数
*@retval
*@note
* 
*@since    2014-10-16 13:01
*@attention
* 
*/
GPublic void os_sleep(Count32_t msec)
{
    Sleep((DWORD)msec);
}

#elif defined(LINUX) || defined(SOLARIS)

#include <unistd.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>

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

    os_sprintf(szFileName, "/proc/%d/status", os_getpid());

    sdwFileID = os_open(szFileName, 0, 0);
    GCMON_CHECK_COND(sdwFileID != -1, OSERROR);
    sdwReaded = os_read(sdwFileID, szMemoryInfo, sizeof(szMemoryInfo));
    GCMON_CHECK_COND(sdwReaded > 0 && sdwReaded < 1024, OSERROR);
    szMemoryInfo[sdwReaded - 1] = '\0';

    szItor = os_strstr(szMemoryInfo, "VmRSS:\t");
    GCMON_CHECK_NULL(szItor, OSERROR);
    szItor = szItor + os_strlen("VmRSS:\t");
    sdwScanf = os_sscanf(szItor, FMTL, &lwPhysicalSize);
    GCMON_CHECK_COND(1 == sdwScanf, OSERROR);

    szItor = os_strstr(szMemoryInfo, "VmSize:\t");
    GCMON_CHECK_NULL(szItor, OSERROR);
    szItor = szItor + os_strlen("VmSize:\t");
    sdwScanf = os_sscanf(szItor, FMTL, &lwVirtualSize);
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
        os_close(sdwFileID);
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
GPublic Int32_t os_getpid()
{
    return getpid();
}

/*!
*@brief        打开文件
*@author       zhaohm3
*@param[in]    filename
*@param[in]    mode
*@retval
*@note
*
*@since    2014-9-24 11:07
*@attention
*
*/
GPublic FILE *os_fopen(String_t filename, String_t mode)
{
    return fopen(filename, mode);
}

/*!
*@brief        打开文件
*@author       zhaohm3
*@param[in]    path
*@param[in]    oflag
*@param[in]    pmode
*@retval
*@note
*
*@since    2014-9-24 17:14
*@attention
*
*/
GPublic Int32_t os_open(String_t path, Int32_t oflag, Int32_t pmode)
{
    return open(path, oflag, pmode);
}

/*!
*@brief        关闭文件
*@author       zhaohm3
*@param[in]    fd
*@retval
*@note
*
*@since    2014-9-24 17:14
*@attention
*
*/
GPublic Int32_t os_close(Int32_t fd)
{
    return close(fd);
}

/*!
*@brief        读取文件
*@author       zhaohm3
*@param[in]    fd
*@param[in]    buffer
*@param[in]    count
*@retval
*@note
*
*@since    2014-9-24 17:15
*@attention
*
*/
GPublic Int32_t os_read(Int32_t fd, Addr_t buffer, Count32_t count)
{
    return read(fd, buffer, count);
}


/*!
*@brief        判断文件的访问属性
*@author       zhaohm3
*@param[in]    path
*@param[in]    mode
*@retval
*@note
*
*@since    2014-9-24 17:15
*@attention
*
*/
GPublic Int32_t os_access(String_t path, Int32_t mode)
{
    return access(path, mode);
}

/*!
*@brief        删除文件或者目录
*@author       zhaohm3
*@param[in]    filename
*@retval
*@note
*
*@since    2014-9-24 17:17
*@attention
*
*/
GPublic Int32_t os_unlink(String_t filename)
{
    Int32_t ret = 0;
    Char_t command[256] = { 0 };
    os_sprintf(command, "rm -rf %s", filename);
    ret = system(command);
    return ret;
}

/*!
*@brief        创建目录
*@author       zhaohm3
*@param[in]    path
*@retval
*@note
*
*@since    2014-9-24 17:15
*@attention
*
*/
GPublic Int32_t os_mkdir(String_t path)
{
    Int32_t ret = 0;
    Char_t command[256] = { 0 };
    os_sprintf(command, "mkdir -p %s", path);
    ret = system(command);
    return ret;
}

/*!
*@brief        当前线程进入睡眠期
*@author       zhaohm3
*@param[in]    msec     睡眠的毫秒数
*@retval
*@note
* 
*@since    2014-10-16 13:01
*@attention
* 
*/
GPublic void os_sleep(Count32_t msec)
{
    usleep(1000 * msec);
}

#else
#error UnsupportedSystem
#endif

