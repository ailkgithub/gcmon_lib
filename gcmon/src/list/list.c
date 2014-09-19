/*!**************************************************************
 *@file list.c
 *@brief    链表接口的实现
 *@author   zhaohm3
 *@date 2014-9-16 15:44
 *@note
 * 
 ****************************************************************/

#include "list/list.h"

/************************************************************************/
/*                             结构体定义                                 */
/************************************************************************/

typedef struct LNode LNode_t, *LNodeP_t;

//! 链表节点定义，节点可以存储任意类型，与此节点相关的接口不对外提供
struct LNode
{
    LDataP_t pData;                             //!< 节点存储的数据
    LNodeP_t pPrev;                             //!< 前一个节点
    LNodeP_t pNext;                             //!< 下一个节点
};

//! 双向链表定义，与链表相关的接口与，仅仅部分对外提供
struct List
{
    LNodeP_t pHead;                             //!< 链表头
    LNodeP_t pTail;                             //!< 链表尾
    Count32_t dwCount;                          //!< 链表存储的节点个数
    Int32_t(*pfnCompare)(LDataP_t, LDataP_t);   //!< 链表节点所存储的数据的比较函数
    void(*pfnFree)(LDataP_t);                   //!< 链表节点所存储的数据的空间释放函数
};

#define LNODE_SIZE sizeof(LNode_t)
#define LIST_SIZE sizeof(List_t)

/************************************************************************/
/*                          链表节点接口声明                               */
/************************************************************************/

GPrivate LNodeP_t lnode_new();
GPrivate void lnode_free(LNodeP_t pNode, void(*pfnFree)(LDataP_t));
GPrivate LNodeP_t lnode_clear(LNodeP_t pNode);
GPrivate LNodeP_t lnode_set(LNodeP_t pNode, LDataP_t pData, LNodeP_t pNext, LNodeP_t pPrev);

/************************************************************************/
/*                          双向链表接口声明                               */
/************************************************************************/

GPrivate ListP_t list_clear(ListP_t pList);
GPrivate LNodeP_t list_find_node(ListP_t pList, LDataP_t pData);
GPrivate ListP_t list_remove_node(ListP_t pList, LNodeP_t pNode);

/************************************************************************/
/*                          链表节点接口实现                               */
/************************************************************************/

/*!
*@brief        申请一个空节点
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-16 16:16
*@attention
* 
*/
GPrivate LNodeP_t lnode_new()
{
    LNodeP_t pNode = NULL;

    GMALLOC(pNode, LNode_t, LNODE_SIZE);
    GCMON_CHECK_NULL(pNode, ERROR);
    lnode_clear(pNode);

ERROR:
    return pNode;
}

/*!
*@brief        释放一个节点
*@author       zhaohm3
*@param[in]    pNode    需要释放的节点
*@param[in]    pfnFree  节点所存放数据的空间释放接口
*@retval
*@note
* 
*@since    2014-9-16 16:16
*@attention
* 
*/
GPrivate void lnode_free(LNodeP_t pNode, void(*pfnFree)(LDataP_t))
{
    GCMON_CHECK_NULL(pNode, ERROR);

    if (pfnFree != NULL && pNode->pData != NULL)
    {
        pfnFree(pNode->pData);
        pNode->pData = NULL;
    }

    GFREE(pNode);

ERROR:
    return;
}

/*!
*@brief        情况节点的所有域
*@author       zhaohm3
*@param[in]    pNode    待情况的节点
*@retval
*@note
* 
*@since    2014-9-16 16:17
*@attention
* 
*/
GPrivate LNodeP_t lnode_clear(LNodeP_t pNode)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    lnode_set(pNode, NULL, NULL, NULL);

ERROR:
    return pNode;
}

/*!
*@brief        设置节点的各个域
*@author       zhaohm3
*@param[in]    pNode    待设置的节点
*@param[in]    pData    数据
*@param[in]    pNext    后一个节点
*@param[in]    pPrev    前一个节点
*@retval
*@note
* 
*@since    2014-9-16 16:17
*@attention
* 
*/
GPrivate LNodeP_t lnode_set(LNodeP_t pNode, LDataP_t pData, LNodeP_t pNext, LNodeP_t pPrev)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pData = pData;
    pNode->pNext = pNext;
    pNode->pPrev = pPrev;

ERROR:
    return pNode;
}

/************************************************************************/
/*                          双向链表接口实现                               */
/************************************************************************/

/*!
*@brief        申请一个空的链表
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-16 16:23
*@attention
* 
*/
GPublic ListP_t list_new()
{
    ListP_t pList = NULL;

    GMALLOC(pList, List_t, LIST_SIZE);
    GCMON_CHECK_NULL(pList, ERROR);
    list_clear(pList);

ERROR:
    return pList;
}

/*!
*@brief        释放双向链表所占有的空间
*@author       zhaohm3
*@param[in]    pList    链表指针
*@retval
*@note
* 
*@since    2014-9-16 16:53
*@attention
* 
*/
GPublic void list_free(ListP_t pList)
{
    LNodeP_t pCurr = NULL;
    void(*pfnFree)(LDataP_t) = NULL;

    GCMON_CHECK_NULL(pList, ERROR);

    pCurr = pList->pHead;
    pfnFree = pList->pfnFree;

    while (pCurr != NULL)
    {
        LNodeP_t pNode = pCurr;

        pCurr = pCurr->pNext;
        lnode_free(pNode, pfnFree);
    }

    GFREE(pList);

ERROR:
    return;
}

/*!
*@brief        清空链表的各个域
*@author       zhaohm3
*@param[in]    pList    待情况的链表
*@retval
*@note
* 
*@since    2014-9-16 16:23
*@attention
* 
*/
GPrivate ListP_t list_clear(ListP_t pList)
{
    GCMON_CHECK_NULL(pList, ERROR);
    pList->pHead = NULL;
    pList->pTail = NULL;
    pList->dwCount = 0;
    pList->pfnCompare = NULL;
    pList->pfnFree = NULL;

ERROR:
    return pList;
}

/*!
*@brief        设置双向链表的pfnFree域
*@author       zhaohm3
*@param[in]    pList    待设置的双向链表
*@param[in]    pfnFree  链表节点所存储的数据的空间释放函数
*@retval
*@note
* 
*@since    2014-9-16 16:26
*@attention
* 
*/
GPublic ListP_t list_set_ldfree(ListP_t pList, void (*pfnFree)(LDataP_t))
{
    GCMON_CHECK_NULL(pList, ERROR);
    pList->pfnFree = pfnFree;

ERROR:
    return pList;
}

/*!
*@brief        获取双向链表的pfnFree域
*@author       zhaohm3
*@param[in]    pList
*@retval
*@note
* 
*@since    2014-9-16 16:30
*@attention
* 
*/
GPublic void (*list_get_ldfree(ListP_t pList))(LDataP_t)
{
    void (*pfnFree)(LDataP_t) = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    pfnFree = pList->pfnFree;

ERROR:
    return pfnFree;
}

/*!
*@brief        设置链表节点所存储的数据的比较函数
*@author       zhaohm3
*@param[in]    pList    待设置的双向链表
*@param[in]    pfnCompare   链表节点所存储的数据的比较函数
*@retval
*@note
* 
*@since    2014-9-16 16:39
*@attention
* 
*/
GPublic ListP_t list_set_ldcompare(ListP_t pList, Int32_t(*pfnCompare)(LDataP_t, LDataP_t))
{
    GCMON_CHECK_NULL(pList, ERROR);
    pList->pfnCompare = pfnCompare;

ERROR:
    return pList;
}

/*!
*@brief        获取链表节点所存储的数据的比较函数
*@author       zhaohm3
*@param[in]    pList
*@retval
*@note
* 
*@since    2014-9-16 16:39
*@attention
* 
*/
GPublic Int32_t (*list_get_ldcompare(ListP_t pList))(LDataP_t, LDataP_t)
{
    Int32_t(*pfnCompare)(LDataP_t, LDataP_t) = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    pfnCompare = pList->pfnCompare;

ERROR:
    return pfnCompare;
}

/*!
*@brief        将pData添加到链表尾
*@author       zhaohm3
*@param[in]    pList    待添加的链表，必须为非空
*@param[in]    pData    待添加的数据
*@retval
*@note
* 
*@since    2014-9-16 17:09
*@attention    由于链表可存放任意数值，因此pData可以为空
* 
*/
GPublic ListP_t list_append(ListP_t pList, LDataP_t pData)
{
    LNodeP_t pNode = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    pNode = lnode_new();
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pData = pData;

    if (0 == pList->dwCount)
    {
        GASSERT(NULL == pList->pHead && NULL == pList->pTail);
        pList->pHead = pList->pTail = pNode;
    }
    else
    {
        GASSERT(1 == pList->dwCount ? pList->pHead == pList->pTail : pList->pHead != pList->pTail);
        GASSERT(pList->pHead != NULL && pList->pTail != NULL);
        pList->pTail->pNext = pNode;
        pNode->pPrev = pList->pTail;
        pList->pTail = pNode;
    }

    pList->dwCount++;

ERROR:
    return pList;
}

/*!
*@brief        将pData添加到链表头
*@author       zhaohm3
*@param[in]    pList    待添加的链表，必须为非空
*@param[in]    pData    待添加的数据
*@retval
*@note
* 
*@since    2014-9-16 17:09
*@attention    由于链表可存放任意数值，因此pData可以为空
* 
*/
GPublic ListP_t list_cons(ListP_t pList, LDataP_t pData)
{
    LNodeP_t pNode = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    pNode = lnode_new();
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pData = pData;

    if (0 == pList->dwCount)
    {
        GASSERT(NULL == pList->pHead && NULL == pList->pTail);
        pList->pHead = pList->pTail = pNode;
    }
    else
    {
        GASSERT(1 == pList->dwCount ? pList->pHead == pList->pTail : pList->pHead != pList->pTail);
        GASSERT(pList->pHead != NULL && pList->pTail != NULL);
        pList->pHead->pPrev = pNode;
        pNode->pNext = pList->pHead;
        pList->pHead = pNode;
    }

    pList->dwCount++;

ERROR:
    return pList;
}

/*!
*@brief        在链表中查找存储和pData相等的节点
*@author       zhaohm3
*@param[in]    pList
*@param[in]    pData
*@retval
*@note
* 
*@since    2014-9-16 17:10
*@attention
* 
*/
GPrivate LNodeP_t list_find_node(ListP_t pList, LDataP_t pData)
{
    LNodeP_t pNode = NULL;
    Int32_t (*pfnCompare)(LDataP_t, LDataP_t) = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    GCMON_CHECK_NULL(pList->pfnCompare, ERROR);

    pNode = pList->pHead;
    pfnCompare = pList->pfnCompare;

    while (pNode != NULL)
    {
        if (0 == pfnCompare(pData, pNode->pData))
        {
            break;
        }

        pNode = pNode->pNext;
    }

ERROR:
    return pNode;
}

/*!
*@brief        在链表中查找存储和pData相等的节点所存储的数据
*@author       zhaohm3
*@param[in]    pList    待查找的链表
*@param[in]    pData    待查找的数据
*@param[out]   pbExist  输出参数，表示pData是否在pList中存在
*@retval
*@note         由于链表可以存放空值，因此当传入的pData为空时，返回值肯定为空，此时需要用pbExist的值
*              指示，为空的pData是否在pList中存在
* 
*@since    2014-9-16 18:05
*@attention
* 
*/
GPublic LDataP_t list_find_data(ListP_t pList, LDataP_t pData, Bool32P_t pbExist)
{
    LNodeP_t pNode = NULL;
    LDataP_t pRet = NULL;
    Bool32_t bExist = FALSE;

    GCMON_CHECK_NULL(pList, ERROR);
    pNode = list_find_node(pList, pData);
    GCMON_CHECK_NULL(pNode, ERROR);
    pRet = pNode->pData;
    bExist = TRUE;

ERROR:

    if (pbExist != NULL)
    {
        *pbExist = bExist;
    }

    return pRet;
}

/*!
*@brief        从链表中移除指定的节点
*@author       zhaohm3
*@param[in]    pList    待进行移除操作的链表
*@param[in]    pNode    待移除的节点
*@retval
*@note
* 
*@since    2014-9-16 18:13
*@attention
* 
*/
GPrivate ListP_t list_remove_node(ListP_t pList, LNodeP_t pNode)
{
    GCMON_CHECK_NULL(pList, ERROR);
    GCMON_CHECK_NULL(pNode, ERROR);
    GCMON_CHECK_CONDITION(pList->dwCount != 0, ERROR);

    if (1 == pList->dwCount)
    {
        GASSERT(pList->pHead == pList->pTail);
        GASSERT(pList->pHead == pNode);

        pList->pHead = pList->pTail = NULL;
    }
    else
    {
        LNodeP_t pPrev = pNode->pPrev;
        LNodeP_t pNext = pNode->pNext;

        if (pPrev != NULL)
        {
            pPrev->pNext = pNode->pNext;
        }
        else
        {
            pList->pHead = pNode->pNext;
        }

        if (pNext != NULL)
        {
            pNext->pPrev = pPrev;
        }
        else
        {
            pList->pTail = pPrev;
        }
    }

    pNode->pNext = pNode->pPrev = NULL;
    lnode_free(pNode, pList->pfnFree);
    pList->dwCount--;

ERROR:
    return pList;
}

/*!
*@brief        移除链表中存储和pData相等的节点
*@author       zhaohm3
*@param[in]    pList    待进行移除操作的链表
*@param[in]    pData    待移除的数据
*@retval
*@note
* 
*@since    2014-9-16 17:11
*@attention
* 
*/
GPublic ListP_t list_remove(ListP_t pList, LDataP_t pData)
{
    LNodeP_t pNode = NULL;

    GCMON_CHECK_NULL(pList, ERROR);
    pNode = list_find_node(pList, pData);
    GCMON_CHECK_NULL(pNode, ERROR);
    list_remove_node(pList, pNode);

ERROR:
    return pList;
}
