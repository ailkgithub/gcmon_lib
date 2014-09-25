/*!**************************************************************
 *@file share.c
 *@brief    共享接口定义
 *@author   zhaohm3
 *@date 2014-9-3 18:23
 *@note
 * 
 ****************************************************************/

#include "share/share.h"
#include "file/file.h"
#include "os/os.h"

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
    va_list args;
    FILE *file = file_get_fdebug();

    if (NULL == file)
    {
        file = stdout;
    }

    va_start(args, fmt);
    len = os_vfprintf(file, fmt, args);
    va_end(args);

    return len;
#else
    return 0;
#endif
}
