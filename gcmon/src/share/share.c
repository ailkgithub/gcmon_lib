/*!**************************************************************
 *@file share.cpp
 *@brief    共享接口定义
 *@author   zhaohm3
 *@date 2014-9-3 18:23
 *@note
 * 
 ****************************************************************/

#include "share/share.h"

//! 记录调试信息
GPrivate FILE *gFile = NULL;

//! 打开gFile
GPublic void gcmon_debug_fopen()
{
#ifdef DEBUG
    if (NULL == gFile)
    {
        gFile = fopen("gcmon_debug.txt", "w+");
    }
#endif
}

//! 关闭gFile
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

//! flush文件缓存
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
 *@brief        打印gcmon运行过程中的调试信息
 *@author       zhaohm3
 *@param[in]    fmt
 *@retval       
 *@note         可以通过更改此接口，将调试信息打印到指定的文件中
 * 
 *@since    2014-9-3 18:23
 *@attention    此接口在Debug版本中生效
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
