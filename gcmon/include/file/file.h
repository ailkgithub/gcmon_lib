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

GPublic void file_open_fdebug();
GPublic void file_open_fresult();
GPublic void file_open_fstat();
GPublic void file_open_all();

GPublic void file_close_fdebug();
GPublic void file_close_fresult();
GPublic void file_close_fstat();
GPublic void file_close_all();

GPublic FILE* file_get_fdebug();
GPublic FILE* file_get_fresult();
GPublic FILE* file_get_fstat();

GPublic String_t file_get_fdname();
GPublic String_t file_get_frname();
GPublic String_t file_get_fsname();

GPublic void file_remove(String_t szFileName);

#endif