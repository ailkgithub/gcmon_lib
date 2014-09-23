/*!**************************************************************
 *@file os.h
 *@brief    提供与操作系统无关的接口
 *@author   zhaohm3
 *@date 2014-9-20 20:14
 *@note
 * 
 ****************************************************************/

#ifndef _os_h__
#define _os_h__

#include "share/share.h"

//! 计算机的物理内存信息
typedef struct PhysicalMemoryInfo PhysicalMemoryInfo_t, *PhysicalMemoryInfoP_t;
struct PhysicalMemoryInfo
{
    Size64_t lwTotalPhys;                   //!< 计算机当前物理内存大小，单位(Byte)
    Size64_t lwAvailPhys;                   //!< 计算机当前可用物理内存大小，单位(Byte)
};

//! 进程当前的内存信息
typedef struct ProccessMemoryInfo ProccessMemoryInfo_t, *ProccessMemoryInfoP_t;
struct ProccessMemoryInfo
{
    Size64_t lwPhysicalSize;                //!< 进程所占用的物理内存大小，单位(Byte)
    Size64_t lwVirtualSize;                 //!< 进程所占用的虚拟内存大小，单位(Byte)
};

GPublic Bool32_t os_get_physical_memory_info(PhysicalMemoryInfoP_t pMemoryInfo);
GPublic Bool32_t os_get_process_memory_info(ProccessMemoryInfoP_t pMemoryInfo);
GPublic Int_t os_getpid();

#endif