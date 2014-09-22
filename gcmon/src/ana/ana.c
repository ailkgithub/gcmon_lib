/*!**************************************************************
 *@file ana.c
 *@brief    JVM性能自动分析模块接口实现
 *@author   zhaohm3
 *@date 2014-9-22 17:30
 *@note
 * 
 ****************************************************************/

#include "ana/ana.h"
#include "rbtree/rbtree.h"
#include "args/args.h"
#include "os/mem.h"

typedef struct GAnalyzeInfo GAnalyzeInfo_t, *GAnalyzeInfoP_t;
struct GAnalyzeInfo
{
    PhysicalMemoryInfo_t sPhys;
    ProccessMemoryInfo_t sProc;
    Size64_t lwInitialHeapSize;
    Size64_t lwMaxHeapSize;
    Size64_t lwNewSize;
    Size64_t lwMaxNewSize;
    Size64_t lwOldSize;
    Size64_t lwPermSize;
    Size64_t lwMaxPermSize;
    Size64_t lwThreadStackSize;

    String_t szVMArgs;
    String_t szInitialHeapSize;
    String_t szMaxHeapSize;
    String_t szNewSize;
    String_t szMaxNewSize;
    String_t szOldSize;
    String_t szPermSize;
    String_t szMaxPermSize;
    String_t szThreadStackSize;
};

GPrivate void ana_OOM_Java_heap_space(RBTreeP_t pPdiTree)
{
    PhysicalMemoryInfo_t sPhys = { 0 };
    ProccessMemoryInfo_t sProc = { 0 };

    os_get_physical_memory_info(&sPhys);
    os_get_process_memory_info(&sProc);
}
