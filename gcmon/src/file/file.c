/*!**************************************************************
 *@file file.c
 *@brief    ���ļ�������صĽӿ�ʵ��
 *@author   zhaohm3
 *@date 2014-9-25 12:54
 *@note
 * 
 ****************************************************************/

#include "file/file.h"
#include "os/os.h"
#include "args/args.h"

GPrivate FILE *gpDebugFile = NULL;          //!< ��¼������Ϣ������DEBUGģʽ����Ч
GPrivate FILE *gpResultFile = NULL;         //!< ���gcmon��Ͻ�����ļ�����outnameѡ��ָ��
GPrivate FILE *gpStatFile = NULL;           //!< ���jstat���ݵ��ļ�����outname��outstat��ֵ��ͬ����

/*!
*@brief        ͨ��outpath��outname���ṩ�Ĺ����׺���ƴ����ļ�
*@author       zhaohm3
*@param[in]    szPostfix    �ļ���׺����
*@retval       ����$(outpath)/$(outname)_pid_$(pid).$(szPostfix)�ļ�
*@note
* 
*@since    2014-9-25 14:27
*@attention
* 
*/
GPrivate FILE* file_open_fpostfix(String_t szPostfix)
{
    FILE *pFile = NULL;
    Char_t szFileName[256] = { 0 };
    String_t szOutpath = agentargs_get_outpath();
    String_t szOutname = agentargs_get_outname();

    os_sprintf(szFileName, "%s%s%s_pid_%d.%s",
        (szOutpath != NULL) ? szOutpath : "",
        (szOutpath != NULL) ? "/" : "",
        (szOutname != NULL) ? szOutname : "gcmon",
        os_getpid(),
        (szPostfix != NULL) ? szPostfix : "unknown");

    pFile = os_fopen(szFileName, "w+");
    return pFile;
}

/*!
*@brief        ����������Ϣ�ļ�¼�ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:30
*@attention
* 
*/
GPublic void file_open_fdebug()
{
#ifdef DEBUG
    gpDebugFile = file_open_fpostfix("debug");
#endif
}

/*!
*@brief        ���������Ͻ�����ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:30
*@attention
* 
*/
GPrivate void file_open_fresult()
{
    gpResultFile = file_open_fpostfix("result");
}

/*!
*@brief        �����������ʱ״̬���ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:30
*@attention
* 
*/
GPrivate void file_open_fstat()
{
    String_t szOutstat = agentargs_get_outstat();

    if (szOutstat != NULL)
    {
        if (0 == os_strcmp(szOutstat, "stdout"))
        {
            gpStatFile = stdout;
        }
        else if (0 == os_strcmp(szOutstat, "file"))
        {
            gpStatFile = file_open_fpostfix("stat");
        }
    }
}

/*!
*@brief        �����ṩ�����������ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:31
*@attention
* 
*/
GPublic void file_open_all()
{
    file_open_fdebug();
    file_open_fresult();
    file_open_fstat();
}

/*!
*@brief        �رմ�ŵ�����Ϣ���ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 16:30
*@attention
* 
*/
GPublic void file_close_fdebug()
{
    os_fflush(gpDebugFile);
    os_fclose(gpDebugFile);
}

/*!
*@brief        �رմ����Ͻ�����ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 16:33
*@attention
* 
*/
GPrivate void file_close_fresult()
{
    os_fflush(gpResultFile);
    os_fclose(gpResultFile);
}

/*!
*@brief        �رմ������ʱ״̬���ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 16:33
*@attention
* 
*/
GPrivate void file_close_fstat()
{
    os_fflush(gpStatFile);
    os_close(gpStatFile);
}

/*!
*@brief        �ⲿ�ӿڣ��ر������ļ�
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:31
*@attention
* 
*/
GPublic void file_close_all()
{
    file_close_fdebug();
    file_close_fresult();
    file_close_fstat();
}

/*!
*@brief        ��ȡ��¼������Ϣ���ļ����
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:31
*@attention
* 
*/
GPublic FILE* file_get_fdebug()
{
    return gpDebugFile;
}

/*!
*@brief        ��ȡ�����Ͻ�����ļ����
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:31
*@attention
* 
*/
GPublic FILE* file_get_fresult()
{
    return gpResultFile;
}

/*!
*@brief        ��ȡ���jstat��¼���ļ����
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:32
*@attention
* 
*/
GPublic FILE* file_get_fstat()
{
    return gpStatFile;
}
