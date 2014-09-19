/*!**************************************************************
 *@file list.c
 *@brief    ����ӿڵ�ʵ��
 *@author   zhaohm3
 *@date 2014-9-16 15:44
 *@note
 * 
 ****************************************************************/

#include "list/list.h"

/************************************************************************/
/*                             �ṹ�嶨��                                 */
/************************************************************************/

typedef struct LNode LNode_t, *LNodeP_t;

//! ����ڵ㶨�壬�ڵ���Դ洢�������ͣ���˽ڵ���صĽӿڲ������ṩ
struct LNode
{
    LDataP_t pData;                             //!< �ڵ�洢������
    LNodeP_t pPrev;                             //!< ǰһ���ڵ�
    LNodeP_t pNext;                             //!< ��һ���ڵ�
};

//! ˫�������壬��������صĽӿ��룬�������ֶ����ṩ
struct List
{
    LNodeP_t pHead;                             //!< ����ͷ
    LNodeP_t pTail;                             //!< ����β
    Count32_t dwCount;                          //!< ����洢�Ľڵ����
    Int32_t(*pfnCompare)(LDataP_t, LDataP_t);   //!< ����ڵ����洢�����ݵıȽϺ���
    void(*pfnFree)(LDataP_t);                   //!< ����ڵ����洢�����ݵĿռ��ͷź���
};

#define LNODE_SIZE sizeof(LNode_t)
#define LIST_SIZE sizeof(List_t)

/************************************************************************/
/*                          ����ڵ�ӿ�����                               */
/************************************************************************/

GPrivate LNodeP_t lnode_new();
GPrivate void lnode_free(LNodeP_t pNode, void(*pfnFree)(LDataP_t));
GPrivate LNodeP_t lnode_clear(LNodeP_t pNode);
GPrivate LNodeP_t lnode_set(LNodeP_t pNode, LDataP_t pData, LNodeP_t pNext, LNodeP_t pPrev);

/************************************************************************/
/*                          ˫������ӿ�����                               */
/************************************************************************/

GPrivate ListP_t list_clear(ListP_t pList);
GPrivate LNodeP_t list_find_node(ListP_t pList, LDataP_t pData);
GPrivate ListP_t list_remove_node(ListP_t pList, LNodeP_t pNode);

/************************************************************************/
/*                          ����ڵ�ӿ�ʵ��                               */
/************************************************************************/

/*!
*@brief        ����һ���սڵ�
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
*@brief        �ͷ�һ���ڵ�
*@author       zhaohm3
*@param[in]    pNode    ��Ҫ�ͷŵĽڵ�
*@param[in]    pfnFree  �ڵ���������ݵĿռ��ͷŽӿ�
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
*@brief        ����ڵ��������
*@author       zhaohm3
*@param[in]    pNode    ������Ľڵ�
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
*@brief        ���ýڵ�ĸ�����
*@author       zhaohm3
*@param[in]    pNode    �����õĽڵ�
*@param[in]    pData    ����
*@param[in]    pNext    ��һ���ڵ�
*@param[in]    pPrev    ǰһ���ڵ�
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
/*                          ˫������ӿ�ʵ��                               */
/************************************************************************/

/*!
*@brief        ����һ���յ�����
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
*@brief        �ͷ�˫��������ռ�еĿռ�
*@author       zhaohm3
*@param[in]    pList    ����ָ��
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
*@brief        �������ĸ�����
*@author       zhaohm3
*@param[in]    pList    �����������
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
*@brief        ����˫�������pfnFree��
*@author       zhaohm3
*@param[in]    pList    �����õ�˫������
*@param[in]    pfnFree  ����ڵ����洢�����ݵĿռ��ͷź���
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
*@brief        ��ȡ˫�������pfnFree��
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
*@brief        ��������ڵ����洢�����ݵıȽϺ���
*@author       zhaohm3
*@param[in]    pList    �����õ�˫������
*@param[in]    pfnCompare   ����ڵ����洢�����ݵıȽϺ���
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
*@brief        ��ȡ����ڵ����洢�����ݵıȽϺ���
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
*@brief        ��pData��ӵ�����β
*@author       zhaohm3
*@param[in]    pList    ����ӵ���������Ϊ�ǿ�
*@param[in]    pData    ����ӵ�����
*@retval
*@note
* 
*@since    2014-9-16 17:09
*@attention    ��������ɴ��������ֵ�����pData����Ϊ��
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
*@brief        ��pData��ӵ�����ͷ
*@author       zhaohm3
*@param[in]    pList    ����ӵ���������Ϊ�ǿ�
*@param[in]    pData    ����ӵ�����
*@retval
*@note
* 
*@since    2014-9-16 17:09
*@attention    ��������ɴ��������ֵ�����pData����Ϊ��
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
*@brief        �������в��Ҵ洢��pData��ȵĽڵ�
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
*@brief        �������в��Ҵ洢��pData��ȵĽڵ����洢������
*@author       zhaohm3
*@param[in]    pList    �����ҵ�����
*@param[in]    pData    �����ҵ�����
*@param[out]   pbExist  �����������ʾpData�Ƿ���pList�д���
*@retval
*@note         ����������Դ�ſ�ֵ����˵������pDataΪ��ʱ������ֵ�϶�Ϊ�գ���ʱ��Ҫ��pbExist��ֵ
*              ָʾ��Ϊ�յ�pData�Ƿ���pList�д���
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
*@brief        ���������Ƴ�ָ���Ľڵ�
*@author       zhaohm3
*@param[in]    pList    �������Ƴ�����������
*@param[in]    pNode    ���Ƴ��Ľڵ�
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
*@brief        �Ƴ������д洢��pData��ȵĽڵ�
*@author       zhaohm3
*@param[in]    pList    �������Ƴ�����������
*@param[in]    pData    ���Ƴ�������
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
