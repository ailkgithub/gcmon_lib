/*!**************************************************************
 *@file ana.c
 *@brief    JVM性能自动分析模块接口实现
 *@author   zhaohm3
 *@date 2014-9-22 17:30
 *@note
 * 
 ****************************************************************/

#include "ana/ana.h"
#include "os/os.h"
#include "gcmon/gcmon.h"
#include "args/args.h"
#include "sample/sample.h"
#include "file/file.h"

//! JVM运行时的内存信息
typedef struct MemAnaInfo MemAnaInfo_t, *MemAnaInfoP_t;
struct MemAnaInfo
{
    PhysicalMemoryInfo_t sPhys;         //!< 物理内存使用情况
    ProccessMemoryInfo_t sProc;         //!< JVM进程的内存使用情况

    Size64_t lwInitialHeapSize;         //!< -XX:InitialHeapSize
    Size64_t lwMaxHeapSize;             //!< -XX:MaxHeapSize
    Size64_t lwNewSize;                 //!< -XX:NewSize
    Size64_t lwMaxNewSize;              //!< -XX:MaxNewSize
    Size64_t lwOldSize;                 //!< -XX:OldSize
    Size64_t lwPermSize;                //!< -XX:PermSize
    Size64_t lwMaxPermSize;             //!< -XX:MaxPermSize
    Size64_t lwThreadStackSize;         //!< -XX:ThreadStackSize

    String_t szInitialHeapSize;         //!< -XX:InitialHeapSize
    String_t szMaxHeapSize;             //!< -XX:MaxHeapSize
    String_t szNewSize;                 //!< -XX:NewSize
    String_t szMaxNewSize;              //!< -XX:MaxNewSize
    String_t szOldSize;                 //!< -XX:OldSize
    String_t szPermSize;                //!< -XX:PermSize
    String_t szMaxPermSize;             //!< -XX:MaxPermSize
    String_t szThreadStackSize;         //!< -XX:ThreadStackSize

    String_t szJavaCmd;                 //!< Java Command
    String_t szVMArgs;                  //!< JVM Arguments
    String_t szAgentOpts;               //!< GCMON Agent Options
};

//! 如果操作系统的剩余可用内存不足100M了，则建议清空缓存
GPrivate Size64_t glwOSOverheadMemorySize = 100 * MB;

//! 如果JVM的实际内存使用量已经到了MaxHeapSize的90%了，HeapSize已经用完了
GPrivate Double_t gdfJVMHeapFreeRatio = 0.95;

/*!
*@brief        收集JVM的内存以及运行时参数信息
*@author       zhaohm3
*@param[in]    pPdiTree
*@param[in]    pMemInfo
*@retval
*@note
* 
*@since    2014-9-24 17:49
*@attention
* 
*/
GPrivate void mai_gather_info(RBTreeP_t pPdiTree, MemAnaInfoP_t pMemInfo)
{
    GCMON_CHECK_NULL(pMemInfo, ERROR);
    os_get_physical_memory_info(&pMemInfo->sPhys);
    os_get_process_memory_info(&pMemInfo->sProc);
    pMemInfo->szJavaCmd = args_get_javacmd(pPdiTree);
    pMemInfo->szVMArgs = args_get_vmargs(pPdiTree);
    pMemInfo->szAgentOpts = args_get_agentopts();
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

/*!
*@brief        释放JVM参数信息所占用的存储空间
*@author       zhaohm3
*@param[in]    pMemInfo
*@retval
*@note
* 
*@since    2014-9-24 17:50
*@attention
* 
*/
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
    os_memset(pMemInfo, 0, sizeof(MemAnaInfo_t));

ERROR:
    return;
}

/*!
*@brief        输出JVM运行时信息
*@author       zhaohm3
*@param[in]    pFile
*@param[in]    pMemInfo
*@retval
*@note
* 
*@since    2014-9-24 17:50
*@attention
* 
*/
GPrivate void mai_print_verbose(FILE* pFile, MemAnaInfoP_t pMemInfo)
{
    GCMON_CHECK_NULL(pFile, ERROR);
    GCMON_CHECK_NULL(pMemInfo, ERROR);

    os_fprintf(pFile, "\n1.Operation System Memory Usage Information: \n\n");

    os_fprintf(pFile, "Physical Memory Total Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sPhys.lwTotalPhys));
    os_fprintf(pFile, "Physical Memory Available Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sPhys.lwAvailPhys));

    os_fprintf(pFile, "\n\n2.JVM Process Memory Usage Information: \n\n");

    os_fprintf(pFile, "Process Physical Memory Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sProc.lwPhysicalSize));
    os_fprintf(pFile, "Process Virtual Memory Size : "FMTL"KB\n", BYTE2KB(pMemInfo->sProc.lwVirtualSize));

    os_fprintf(pFile, "\n\n3.JVM Arguments Information: \n\n");

    if (pMemInfo->szVMArgs != NULL)
    {
        os_fprintf(pFile, "JVM Arguments : %s \n", pMemInfo->szVMArgs);
    }

    os_fprintf(pFile, "Java Command : %s \n", pMemInfo->szJavaCmd);

    if (pMemInfo->szAgentOpts != NULL)
    {
        os_fprintf(pFile, "GCMON Options : %s \n", pMemInfo->szAgentOpts);
    }

    os_fprintf(pFile, "\n\n4.JVM Memory Setting Options Information: \n\n");

    if (pMemInfo->szInitialHeapSize != NULL)
    {
        os_fprintf(pFile, "InitialHeapSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwInitialHeapSize),
            pMemInfo->szInitialHeapSize);
    }

    if (pMemInfo->szMaxHeapSize != NULL)
    {
        os_fprintf(pFile, "MaxHeapSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxHeapSize),
            pMemInfo->szMaxHeapSize);
    }

    if (pMemInfo->szNewSize != NULL)
    {
        os_fprintf(pFile, "NewSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwNewSize),
            pMemInfo->szNewSize);
    }

    if (pMemInfo->szMaxNewSize != NULL)
    {
        os_fprintf(pFile, "MaxNewSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxNewSize),
            pMemInfo->szMaxNewSize);
    }

    if (pMemInfo->szOldSize != NULL)
    {
        os_fprintf(pFile, "OldSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwOldSize),
            pMemInfo->szOldSize);
    }

    if (pMemInfo->szPermSize != NULL)
    {
        os_fprintf(pFile, "PermSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwPermSize),
            pMemInfo->szPermSize);
    }

    if (pMemInfo->szMaxPermSize != NULL)
    {
        os_fprintf(pFile, "MaxPermSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwMaxPermSize),
            pMemInfo->szMaxPermSize);
    }

    if (pMemInfo->szThreadStackSize != NULL)
    {
        os_fprintf(pFile, "ThreadStackSize : "FMTL"KB --> %s\n",
            BYTE2KB(pMemInfo->lwThreadStackSize),
            pMemInfo->szThreadStackSize);
    }

    os_fprintf(pFile, "\n\n5.JVM Generation Space and GC Status Information: \n\n");
    s_out_line(pFile, NULL);

ERROR:
    return;
}

/*!
*@brief        对OOM的Java Heap Space异常进行诊断
*@author       zhaohm3
*@param[in]    pPdiTree
*@param[in]    sdwOOMType
*@retval
*@note
* 
*@since    2014-9-24 17:51
*@attention
* 
*/
GPrivate void ana_OOM_Java_heap_space(RBTreeP_t pPdiTree, Int32_t sdwOOMType)
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

    pAnaFile = file_get_fresult();
    GCMON_CHECK_NULL(pAnaFile, ERROR);
    mai_gather_info(pPdiTree, &sMemInfo);
    mai_print_verbose(pAnaFile, &sMemInfo);

    os_fprintf(pAnaFile, "\n\n6.JVM OOM Message Information: \n\n");
    os_fprintf(pAnaFile, "java.lang.OutOfMemoryError: %s\n", gcmon_get_oom_desc(sdwOOMType));

    os_fprintf(pAnaFile, "\n\n7.JVM OOM Cause Information: \n\n");

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
            os_fprintf(pAnaFile,
                "Max heap size is "FMTF"KB,\n"
                "Current used heap size is "FMTF"KB,\n"
                "Heap resource is exhausted.\n",
                 dfMaxHeapSize, dfHeapUsed);
            os_fprintf(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\n"
                "Current usable size is "FMTL"KB,\n"
                "Physical memory resource is exhausted.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            os_fprintf(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            os_fprintf(pAnaFile,
                "Set a value bigger than "FMTL"KB to -Xmx or -XX:MaxHeapSize,"
                " and clear the application's cache.\n",
                (Size64_t)dfMaxHeapSize);
        }
        else
        {
            /*!
            * 1、HeapSize已经用尽
            * 2、操作系统内存还有剩余
            */
            os_fprintf(pAnaFile,
                "Max heap size is "FMTF"KB,\n"
                "Current used heap size is "FMTF"KB,\n"
                "Heap resource is exhausted.\n",
                 dfMaxHeapSize, dfHeapUsed);
            os_fprintf(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\n"
                "Current usable size is "FMTL"KB,\n"
                "Physical memory resource is available.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            os_fprintf(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            os_fprintf(pAnaFile,
                "Set a value bigger than "FMTL"KB to -Xmx or -XX:MaxHeapSize.\n",
                (Size64_t)dfMaxHeapSize);
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
            os_fprintf(pAnaFile,
                "Max heap size is "FMTF"KB,\n"
                "Current used heap size is "FMTF"KB,\n"
                "Heap resource is available.\n",
                 dfMaxHeapSize, dfHeapUsed);
            os_fprintf(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\n"
                "Current usable size is "FMTL"KB,\n"
                "Physical memory resource is exhausted.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            os_fprintf(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            os_fprintf(pAnaFile, "Clear the application's cache.\n");
        }
        else
        {
            /*!
            * 1、HeapSize还很充足
            * 2、操作系统内存还有剩余
            */
            os_fprintf(pAnaFile,
                "Max heap size is "FMTF"KB,\n"
                "Current used heap size is "FMTF"KB,\n"
                "Heap resource is available.\n",
                 dfMaxHeapSize, dfHeapUsed);
            os_fprintf(pAnaFile,
                "\nPhysical memory size is "FMTL"KB,\n"
                "Current usable size is "FMTL"KB,\n"
                "Physical memory resource is available.\n",
                 BYTE2KB(sMemInfo.sPhys.lwTotalPhys),
                 BYTE2KB(sMemInfo.sPhys.lwAvailPhys));
            os_fprintf(pAnaFile, "\n\n8.JVM OOM Analyze Suggestions: \n\n");
            os_fprintf(pAnaFile, "Reset each generation's capacity.\n");
        }
    }

    if (GOOM_PERM_SPACE == sdwOOMType)
    {
        os_fprintf(pAnaFile, "And PermSize is "FMTF"KB, "
            "current used "FMTF"KB, "
            "and "FMTF"KB left.\n"
            "Please set a more proper value to -XX:PermSize or -XX:MaxPermSize.\n",
            s_pc(), s_pu(), s_pf());
    }

ERROR:
    mai_release_info(&sMemInfo);
    return;
}

/*!
*@brief        诊断OOM异常
*@author       zhaohm3
*@param[in]    pPdiTree
*@param[in]    sdwOOMType
*@retval
*@note
* 
*@since    2014-9-24 17:51
*@attention
* 
*/
GPublic void ana_OOM(RBTreeP_t pPdiTree, Int32_t sdwOOMType)
{
    GCMON_CHECK_NULL(pPdiTree, ERROR);

    switch (sdwOOMType)
    {
    case GOOM_HEAP_SPACE:
    case GOOM_OVERHEAD_LIMIT:
    case GOOM_PERM_SPACE:
        ana_OOM_Java_heap_space(pPdiTree, sdwOOMType);
        break;
    default:
        break;
    }

ERROR:
    return;
}
