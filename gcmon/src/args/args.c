/*!**************************************************************
 *@file args.c
 *@brief    �����������صĽӿ�ʵ��
 *@author   zhaohm3
 *@date 2014-9-22 9:55
 *@note
 * 
 ****************************************************************/

#include "args/args.h"
#include "perf/perf.h"

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
*@param[in]    szVmArgs     ���ݸ�JVM������ʱ����
*@param[in]    szPrefix     ��Ҫ��ȡ�Ĳ���ǰ׺������-Xms, -Xmx, -Xmn�ȵ�
*@retval
*@note         JVM�еĲ��������ظ����ã�������Ч�������һ��
* 
*@since    2014-9-22 10:47
*@attention
* 
*/
GPrivate String_t vmargs_get_last(String_t szVmArgs, String_t szPrefix)
{
    String_t szCurr = NULL, szLast = NULL;
    Size32_t dwPrefixSize = 0;

    GCMON_CHECK_NULL(szVmArgs, ERROR);
    GCMON_CHECK_NULL(szPrefix, ERROR);

    szCurr = strstr(szVmArgs, szPrefix);
    dwPrefixSize = (Size32_t)strlen(szPrefix);

    while (szCurr != NULL)
    {
        szLast = szCurr;
        szCurr += dwPrefixSize;
        szCurr = strstr(szCurr, szPrefix);
    }

ERROR:
    return szLast;
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
    szStart = szItor = vmargs_get_last(szVmArgs, szPrefix);
    GCMON_CHECK_NULL(szItor, ERROR);
    szItor += strlen(szPrefix);

    sdwScanf = sscanf(szItor, FMTL, &lwSize);
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
        strncpy(szArgs, szStart, wSize - 1);
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

    szLast1 = vmargs_get_last(szVmArgs, szPrefix1);
    szLast2 = vmargs_get_last(szVmArgs, szPrefix2);
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
