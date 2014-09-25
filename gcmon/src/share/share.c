/*!**************************************************************
 *@file share.c
 *@brief    ����ӿڶ���
 *@author   zhaohm3
 *@date 2014-9-3 18:23
 *@note
 * 
 ****************************************************************/

#include "share/share.h"
#include "file/file.h"
#include "os/os.h"

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
