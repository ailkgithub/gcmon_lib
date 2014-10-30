/*!**************************************************************
 *@file file.c
 *@brief    与文件管理相关的接口实现
 *@author   zhaohm3
 *@date 2014-9-25 12:54
 *@note
 * 
 ****************************************************************/

#include "file/file.h"
#include "os/os.h"
#include "args/args.h"

GPrivate FILE *gpDebugFile = NULL;          //!< 记录调试信息，仅在DEBUG模式下有效
GPrivate FILE *gpResultFile = NULL;         //!< 存放gcmon诊断结果的文件，由outname选项指定
GPrivate FILE *gpStatFile = NULL;           //!< 存放jstat数据的文件，由outname和outstat的值共同决定

GPrivate Char_t gszDFileName[256] = { 0 };  //!< 记录调试信息文件的文件名，仅在DEBUG模式下有效
GPrivate Char_t gszRFileName[256] = { 0 };  //!< 存放gcmon诊断结果文件的文件名，由outname选项指定
GPrivate Char_t gszSFileName[256] = { 0 };  //!< 存放jstat数据文件的文明名，由outname和outstat的值共同决定

/*!
*@brief        通过outpath、outname和提供的构造后缀名称创建文件
*@author       zhaohm3
*@param[in]    szPostfix    文件后缀名称
*@param[out]   szFileName   文件名，用于输出
*@retval       创建$(outpath)/$(outname)_pid_$(pid).$(szPostfix)文件
*@note
* 
*@since    2014-9-25 14:27
*@attention
* 
*/
GPrivate FILE* file_open_fpostfix(String_t szPostfix, String_t szFileName)
{
    FILE *pFile = NULL;
    String_t szOutpath = agentargs_get_outpath();
    String_t szOutname = agentargs_get_outname();

    os_sprintf(szFileName, "%s%s%s_pid_%d.%s",
        (szOutpath != NULL) ? szOutpath : "",
        (szOutpath != NULL) ? "/" : "",
        (szOutname != NULL) ? szOutname : "gcmon",
        os_getpid(),
        (szPostfix != NULL) ? szPostfix : "unknown");

    pFile = os_fopen(szFileName, "w+");

    if (NULL == pFile)
    {
        szFileName[0] = '\0';
    }

    return pFile;
}

/*!
*@brief        创建调试信息的记录文件
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
    gpDebugFile = file_open_fpostfix("debug", gszDFileName);
#endif
}

/*!
*@brief        创建存放诊断结果的文件
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:30
*@attention
* 
*/
GPublic void file_open_fresult()
{
    gpResultFile = file_open_fpostfix("result", gszRFileName);
}

/*!
*@brief        创建存放运行时状态的文件
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 14:30
*@attention
* 
*/
GPublic void file_open_fstat()
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
            gpStatFile = file_open_fpostfix("stat", gszSFileName);
        }
    }
}

/*!
*@brief        对外提供，创建所有文件
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
*@brief        关闭存放调试信息的文件
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
    gpDebugFile = NULL;
}

/*!
*@brief        关闭存放诊断结果的文件
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 16:33
*@attention
* 
*/
GPublic void file_close_fresult()
{
    os_fflush(gpResultFile);
    os_fclose(gpResultFile);
    gpResultFile = NULL;
}

/*!
*@brief        关闭存放运行时状态的文件
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-25 16:33
*@attention
* 
*/
GPublic void file_close_fstat()
{
    os_fflush(gpStatFile);
    os_fclose(gpStatFile);
    gpStatFile = NULL;
}

/*!
*@brief        外部接口，关闭所有文件
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
*@brief        获取记录调试信息的文件句柄
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
*@brief        获取存放诊断结果的文件句柄
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
*@brief        获取存放jstat记录的文件句柄
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

GPublic String_t file_get_fdname()
{
    String_t szFileName = NULL;

    if (gszDFileName[0] != '\0')
    {
        szFileName = gszDFileName;
    }

    return szFileName;
}

GPublic String_t file_get_frname()
{
    String_t szFileName = NULL;

    if (gszRFileName[0] != '\0')
    {
        szFileName = gszRFileName;
    }

    return szFileName;
}

GPublic String_t file_get_fsname()
{
    String_t szFileName = NULL;

    if (gszSFileName[0] != '\0')
    {
        szFileName = gszSFileName;
    }

    return szFileName;
}

GPublic void file_remove(String_t szFileName)
{
    if (szFileName != NULL)
    {
        os_unlink(szFileName);
    }
}