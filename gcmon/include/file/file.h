/*!**************************************************************
 *@file file.h
 *@brief    与文件管理相关的接口声明
 *@author   zhaohm3
 *@date 2014-9-25 13:25
 *@note
 * 
 ****************************************************************/

#ifndef _file_h__
#define _file_h__

#include "share/share.h"

GPublic void file_open_all();
GPublic void file_close_all();
GPublic FILE* file_get_fdebug();
GPublic FILE* file_get_fresult();
GPublic FILE* file_get_fstat();

#endif