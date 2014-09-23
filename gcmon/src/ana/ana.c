/*!**************************************************************
 *@file ana.c
 *@brief    JVM性能自动分析模块接口实现
 *@author   zhaohm3
 *@date 2014-9-22 17:30
 *@note
 * 
 ****************************************************************/

#include "ana/ana.h"
#include "args/args.h"
#include "os/os.h"
#include "sample/sample.h"

typedef struct MemAnaInfo MemAnaInfo_t, *MemAnaInfoP_t;
struct MemAnaInfo
{
    PhysicalMemoryInfo_t sPhys;
    ProccessMemoryInfo_t sProc;

    String_t szJavaCmd;
    String_t szVMArgs;

    Size64_t lwInitialHeapSize;
    Size64_t lwMaxHeapSize;
    Size64_t lwNewSize;
    Size64_t lwMaxNewSize;
    Size64_t lwOldSize;
    Size64_t lwPermSize;
    Size64_t lwMaxPermSize;
    Size64_t lwThreadStackSize;

    String_t szInitialHeapSize;
    String_t szMaxHeapSize;
    String_t szNewSize;
    String_t szMaxNewSize;
    String_t szOldSize;
    String_t szPermSize;
    String_t szMaxPermSize;
    String_t szThreadStackSize;
};

GPrivate Size64_t glwOSOverheadMemorySize = 100 * MB;               //!< 如果操作系统的剩余可用内存不足100M了，则建议清空缓存
GPrivate Double_t gdfJVMHeapFreeRatio = 0.95;                       //!< 如果JVM的实际内存使用量已经到了MaxHeapSize的90%了，HeapSize已经用完了

GPrivate FILE *ana_file_open(Int_t pid)
{
    Char_t szFileName[32] = { 0 };

    sprintf(szFileName, "gcmon_pid_%d.out", pid);
    return fopen(szFileName, "w+");
}

GPrivate void ana_file_close(FILE *pFile)
{
    if (pFile != NULL)
    {
        fclose(pFile);
    }
}

GPrivate void ana_file_flush(FILE *pFile)
{
    if (pFile != NULL)
    {
        fflush(pFile);
    }
}

GPrivate Int_t ana_file_print(FILE *pFile, const char *fmt, ...)
{
    Int_t len = 0;
    va_list args;

    va_start(args, fmt);
    len = vfprintf(pFile, fmt, args);
    va_end(args);
    return len;
}

GPrivate void mai_gather_info(RBTreeP_t pPdiTree, MemAnaInfoP_t pMemInfo)
{
    GCMON_CHECK_NULL(pMemInfo, ERROR);
    os_get_physical_memory_info(&pMemInfo->sPhys);
    os_get_process_memory_info(&pMemInfo->sProc);
    pMemInfo->szJavaCmd = args_get_javacmd(pPdiTree);
    pMemInfo->szVMArgs = args_get_vmargs(pPdiTree);
    pMemInfo->lwInitialHeapSize = vmargs_parse_InitialHeapSize(pMemInfo->szVMArgs, &pMemInfo->szInitialHeapSize);
    pMemInfo->lwMaxHeapSize = vmargs_parse_MaxHeapSize(pMemInfo->szVMArgs, &pMemInfo->szMaxHeapSize);
    pMemInfo->lwNewSize = vmargs_parse_NewSize(pMemInfo->szVMArgs, &pMemInfo->szNewSize);
    pMemInfo->lwMaxNewSize = vmargs_parse_MaxNewSize(pMemInfo->szVMArgs, &pMemInfo->szMaxNewSize);
    pMemInfo->lwOldSize = vmargs_parse_OldSize(pMemInfo->szVMArgs, &pMemInfo->szOldSize);
    pMemInfo->lwPermSize = vmargs_parse_PermSize(pMemInfo->szVMArgs, &pMemInfo->szPermSize);
    pMemInfo->lwMaxPermSize = vmargs_parse_MaxPermSize(pMemInfo->szVMArgs, &pMemInfo->szMaxPermSize);
    pMemInfo->lwThreadStackSize = vmargs_parse_ThreadStackSize(pMemInfo->szVMArgs, &pMemInfo->szThreadStackSize);

ERROR:
    return;
}

GPrivate void mai_release_info(MemAnaInfoP_t pMemInfo)
{
    GCMON_CHECK_NULL(pMemInfo, ERROR);
    GFREE(pMemInfo->szInitialHeapSize);
    GFREE(pMemInfo->szMaxHeapSize);
    GFREE(pMemInfo->szNewSize);
    GFREE(pMemInfo->szMaxNewSize);
    GFREE(pMemInfo->szOldSize);
    GFREE(pMemInfo->szPermSize);
    GFREE(pMemInfo->szMaxPermSize);
    GFREE(pMemInfo->szThreadStackSize);
    memset(pMemInfo, 0, sizeof(MemAnaInfo_t));

ERROR:
    return;
}

GPrivate void mai_print_verbose(FILE* pFile, MemAnaInfoP_t pMemInfo)
{
    GCMON_CHECK_NULL(pFile, ERROR);
    GCMON_CHECK_NULL(pMemInfo, ERROR);

    ana_file_print(pFile, "\n1.Operation System Memory Usage Information: \n\n");

    ana_file_print(pFile, "Physical Memory Total Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sPhys.lwTotalPhys));
    ana_file_print(pFile, "Physical Memory Available Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sPhys.lwAvailPhys));

    ana_file_print(pFile, "\n\n2.JVM Process Memory Usage Information: \n\n");

    ana_file_print(pFile, "Process Physical Memory Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sProc.lwPhysicalSize));
    ana_file_print(pFile, "Process Virtual Memory Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sProc.lwVirtualSize));

    ana_file_print(pFile, "\n\n3.JVM Arguments Information: \n\n");

    ana_file_print(pFile, "JVM Arguments : %s \n", pMemInfo->szVMArgs);
    ana_file_print(pFile, "Java Command : %s \n", pMemInfo->szJavaCmd);

    ana_file_print(pFile, "\n\n4.JVM Memory Setting Options Information: \n\n");

    if (pMemInfo->szInitialHeapSize != NULL)
    {
        ana_file_print(pFile, "InitialHeapSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwInitialHeapSize),
            pMemInfo->szInitialHeapSize);
    }

    if (pMemInfo->szMaxHeapSize != NULL)
    {
        ana_file_print(pFile, "MaxHeapSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxHeapSize),
            pMemInfo->szMaxHeapSize);
    }

    if (pMemInfo->szNewSize != NULL)
    {
        ana_file_print(pFile, "NewSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwNewSize),
            pMemInfo->szNewSize);
    }

    if (pMemInfo->szMaxNewSize != NULL)
    {
        ana_file_print(pFile, "MaxNewSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxNewSize),
            pMemInfo->szMaxNewSize);
    }

    if (pMemInfo->szOldSize != NULL)
    {
        ana_file_print(pFile, "OldSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwOldSize),
            pMemInfo->szOldSize);
    }

    if (pMemInfo->szPermSize != NULL)
    {
        ana_file_print(pFile, "PermSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwPermSize),
            pMemInfo->szPermSize);
    }

    if (pMemInfo->szMaxPermSize != NULL)
    {
        ana_file_print(pFile, "MaxPermSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxPermSize),
            pMemInfo->szMaxPermSize);
    }

    if (pMemInfo->szThreadStackSize != NULL)
    {
        ana_file_print(pFile, "ThreadStackSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwThreadStackSize),
            pMemInfo->szThreadStackSize);
    }

    ana_file_print(pFile, "\n\n5.JVM Generation Space and GC Status Information: \n\n");
    s_out_line(pFile, NULL);

ERROR:
    return;
}

GPublic void ana_OOM_Java_heap_space(RBTreeP_t pPdiTree)
{
    MemAnaInfo_t sMemInfo = { 0 };
    Double_t dfHeapUsed = 0;
    Double_t dfHeapFree = 0;
    Double_t dfHeapSize = 0;
    Double_t dfMaxHeapSize = 0;
    FILE *pAnaFile = NULL;

    dfHeapUsed = s_s0u() + s_s1u() + s_eu() + s_ou();
    dfHeapFree = s_s0f() + s_s1f() + s_ef() + s_of();
    dfHeapSize = s_ngc() + s_ogc();
    dfMaxHeapSize = s_ngcmx() + s_ogcmx();

    pAnaFile = ana_file_open(os_getpid());
    GCMON_CHECK_NULL(pAnaFile, ERROR);
    mai_gather_info(pPdiTree, &sMemInfo);
    mai_print_verbose(pAnaFile, &sMemInfo);

    ana_file_print(pAnaFile, "\n\n6.JVM OOM Message Information: \n\n");
    ana_file_print(pAnaFile, "java.lang.OutOfMemoryError: Java heap space\n");

    ana_file_print(pAnaFile, "\n\n7.JVM OOM Cause Information: \n\n");

    /*!
     * 1、当前已经申请的HeapSize已经大于MaxHeapSize的95%
     * 2、当前的HeapSize中95%的空间已经分配了对象了
     */
    if (dfHeapUsed >= dfMaxHeapSize * gdfJVMHeapFreeRatio)
    {
        //! 操作系统剩余的可用内存已经不足100MB了
        if (sMemInfo.sPhys.lwAvailPhys <= glwOSOverheadMemorySize)
        {
            /*!
             * 1、HeapSize已经用尽
             * 2、操作系统内存不足
             */
            ana_file_print(pAnaFile,
                "Max heap size is "FMTF"KB,\nCurrent used heap size is "FMTF"KB,\nHeap resource is exhausted.\n",
                 dfMaxHeapSize, dfHeapUsed);
            ana_file_print(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\nCurrent usable size is "FMTL"KB,\nPhysical memory resource is exhausted.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            ana_file_print(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            ana_file_print(pAnaFile, "Set a value bigger than "FMTL"KB to -Xmx or -XX:MaxHeapSize, and clear the application's cache.\n", (Size64_t)dfMaxHeapSize);
        }
        else
        {
            /*!
            * 1、HeapSize已经用尽
            * 2、操作系统内存还有剩余
            */
            ana_file_print(pAnaFile,
                "Max heap size is "FMTF"KB,\nCurrent used heap size is "FMTF"KB,\nHeap resource is exhausted.\n",
                 dfMaxHeapSize, dfHeapUsed);
            ana_file_print(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\nCurrent usable size is "FMTL"KB,\nPhysical memory resource is available.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            ana_file_print(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            ana_file_print(pAnaFile, "Set a value bigger than "FMTL"KB to -Xmx or -XX:MaxHeapSize.\n", (Size64_t)dfMaxHeapSize);
        }
    }
    else
    {
        //! 操作系统剩余的可用内存已经不足100MB了
        if (sMemInfo.sPhys.lwAvailPhys <= glwOSOverheadMemorySize)
        {
            /*!
            * 1、HeapSize还很充足
            * 2、操作系统内存不足
            */
            ana_file_print(pAnaFile,
                "Max heap size is "FMTF"KB,\nCurrent used heap size is "FMTF"KB,\nHeap resource is available.\n",
                 dfMaxHeapSize, dfHeapUsed);
            ana_file_print(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\nCurrent usable size is "FMTL"KB,\nPhysical memory resource is exhausted.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            ana_file_print(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            ana_file_print(pAnaFile, "Clear the application's cache.\n");
        }
        else
        {
            /*!
            * 1、HeapSize还很充足
            * 2、操作系统内存还有剩余
            */
            ana_file_print(pAnaFile,
                "Max heap size is "FMTF"KB,\nCurrent used heap size is "FMTF"KB,\nHeap resource is available.\n",
                 dfMaxHeapSize, dfHeapUsed);
            ana_file_print(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\nCurrent usable size is "FMTL"KB,\nPhysical memory resource is available.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            ana_file_print(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            ana_file_print(pAnaFile, "Reset each generation's capacity.\n");
        }
    }

ERROR:
    mai_release_info(&sMemInfo);
    ana_file_flush(pAnaFile);
    ana_file_close(pAnaFile);
    return;
}

GPublic void ana_OOM(RBTreeP_t pPdiTree, Int32_t sdwOOMType)
{
    GCMON_CHECK_NULL(pPdiTree, ERROR);

    switch (sdwOOMType)
    {
    case 0:
        ana_OOM_Java_heap_space(pPdiTree);
        break;
    default:
        break;
    }

ERROR:
    return;
}
