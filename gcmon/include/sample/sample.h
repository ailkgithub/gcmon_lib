/*!**************************************************************
 *@file sample.h
 *@brief    ��PerfMemory��ָ����������ݲ����Ķ���ӿ�����
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