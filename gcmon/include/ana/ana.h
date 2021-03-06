/*!**************************************************************
 *@file ana.h
 *@brief    JVM性能自动分析模块接口声明
 *@author   zhaohm3
 *@date 2014-9-22 17:29
 *@note
 * 
 ****************************************************************/

#ifndef _ana_h__
#define _ana_h__

#include "rbtree/rbtree.h"

GPublic void ana_OOM(RBTreeP_t pPdiTree, Int32_t sdwOOMType);

#endif