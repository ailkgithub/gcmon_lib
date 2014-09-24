/*!**************************************************************
 *@file args.c
 *@brief    与参数解析相关的接口实现
 *@author   zhaohm3
 *@date 2014-9-22 9:55
 *@note
 * 
 ****************************************************************/

#include "args/args.h"
#include "perf/perf.h"
#include "gcmon/gcmon.h"

/*!
*@brief        从pdi树中搜索sun.rt.javaCommand项，得到JVM的运行命令
*@author       zhaohm3
*@param[in]    pPdiTree
*@retval
*@note
* 
*@since    2014-9-23 10:43
*@attention
* 
*/
GPublic String_t args_get_javacmd(RBTreeP_t pPdiTree)
{
    String_t szCmdName = "sun.rt.javaCommand";
    String_t szJavaCmd = NULL;
    PerfDataItemP_t pCmd = NULL;

    GCMON_CHECK_NULL(pPdiTree, ERROR);
    pCmd = pdi_search_item(pPdiTree, szCmdName);
    GCMON_CHECK_NULL(pCmd, ERROR);
    szJavaCmd = pdi_get_string(pCmd);

ERROR:
    return szJavaCmd;
}

/*!
*@brief        从pdi树中搜索java.rt.vmArgs项，得到JVM的运行时启动参数
*@author       zhaohm3
*@param[in]    pPdiTree
*@retval
*@note
* 
*@since    2014-9-22 10:34
*@attention
* 
*/
GPublic String_t args_get_vmargs(RBTreeP_t pPdiTree)
{
    String_t szArgName = "java.rt.vmArgs";
    String_t szVmArgs = NULL;
    PerfDataItemP_t pPdiVmArgs = NULL;

    GCMON_CHECK_NULL(pPdiTree, ERROR);
    pPdiVmArgs = pdi_search_item(pPdiTree, szArgName);
    GCMON_CHECK_NULL(pPdiVmArgs, ERROR);
    szVmArgs = pdi_get_string(pPdiVmArgs);

ERROR:
    return szVmArgs;
}

/*!
*@brief        获取传递给动态库gcmon的参数
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-24 14:54
*@attention
* 
*/
GPublic String_t args_get_agentargs()
{
    return gcmon_get_agent_opts();
}

/*!
*@brief        从JVM运行时参数中获取指定参数最后一次出现的位置
*@author       zhaohm3
*@param[in]    szArgs       传递给JVM的运行时参数
*@param[in]    szPrefix     需要获取的参数前缀，比如-Xms, -Xmx, -Xmn等等
*@retval
*@note         JVM中的参数可以重复设置，最终生效的是最后一个
* 
*@since    2014-9-22 10:47
*@attention
* 
*/
GPrivate String_t args_get_last(String_t szArgs, String_t szPrefix)
{
    String_t szCurr = NULL, szLast = NULL;
    Size32_t dwPrefixSize = 0;

    GCMON_CHECK_NULL(szArgs, ERROR);
    GCMON_CHECK_NULL(szPrefix, ERROR);

    szCurr = os_strstr(szArgs, szPrefix);
    dwPrefixSize = (Size32_t)os_strlen(szPrefix);

    while (szCurr != NULL)
    {
        szLast = szCurr;
        szCurr += dwPrefixSize;
        szCurr = os_strstr(szCurr, szPrefix);
    }

ERROR:
    return szLast;
}

/*!
*@brief        获取传递给动态库gcmon的选项值
*@author       zhaohm3
*@param[in]    szPrefix
*@retval
*@note
* 
*@since    2014-9-24 15:32
*@attention
* 
*/
GPrivate String_t agentargs_get_option(String_t szPrefix)
{
    String_t szArgs = args_get_agentargs();
    String_t szStart = NULL, szItor = NULL;
    String_t szOption = NULL;
    Int_t dwSize = 0;

    szStart = args_get_last(szArgs, szPrefix);
    GCMON_CHECK_NULL(szStart, ERROR);
    szItor = szStart = szStart + os_strlen(szPrefix);
    while (szItor[0] != ',' && szItor[0] != '\0') szItor++;
    GCMON_CHECK_COND(szItor > szStart, ERROR);
    dwSize = szItor - szStart + 1;
    GMALLOC(szOption, Char_t, dwSize);
    GCMON_CHECK_NULL(szOption, ERROR);
    os_strncpy(szOption, szStart, dwSize - 1);
    szOption[dwSize - 1] = '\0';

ERROR:
    return szOption;
}

/*!
*@brief        获取传递给动态库gcmon的outpath值
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-24 14:59
*@attention
*
*/
GPublic String_t agentargs_get_outpath()
{
    String_t szPath = NULL, szOutpath = NULL;
    Int_t dwSize = 0;

    szPath = agentargs_get_option("outpath=");
    GCMON_CHECK_NULL(szPath, ERROR);
    dwSize = os_strlen(szPath);

    if (szPath[dwSize - 1] != '/' && szPath[dwSize - 1] != '\\')
    {
        GMALLOC(szOutpath, Char_t, dwSize + 2);
        GASSERT(szOutpath != NULL);
        os_sprintf(szOutpath, "%s/", szPath);
        szOutpath[dwSize + 1] = '\0';
        GFREE(szPath);
    }
    else
    {
        szOutpath = szPath;
    }

    //! 如果目录不存在，则创建之
    if (szOutpath != NULL && os_access(szOutpath, 0) != 0)
    {
        os_mkdir(szOutpath);
    }

ERROR:
    return szOutpath;
}

/*!
*@brief        获取传递给动态库gcmon的outname的值
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-24 15:00
*@attention
*
*/
GPublic String_t agentargs_get_outname()
{
    return agentargs_get_option("outname=");
}

/*!
*@brief        解析JVM的运行时启动参数中与内存大小有关的参数值
*@author       zhaohm3
*@param[in]    szVmArgs     传递给JVM的运行时参数
*@param[in]    szPrefix     需要获取的参数前缀，比如-Xms, -Xmx, -Xmn, -XX:MaxHeapSize=
*@param[out]   pszArgs      输出参数，表示具体的参数设置
*@retval       返回参数所设置的大小，单位(Byte)
*@note         注意数值溢出检测
* 
*@since    2014-9-22 10:35
*@attention
* 
*/
GPublic Size64_t vmargs_parse_size(String_t szVmArgs, String_t szPrefix, StringP_t pszArgs)
{
    Size64_t lwSize = 0;
    Int32_t sdwScanf = 0;
    String_t szStart = NULL, szItor = NULL;

    GCMON_CHECK_NULL(szVmArgs, ERROR);
    GCMON_CHECK_NULL(szPrefix, ERROR);
    szStart = szItor = args_get_last(szVmArgs, szPrefix);
    GCMON_CHECK_NULL(szItor, ERROR);
    szItor += os_strlen(szPrefix);

    sdwScanf = os_sscanf(szItor, FMTL, &lwSize);
    GCMON_CHECK_COND(1 == sdwScanf, ERROR);

    while (szItor[0] != '\0' && szItor[0] >= '0' && szItor[0] <= '9') {
        szItor++;
    }

    GCMON_CHECK_COND('\0' == szItor[0] || ' ' == szItor[0] || '\0' == szItor[1] || ' ' == szItor[1], ERROR);

    //! 乘法可能产生数值溢出，此处未进行检测
    switch (szItor[0])
    {
    case 'T': case 't':
        lwSize *= TB;
        break;
    case 'G': case 'g':
        lwSize *= GB;
        break;
    case 'M': case 'm':
        lwSize *= MB;
        break;
    case 'K': case 'k':
        lwSize *= KB;
        break;
    case '\0':
        break;
    default:
        lwSize = 0;
        break;
    }

    if (lwSize != 0 && pszArgs != NULL)
    {
        String_t szArgs = NULL;
        Int_t wSize = szItor - szStart + 2;
        GMALLOC(szArgs, Char_t, wSize);
        GCMON_CHECK_NULL(szArgs, ERROR);
        os_strncpy(szArgs, szStart, wSize - 1);
        szArgs[wSize - 1] = '\0';
        *pszArgs = szArgs;
    }

ERROR:
    return lwSize;
}

/*!
*@brief        选择最终的参数值
*@author       zhaohm3
*@param[in]    szVmArgs     JVM启动参数
*@param[in]    szPrefix1
*@param[in]    szPrefix2
*@param[in]    pszArgs      输出值，表示最终生效的参数设置情况
*@retval
*@note         在JVM中，有形如-Xmn和-XX:NewSize设置等效的情况，JVM会选择参数列表中的
*              最后一项去设置最终的运行时参数值
* 
*@since    2014-9-22 14:52
*@attention
* 
*/
GPrivate Size64_t vmargs_parse_final_size(String_t szVmArgs,
    String_t szPrefix1,
    String_t szPrefix2,
    StringP_t pszArgs)
{
    String_t szLast1 = NULL, szLast2 = NULL;
    String_t szPrefix = NULL, szArgs = NULL;
    Size64_t lwSize = 0;

    szLast1 = args_get_last(szVmArgs, szPrefix1);
    szLast2 = args_get_last(szVmArgs, szPrefix2);
    GCMON_CHECK_COND(szLast1 != NULL || szLast2 != NULL, ERROR);

    szPrefix = (szLast2 - szLast1 > 0) ? szPrefix2 : szPrefix1;
    lwSize = vmargs_parse_size((szLast2 - szLast1 > 0) ? szLast2 : szLast1, szPrefix, pszArgs);

ERROR:
    return lwSize;
}

GPublic Size64_t vmargs_parse_InitialHeapSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xms", "-XX:InitialHeapSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_MaxHeapSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmx", "-XX:MaxHeapSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_NewSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmn", "-XX:NewSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_MaxNewSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmn", "-XX:MaxNewSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_OldSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:OldSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_PermSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:PermSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_MaxPermSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:MaxPermSize=", pszArgs);
}

GPublic Size64_t vmargs_parse_ThreadStackSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xss", "-XX:ThreadStackSize=", pszArgs);

}
