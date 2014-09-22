/*!**************************************************************
 *@file rbtree.h
 *@brief    定义红黑树的对外接口
 *@author   zhaohm3
 *@date 2014-9-3 18:07
 *@note
 * 
 ****************************************************************/

#ifndef _rbtree_h__
#define _rbtree_h__

#include "share/share.h"

//! 红黑树可以存储任意数据类型
typedef Any_t RBData_t, *RBDataP_t;

//! 红黑树类型定义，对外仅仅提供类型名称，不提供类型的具体实现
typedef struct RBTree RBTree_t, *RBTreeP_t;

//! 创建一颗空的红黑树
GPublic RBTreeP_t rbtree_new();

//! 释放红黑树所占用的内存空间，包括红黑树所存放的数据所占用的空间
GPublic void rbtree_free(RBTreeP_t pTree);

//! 指定红黑树所存放数据类型的比较函数
GPublic RBTreeP_t rbtree_set_rbd_compare(RBTreeP_t pTree,
                                         Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t));

//! 指定红黑树所存放数据类型的内存空间释放函数
GPublic RBTreeP_t rbtree_set_rbd_free(RBTreeP_t pTree,
                                      void (*pfnFree)(RBDataP_t));

//! 指定红黑树节点所存储的数据的联合函数
GPublic RBTreeP_t rbtree_set_rbd_combiner(RBTreeP_t pTree,
                                          RBDataP_t (*pfnCombiner)(RBDataP_t, RBDataP_t));

//! 获取当前红黑树中所存放的数据类型的比较函数接口
GPublic Int32_t (*rbtree_get_rbd_compare(RBTreeP_t pTree))(RBDataP_t, RBDataP_t);

//! 获取当前红黑树中所存放的数据类型的内存空间释放接口
GPublic void (*rbtree_get_rbd_free(RBTreeP_t pTree))(RBDataP_t);

//! 获取红黑树节点所存储的数据的联合函数
GPublic RBDataP_t (*rbtree_get_rbd_combiner(RBTreeP_t pTree))(RBDataP_t, RBDataP_t);

//! 红黑树节点个数
GPublic Count32_t rbtree_node_count(RBTreeP_t pTree);

//! 红黑树是否为空
GPublic Bool32_t rbtree_is_empty(RBTreeP_t pTree);

//! 将pData插入红黑树
GPublic RBTreeP_t rbtree_insert(RBTreeP_t pTree, RBDataP_t pData, Bool32P_t pbExist);

//! 从红黑树中删除存储pData的节点
GPublic RBTreeP_t rbtree_delete(RBTreeP_t pTree, RBDataP_t pData);

//! 从红黑树中查找pData
GPublic RBDataP_t rbtree_search(RBTreeP_t pTree, RBDataP_t pData);

#endif