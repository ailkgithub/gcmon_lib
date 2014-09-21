/*!**************************************************************
 *@file sample.h
 *@brief    对PerfMemory的指定项进行数据采样的对外接口声明
 *@author   zhaohm3
 *@date 2014-9-20 20:12
 *@note
 * 
 ****************************************************************/

#ifndef _sample_h__
#define _sample_h__

#include "share/share.h"
#include "rbtree/rbtree.h"

GPublic void sample_init(RBTreeP_t pTree);
GPublic void sample_doit(String_t szContext);

#endif