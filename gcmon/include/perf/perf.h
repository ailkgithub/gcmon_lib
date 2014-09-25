/*!**************************************************************
 *@file perf.h
 *@brief    ����Hotspot PerfMemory
 *@author   zhaohm3
 *@date 2014-9-11 15:03
 *@note     ��Ҫ���ڴ���Hotspot�����ܼ���ڴ���
 * 
 ****************************************************************/

#ifndef _perf_h__
#define _perf_h__

#include "rbtree/rbtree.h"

typedef struct PerfDataItem PerfDataItem_t, *PerfDataItemP_t;

GPublic RBTreeP_t pdi_build_tree(Addr_t pPerfMemory);
GPublic PerfDataItemP_t pdi_search_item(RBTreeP_t pTree, String_t szName);
GPublic jlong pdi_get_jlong(PerfDataItemP_t pItem);
GPublic Double_t pdi_get_double(PerfDataItemP_t pItem);
GPublic String_t pdi_get_string(PerfDataItemP_t pItem);
GPublic Addr_t pdi_get_other(PerfDataItemP_t pItem);

#endif