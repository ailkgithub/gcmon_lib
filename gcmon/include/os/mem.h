/*!**************************************************************
 *@file mem.h
 *@brief    �ṩ�����ϵͳ�޹ص��ڴ���Ϣ��ȡ�ӿ�
 *@author   zhaohm3
 *@date 2014-9-20 20:14
 *@note
 * 
 ****************************************************************/

#ifndef _mem_h__
#define _mem_h__

#include "share/share.h"

//! ������������ڴ���Ϣ
typedef struct PhysicalMemoryInfo PhysicalMemoryInfo_t, *PhysicalMemoryInfoP_t;
struct PhysicalMemoryInfo
{
    Size64_t lwTotalPhys;                   //!< �������ǰ�����ڴ��С����λ(Byte)
    Size64_t lwAvailPhys;                   //!< �������ǰ���������ڴ��С����λ(Byte)
};

//! ���̵�ǰ���ڴ���Ϣ
typedef struct ProccessMemoryInfo ProccessMemoryInfo_t, *ProccessMemoryInfoP_t;
struct ProccessMemoryInfo
{
    Size64_t lwPhysicalSize;                //!< ������ռ�õ������ڴ��С����λ(Byte)
    Size64_t lwVirtualSize;                 //!< ������ռ�õ������ڴ��С����λ(Byte)
};

GPublic Bool32_t os_get_physical_memory_info(PhysicalMemoryInfoP_t pMemoryInfo);
GPublic Bool32_t os_get_process_memory_info(ProccessMemoryInfoP_t pMemoryInfo);

#endif