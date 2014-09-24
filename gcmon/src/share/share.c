/*!**************************************************************
 *@file share.c
 *@brief    ����ӿڶ���
 *@author   zhaohm3
 *@date 2014-9-3 18:23
 *@note
 * 
 ****************************************************************/

#include "share/share.h"
#include "args/args.h"

//! ��¼������Ϣ
GPrivate FILE *gpDebugFile = NULL;

//! ��gFile
GPublic void gcmon_debug_fopen()
{
#ifdef DEBUG
    if (NULL == gpDebugFile)
    {
        Char_t szFileName[256] = { 0 };
        String_t szOutpath = agentargs_get_outpath();
        String_t szOutname = agentargs_get_outname();

        os_sprintf(szFileName, "%s%s_pid_%d_debug",
            (szOutpath != NULL) ? szOutpath : "",
            (szOutname != NULL) ? szOutname : "gcmon",
            os_getpid());

        GFREE(szOutpath);
        GFREE(szOutname);

        gpDebugFile = os_fopen(szFileName, "w+");
    }
#endif
}

//! �ر�gFile
GPublic void gcmon_debug_fclose()
{
    if (gpDebugFile != NULL)
    {
        os_fflush(gpDebugFile);
        os_fclose(gpDebugFile);
    }
    else
    {
        os_fflush(stdout);
    }
}

//! flush�ļ�����
GPublic void gcmon_debug_flush()
{
    if (gpDebugFile != NULL)
    {
        os_fflush(gpDebugFile);
    }
    else
    {
        os_fflush(stdout);
    }
}

 /*!
 *@brief        ��ӡgcmon���й����еĵ�����Ϣ
 *@author       zhaohm3
 *@param[in]    fmt
 *@retval       
 *@note         ����ͨ�����Ĵ˽ӿڣ���������Ϣ��ӡ��ָ�����ļ���
 * 
 *@since    2014-9-3 18:23
 *@attention    �˽ӿ���Debug�汾����Ч
 * 
 */
int gcmon_debug_msg(const char *fmt, ...)
{
#ifdef DEBUG
    int len = 0;
    va_list args;

    va_start(args, fmt);
    if (gpDebugFile != NULL)
    {
        len = os_vfprintf(gpDebugFile, fmt, args);
    }
    else
    {
        len = os_vprintf(fmt, args);
    }
    va_end(args);
    return len;
#else
    return 0;
#endif
}
