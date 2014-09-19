/*!**************************************************************
 *@file list.h
 *@brief    定义双向链表的对外接口
 *@author   zhaohm3
 *@date 2014-9-16 9:31
 *@note
 * 
 ****************************************************************/

#ifndef _list_h__
#define _list_h__

#include "share/share.h"

typedef Any_t LData_t, *LDataP_t;
typedef struct List List_t, *ListP_t;

GPublic ListP_t list_new();
GPublic void list_free(ListP_t pList);
GPublic ListP_t list_set_ldfree(ListP_t pList, void(*pfnFree)(LDataP_t));
GPublic void (*list_get_ldfree(ListP_t pList))(LDataP_t);
GPublic ListP_t list_set_ldcompare(ListP_t pList, Int32_t(*pfnCompare)(LDataP_t, LDataP_t));
GPublic Int32_t (*list_get_ldcompare(ListP_t pList))(LDataP_t, LDataP_t);

GPublic ListP_t list_append(ListP_t pList, LDataP_t pData);
GPublic ListP_t list_cons(ListP_t pList, LDataP_t pData);
GPublic ListP_t list_remove(ListP_t pList, LDataP_t pData);

GPublic LDataP_t list_find_data(ListP_t pList, LDataP_t pData, Bool32P_t pbExist);

#endif
