/*!**************************************************************
 *@file rbtree.h
 *@brief    ���������Ķ���ӿ�
 *@author   zhaohm3
 *@date 2014-9-3 18:07
 *@note
 * 
 ****************************************************************/

#ifndef _rbtree_h__
#define _rbtree_h__

#include "share/share.h"

//! ��������Դ洢������������
typedef Any_t RBData_t, *RBDataP_t;

//! ��������Ͷ��壬��������ṩ�������ƣ����ṩ���͵ľ���ʵ��
typedef struct RBTree RBTree_t, *RBTreeP_t;

//! ����һ�ſյĺ����
GPublic RBTreeP_t rbtree_new();

//! �ͷź������ռ�õ��ڴ�ռ䣬�������������ŵ�������ռ�õĿռ�
GPublic void rbtree_free(RBTreeP_t pTree);

//! ָ�������������������͵ıȽϺ���
GPublic RBTreeP_t rbtree_set_rbd_compare(RBTreeP_t pTree,
                                         Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t));

//! ָ�������������������͵��ڴ�ռ��ͷź���
GPublic RBTreeP_t rbtree_set_rbd_free(RBTreeP_t pTree,
                                      void (*pfnFree)(RBDataP_t));

//! ָ��������ڵ����洢�����ݵ����Ϻ���
GPublic RBTreeP_t rbtree_set_rbd_combiner(RBTreeP_t pTree,
                                          RBDataP_t (*pfnCombiner)(RBDataP_t, RBDataP_t));

//! ��ȡ��ǰ�����������ŵ��������͵ıȽϺ����ӿ�
GPublic Int32_t (*rbtree_get_rbd_compare(RBTreeP_t pTree))(RBDataP_t, RBDataP_t);

//! ��ȡ��ǰ�����������ŵ��������͵��ڴ�ռ��ͷŽӿ�
GPublic void (*rbtree_get_rbd_free(RBTreeP_t pTree))(RBDataP_t);

//! ��ȡ������ڵ����洢�����ݵ����Ϻ���
GPublic RBDataP_t (*rbtree_get_rbd_combiner(RBTreeP_t pTree))(RBDataP_t, RBDataP_t);

//! ������ڵ����
GPublic Count32_t rbtree_node_count(RBTreeP_t pTree);

//! ������Ƿ�Ϊ��
GPublic Bool32_t rbtree_is_empty(RBTreeP_t pTree);

//! ��pData��������
GPublic RBTreeP_t rbtree_insert(RBTreeP_t pTree, RBDataP_t pData, Bool32P_t pbExist);

//! �Ӻ������ɾ���洢pData�Ľڵ�
GPublic RBTreeP_t rbtree_delete(RBTreeP_t pTree, RBDataP_t pData);

//! �Ӻ�����в���pData
GPublic RBDataP_t rbtree_search(RBTreeP_t pTree, RBDataP_t pData);

#endif