/*!**************************************************************
 *@file share.cpp
 *@brief    ����ӿڶ���
 *@author   zhaohm3
 *@date 2014-9-3 18:23
 *@note
 * 
 ****************************************************************/

#include "share/share.h"

//! ��¼������Ϣ
GPrivate FILE *gFile = NULL;

//! ��gFile
GPublic void gcmon_debug_fopen()
{
#ifdef DEBUG
    if (NULL == gFile)
    {
        gFile = fopen("gcmon_debug.txt", "w+");
    }
#endif
}

//! �ر�gFile
GPublic void gcmon_debug_fclose()
{
    if (gFile != NULL)
    {
        fflush(gFile);
        fclose(gFile);
    }
    else
    {
        fflush(stdout);
    }
}

//! flush�ļ�����
GPublic void gcmon_debug_flush()
{
    if (gFile != NULL)
    {
        fflush(gFile);
    }
    else
    {
        fflush(stdout);
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
    va_list args = NULL;

    va_start(args, fmt);
    if (gFile != NULL)
    {
        len = vfprintf(gFile, fmt, args);
    }
    else
    {
        len = vprintf(fmt, args);
    }
    va_end(args);
    return len;
#else
    return 0;
#endif
}
