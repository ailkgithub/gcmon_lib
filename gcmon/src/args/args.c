/*!**************************************************************
*@file args.c
*@brief    �����������صĽӿ�ʵ��
*@author   zhaohm3
*@date 2014-9-22 9:55
*@note
*
****************************************************************/

#include "args/args.h"
#include "os/os.h"
#include "perf/perf.h"

//! ���ݸ�gcmon��ѡ����Ϣ
typedef struct AgentArgs AgentArgs_t, *AgentArgsP_t;
struct AgentArgs
{
    String_t szOptions;             //!< ���ݸ�gcmon������Options����
    String_t szOutpath;             //!< outpath=""��ָ��gcmon����ļ��Ĵ��·����Ĭ��Ϊ��ǰĿ¼
    String_t szOutname;             //!< outname=""��ָ����������ļ����ļ����ƣ�Ĭ��Ϊ��gcmon_pid_$pid.result
    String_t szOutstat;             //!< outstat="stdout" | "file"���Ƿ��ӡ��jstat�������Ƶ����ݣ�Ĭ�ϲ���ӡ
                                    //!< stdout��ʾ��ӡ����׼�����file��ʾ��ӡ���ļ�
};

//! ���ݸ�gcmon��ѡ��ֵ
GPrivate AgentArgs_t gAgentArgs = { 0 };

/*!
*@brief        ��pdi��������sun.rt.javaCommand��õ�JVM����������
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
*@brief        ��pdi��������java.rt.vmArgs��õ�JVM������ʱ��������
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
*@brief        ��JVM����ʱ�����л�ȡָ���������һ�γ��ֵ�λ��
*@author       zhaohm3
*@param[in]    szArgs       ���ݸ�JVM������ʱ����
*@param[in]    szPrefix     ��Ҫ��ȡ�Ĳ���ǰ׺������-Xms, -Xmx, -Xmn�ȵ�
*@retval
*@note         JVM�еĲ��������ظ����ã�������Ч�������һ��
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
*@brief        ��ȡ���ݸ���̬��gcmon��ѡ��ֵ
*@author       zhaohm3
*@param[in]    szPrefix
*@param[in]    pdwSize
*@retval
*@note
*
*@since    2014-9-24 15:32
*@attention
*
*/
GPrivate String_t agentargs_get_option(String_t szPrefix, IntP_t pdwSize)
{
    String_t szArgs = args_get_agentopts();
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

    if (pdwSize != NULL)
    {
        *pdwSize = dwSize - 1;
    }

ERROR:
    return szOption;
}

/*!
*@brief        ��ʼ��gAgentArgs.szOutpath
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 11:51
*@attention
* 
*/
GPrivate void agentargs_init_outpath()
{
    String_t szPath = NULL;
    Int_t dwSize = 0;
    Int_t ret = 0;

    szPath = agentargs_get_option("outpath=", &dwSize);
    GCMON_CHECK_NULL(szPath, ERROR);

    if ('/' == szPath[dwSize - 1] || '\\' == szPath[dwSize - 1])
    {
        szPath[dwSize - 1] = '\0';
        dwSize--;
    }

    //! ���Ŀ¼�����ڣ��򴴽�֮
    if (dwSize > 0 && os_access(szPath, 0) != 0)
    {
        ret = os_mkdir(szPath);
    }

ERROR:
    gAgentArgs.szOutpath = szPath;
    return;
}

/*!
*@brief        ��ʼ��gAgentArgs
*@author       zhaohm3
*@param[in]    szOptions
*@retval
*@note
* 
*@since    2014-9-25 12:36
*@attention
* 
*/
GPublic void args_init_agentargs(String_t szOptions)
{
    gAgentArgs.szOptions = szOptions;
    agentargs_init_outpath();
    gAgentArgs.szOutname = agentargs_get_option("outname=", NULL);
    gAgentArgs.szOutstat = agentargs_get_option("outstat=", NULL);
}

/*!
*@brief        �ͷ�gAgentArgs��ռ�õĶѿռ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 12:36
*@attention
* 
*/
GPublic void args_free_agentargs()
{
    GFREE(gAgentArgs.szOutpath);
    GFREE(gAgentArgs.szOutname);
    GFREE(gAgentArgs.szOutstat);
}

/*!
*@brief        ��ȡ���ݸ���̬��gcmon�Ĳ���
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-24 14:54
*@attention
*
*/
GPublic String_t args_get_agentopts()
{
    return gAgentArgs.szOptions;
}

/*!
*@brief        ��ȡ���ݸ���̬��gcmon��outpathֵ
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
    return gAgentArgs.szOutpath;
}

/*!
*@brief        ��ȡ���ݸ���̬��gcmon��outname��ֵ
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
    return gAgentArgs.szOutname;
}

/*!
*@brief        ��ȡ���ݸ���̬��gcmon��outstat��ֵ
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-25 11:22
*@attention
*
*/
GPublic String_t agentargs_get_outstat()
{
    return gAgentArgs.szOutstat;
}

/*!
*@brief        ����JVM������ʱ�������������ڴ��С�йصĲ���ֵ
*@author       zhaohm3
*@param[in]    szVmArgs     ���ݸ�JVM������ʱ����
*@param[in]    szPrefix     ��Ҫ��ȡ�Ĳ���ǰ׺������-Xms, -Xmx, -Xmn, -XX:MaxHeapSize=
*@param[out]   pszArgs      �����������ʾ����Ĳ�������
*@retval       ���ز��������õĴ�С����λ(Byte)
*@note         ע����ֵ������
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

    //! �˷����ܲ�����ֵ������˴�δ���м��
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
*@brief        ѡ�����յĲ���ֵ
*@author       zhaohm3
*@param[in]    szVmArgs     JVM��������
*@param[in]    szPrefix1
*@param[in]    szPrefix2
*@param[in]    pszArgs      ���ֵ����ʾ������Ч�Ĳ����������
*@retval
*@note         ��JVM�У�������-Xmn��-XX:NewSize���õ�Ч�������JVM��ѡ������б��е�
*              ���һ��ȥ�������յ�����ʱ����ֵ
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

/*!
*@brief        ͨ��-Xms��-XX:InitialHeapSize����InitialHeapSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:39
*@attention
* 
*/
GPublic Size64_t vmargs_parse_InitialHeapSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xms", "-XX:InitialHeapSize=", pszArgs);
}

/*!
*@brief        ͨ��-Xmx��-XX:MaxHeapSize����MaxHeapSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:40
*@attention
* 
*/
GPublic Size64_t vmargs_parse_MaxHeapSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmx", "-XX:MaxHeapSize=", pszArgs);
}

/*!
*@brief        ͨ��-Xmn��-XX:NewSize����NewSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:40
*@attention
* 
*/
GPublic Size64_t vmargs_parse_NewSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmn", "-XX:NewSize=", pszArgs);
}

/*!
*@brief        ͨ��-Xmn��-XX:MaxNewSize����MaxNewSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:40
*@attention
* 
*/
GPublic Size64_t vmargs_parse_MaxNewSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xmn", "-XX:MaxNewSize=", pszArgs);
}

/*!
*@brief        ͨ��-XX:OldSize����OldSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:41
*@attention
* 
*/
GPublic Size64_t vmargs_parse_OldSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:OldSize=", pszArgs);
}

/*!
*@brief        ͨ��-XX:PermSize����PermSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:41
*@attention
* 
*/
GPublic Size64_t vmargs_parse_PermSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:PermSize=", pszArgs);
}

/*!
*@brief        ͨ��-XX:MaxPermSize����MaxPermSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:41
*@attention
* 
*/
GPublic Size64_t vmargs_parse_MaxPermSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_size(szVmArgs, "-XX:MaxPermSize=", pszArgs);
}

/*!
*@brief        ͨ��-Xss��-XX:ThreadStackSize����ThreadStackSize
*@author       zhaohm3
*@param[in]    szVmArgs
*@param[in]    pszArgs
*@retval
*@note
* 
*@since    2014-9-25 15:41
*@attention
* 
*/
GPublic Size64_t vmargs_parse_ThreadStackSize(String_t szVmArgs, StringP_t pszArgs)
{
    return vmargs_parse_final_size(szVmArgs, "-Xss", "-XX:ThreadStackSize=", pszArgs);

}
