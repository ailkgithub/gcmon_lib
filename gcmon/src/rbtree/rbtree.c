/*!**************************************************************
 *@file rbtree.cpp
 *@brief    �������ʵ��
 *@author   zhaohm3
 *@date 2014-9-3 18:27
 *@note
 * 
 ****************************************************************/


#include "rbtree/rbtree.h"


/************************************************************************/
/*                             �ṹ�嶨��                                 */
/************************************************************************/

GTYPES(RBNode);
GTYPEE(RBColor);

//! ������ڵ���ɫö����
enum RBColor
{
    RBC_NONE = 0,                                   //!< ��ʼ��Ĭ��ɫ
    RBC_RED,                                        //!< ��ɫ
    RBC_BLACK                                       //!< ��ɫ
};

//! ������ڵ㶨�壬�˶��岻�����ṩ���������صĽӿ��಻�����ṩ
struct RBNode
{
    RBColor_t mColor;                               //!< ��ǰ�ڵ���ɫ
    RBDataP_t pData;                                //!< ��ǰ�ڵ�洢�����ݣ�����ʹ�������͵�ָ��
    RBNodeP_t pParent;                              //!< ��ǰ�ڵ�ĸ��ڵ�
    RBNodeP_t pLeft;                                //!< ��ǰ�ڵ�����ӽڵ�
    RBNodeP_t pRight;                               //!< ��ǰ�ڵ�����ӽڵ�
};

//! ��������壬�˶��岻�����ṩ�����������صĽӿڽ��ṩ���ֱ����
struct RBTree
{
    RBNodeP_t pRoot;                                //!< ������ĸ��ڵ�
    Count32_t dwCount;                              //!< ������ڵ����
    Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t);    //!< ������ڵ����洢�����ݵıȽϺ���
    void (*pfnFree)(RBDataP_t);                     //!< ������ڵ����洢�����ݵĿռ��ͷź���
    RBDataP_t(*pfnCombiner)(RBDataP_t, RBDataP_t);  //!< ������ڵ����洢�����ݵ����Ӻ���
};

#define RBNODE_SIZE sizeof(RBNode_t)
#define RBTREE_SIZE sizeof(RBTree_t)

#define RBNIL ((RBNodeP_t)&gRBNIL)
GPrivate RBNode_t gRBNIL = { RBC_BLACK, NULL, NULL, RBNIL, RBNIL };

/************************************************************************/
/*                         ������ڵ�ӿ�����                               */
/************************************************************************/

GPrivate RBNodeP_t rbnode_new();
GPrivate void rbnode_free(RBNodeP_t pNode, void(*pfnFree)(RBDataP_t));
GPrivate void rbnode_free_tree(RBNodeP_t pRoot, void(*pfnFree)(RBDataP_t));

GPrivate RBNodeP_t rbnode_clear(RBNodeP_t pNode);
GPrivate RBNodeP_t rbnode_set_color(RBNodeP_t pNode, RBColor_t color);
GPrivate RBNodeP_t rbnode_set_data(RBNodeP_t pNode, RBDataP_t pData);
GPrivate RBNodeP_t rbnode_set_parent(RBNodeP_t pNode, RBNodeP_t pParent);
GPrivate RBNodeP_t rbnode_set_left(RBNodeP_t pNode, RBNodeP_t pLeft);
GPrivate RBNodeP_t rbnode_set_right(RBNodeP_t pNode, RBNodeP_t pRight);
GPrivate RBNodeP_t rbnode_set(RBNodeP_t pNode,
                              RBColor_t color,
                              RBDataP_t pData,
                              RBNodeP_t pParent,
                              RBNodeP_t pLeft,
                              RBNodeP_t pRight);

GPrivate RBColor_t rbnode_get_color(RBNodeP_t pNode);
GPrivate RBDataP_t rbnode_get_data(RBNodeP_t pNode);
GPrivate RBNodeP_t rbnode_get_parent(RBNodeP_t pNode);
GPrivate RBNodeP_t rbnode_get_left(RBNodeP_t pNode);
GPrivate RBNodeP_t rbnode_get_right(RBNodeP_t pNode);

/************************************************************************/
/*                            ������ӿ�����                               */
/************************************************************************/

GPrivate RBNodeP_t rbtree_get_root(RBTreeP_t pTree);
GPrivate RBTreeP_t rbtree_set_root(RBTreeP_t pTree, RBNodeP_t pRoot);

GPrivate RBTreeP_t rbtree_rotate_left(RBTreeP_t pTree, RBNodeP_t pSubRoot);
GPrivate RBTreeP_t rbtree_roatate_right(RBTreeP_t pTree, RBNodeP_t pSubRoot);
GPrivate RBTreeP_t rbtree_insert_fixup(RBTreeP_t pTree, RBNodeP_t pSubRoot);
GPrivate RBTreeP_t rbtree_delete_fixup(RBTreeP_t pTree, RBNodeP_t pSubRoot);
GPrivate RBTreeP_t rbtree_delete_node(RBTreeP_t pTree, RBNodeP_t pNode);
GPrivate RBNodeP_t rbtree_search_node(RBTreeP_t pTree, RBDataP_t pData);

/************************************************************************/
/*                          ������ڵ�ӿ�ʵ��                              */
/************************************************************************/

/*!
*@brief        ����һ���սڵ㣬�ڵ���ɫ��ʼ��ΪRBC_NONE
*@author       zhaohm3
*@retval       ��������ɹ��Ľڵ�ָ��
*@note
* 
*@since    2014-9-5 10:46
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_new()
{
    RBNodeP_t pNode = NULL;

    GMALLOC(pNode, RBNode_t, RBNODE_SIZE);
    GCMON_CHECK_NULL(pNode, ERROR);

    //! �´����Ľڵ�Ĭ��Ϊ��ɫ����������ΪRBNIL�����ڵ�ΪNULL
    rbnode_set(pNode, RBC_BLACK, NULL, NULL, RBNIL, RBNIL);

    return pNode;

ERROR:
    GFREE(pNode);
    return NULL;
}

/*!
*@brief        �ͷ�һ��������ڵ㣬�ڵ����洢�����ݿռ���pfnFree�����ͷ�
*@author       zhaohm3
*@param[in]    pNode    ���ͷŵĺ�����ڵ�ָ��
*@param[in]    pfnFree  �����ͷŽڵ�洢�����ݵĺ���ָ��
*@retval
*@note         ������ڵ����洢�����ݿ������������ͣ�����ͷŸ����͵Ľӿ���Ҫ�û��ڴ����������ʱ��ָ��
* 
*@since    2014-9-5 10:47
*@attention
* 
*/
GPrivate void rbnode_free(RBNodeP_t pNode, void (*pfnFree)(RBDataP_t))
{
    RBDataP_t pData = NULL;

    GCMON_CHECK_NULL(pNode, ERROR);
    GCMON_CHECK_COND(pNode != RBNIL, ERROR);

    pData = pNode->pData;

    //! ���û��Զ���ӿ��ͷŽڵ�洢������
    if (pfnFree != NULL && pData != NULL)
    {
        pfnFree(pData);
    }

    rbnode_clear(pNode);
    GFREE(pNode);

ERROR:
    return;
}

/*!
*@brief        �ݹ��ͷ�pRootΪ���ڵ�Ķ�������ռ�õĿռ�
*@author       zhaohm3
*@param[in]    pRoot    ���ڵ�
*@param[in]    pfnFree  �ڵ�洢���ݵ��ͷŽӿ�ָ��
*@retval
*@note         ����ÿ���ڵ����洢������ռ�õĿռ���pfnFree�ͷ�
* 
*@since    2014-9-5 10:53
*@attention
* 
*/
GPrivate void rbnode_free_tree(RBNodeP_t pRoot, void (*pfnFree)(RBDataP_t))
{
    RBNodeP_t pLeft = NULL;
    RBNodeP_t pRight = NULL;

    GCMON_CHECK_NULL(pRoot, ERROR);
    GCMON_CHECK_COND(pRoot != RBNIL, ERROR);

    pLeft = pRoot->pLeft;
    pRight = pRoot->pRight;

    rbnode_free(pRoot, pfnFree);                   //!< �ͷŸ��ڵ���ռ�õĿռ�
    rbnode_free_tree(pLeft, pfnFree);       //!< �ݹ��ͷ���������ռ�õĿռ�
    rbnode_free_tree(pRight, pfnFree);      //!< �ݹ��ͷ���������ռ�õĿռ�

ERROR:
    return;
}

/*!
*@brief        ���һ���������ڵ�
*@author       zhaohm3
*@param[in]    pNode    ����յĽڵ�
*@retval
*@note
* 
*@since    2014-9-5 10:55
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_clear(RBNodeP_t pNode)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    rbnode_set(pNode, RBC_NONE, NULL, NULL, NULL, NULL);

ERROR:
    return pNode;
}

/*!
*@brief        ���ýڵ���ɫ
*@author       zhaohm3
*@param[in]    pNode
*@param[in]    color
*@retval
*@note
* 
*@since    2014-9-5 10:58
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set_color(RBNodeP_t pNode, RBColor_t color)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->mColor = color;

ERROR:
    return pNode;
}

/*!
*@brief        ���ýڵ����洢������
*@author       zhaohm3
*@param[in]    pNode
*@param[in]    pData
*@retval
*@note
* 
*@since    2014-9-5 10:58
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set_data(RBNodeP_t pNode, RBDataP_t pData)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pData = pData;

ERROR:
    return pNode;
}

/*!
*@brief        ���ø��ڵ�
*@author       zhaohm3
*@param[in]    pNode
*@param[in]    pParent
*@retval
*@note
* 
*@since    2014-9-5 10:58
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set_parent(RBNodeP_t pNode, RBNodeP_t pParent)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pParent = pParent;

ERROR:
    return pNode;
}

/*!
*@brief        �������ӽڵ�
*@author       zhaohm3
*@param[in]    pNode
*@param[in]    pLeft
*@retval
*@note
* 
*@since    2014-9-5 10:59
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set_left(RBNodeP_t pNode, RBNodeP_t pLeft)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pLeft = pLeft;

ERROR:
    return pNode;
}

/*!
*@brief        �������ӽڵ�
*@author       zhaohm3
*@param[in]    pNode
*@param[in]    pRight
*@retval
*@note
* 
*@since    2014-9-5 10:59
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set_right(RBNodeP_t pNode, RBNodeP_t pRight)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->pRight = pRight;

ERROR:
    return pNode;
}

/*!
*@brief        ���ýڵ�����
*@author       zhaohm3
*@param[in]    pNode    �����õĽڵ�ָ��
*@param[in]    color    �ڵ���ɫ
*@param[in]    pData    ����
*@param[in]    pParent  ���ڵ�
*@param[in]    pLeft    ���ӽڵ�
*@param[in]    pRight   ���ӽڵ�
*@retval
*@note
* 
*@since    2014-9-5 10:59
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_set(RBNodeP_t pNode,
                     RBColor_t color,
                     RBDataP_t pData,
                     RBNodeP_t pParent,
                     RBNodeP_t pLeft,
                     RBNodeP_t pRight)
{
    GCMON_CHECK_NULL(pNode, ERROR);
    pNode->mColor = color;
    pNode->pData = pData;
    pNode->pParent = pParent;
    pNode->pLeft = pLeft;
    pNode->pRight = pRight;

ERROR:
    return pNode;
}

/*!
*@brief        ��ȡ�ڵ���ɫ
*@author       zhaohm3
*@param[in]    pNode
*@retval
*@note
* 
*@since    2014-9-5 11:00
*@attention
* 
*/
GPrivate RBColor_t rbnode_get_color(RBNodeP_t pNode)
{
    RBColor_t color = RBC_NONE;
    GCMON_CHECK_NULL(pNode, ERROR);
    color = pNode->mColor;

ERROR:
    return color;
}

/*!
*@brief        ��ȡ�ڵ����洢������
*@author       zhaohm3
*@param[in]    pNode
*@retval
*@note
* 
*@since    2014-9-5 11:02
*@attention
* 
*/
GPrivate RBDataP_t rbnode_get_data(RBNodeP_t pNode)
{
    RBDataP_t pData = NULL;
    GCMON_CHECK_NULL(pNode, ERROR);
    pData = pNode->pData;

ERROR:
    return pData;
}

/*!
*@brief        ��ȡ���ڵ�
*@author       zhaohm3
*@param[in]    pNode
*@retval
*@note
* 
*@since    2014-9-5 11:02
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_get_parent(RBNodeP_t pNode)
{
    RBNodeP_t pParent = NULL;
    GCMON_CHECK_NULL(pNode, ERROR);
    pParent = pNode->pParent;

ERROR:
    return pParent;
}

/*!
*@brief        ��ȡ���ӽڵ�
*@author       zhaohm3
*@param[in]    pNode
*@retval
*@note
* 
*@since    2014-9-5 11:02
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_get_left(RBNodeP_t pNode)
{
    RBNodeP_t pLeft = NULL;
    GCMON_CHECK_NULL(pNode, ERROR);
    pLeft = pNode->pLeft;

ERROR:
    return pLeft;
}

/*!
*@brief        ��ȡ���ӽڵ�
*@author       zhaohm3
*@param[in]    pNode
*@retval
*@note
* 
*@since    2014-9-5 11:02
*@attention
* 
*/
GPrivate RBNodeP_t rbnode_get_right(RBNodeP_t pNode)
{
    RBNodeP_t pRight = NULL;
    GCMON_CHECK_NULL(pNode, ERROR);
    pRight = pNode->pRight;

ERROR:
    return pRight;
}


/************************************************************************/
/*                              ������ӿ�ʵ��                             */
/************************************************************************/

 /*!
 *@brief        ��ȡ������ĸ��ڵ�
 *@author       zhaohm3
 *@param[in]    pTree
 *@retval
 *@note
 * 
 *@since    2014-9-5 11:02
 *@attention
 * 
 */
 GPrivate RBNodeP_t rbtree_get_root(RBTreeP_t pTree)
{
    RBNodeP_t pRoot = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pRoot = pTree->pRoot;

ERROR:
    return pRoot;
}

/*!
*@brief        ���ú�����ĸ��ڵ�
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pRoot
*@retval
*@note
* 
*@since    2014-9-5 11:29
*@attention
* 
*/
GPrivate RBTreeP_t rbtree_set_root(RBTreeP_t pTree, RBNodeP_t pRoot)
{
    GCMON_CHECK_NULL(pTree, ERROR);
    pTree->pRoot = pRoot;

ERROR:
    return pTree;
}

/*!
*@brief        ����һ�ſյĺ����
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-5 11:29
*@attention
* 
*/
GPublic RBTreeP_t rbtree_new()
{
    RBTreeP_t pTree = NULL;
    GMALLOC(pTree, RBTree_t, RBTREE_SIZE);
    GCMON_CHECK_NULL(pTree, ERROR);
    pTree->pRoot = RBNIL;
    pTree->dwCount = 0;
    pTree->pfnCompare = NULL;
    pTree->pfnFree = NULL;
    pTree->pfnCombiner = NULL;

    return pTree;

ERROR:
    GFREE(pTree);
    return NULL;
}

/*!
*@brief        �ͷź������ռ�õ��ڴ�ռ䣬�������������ŵ�������ռ�õĿռ�
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note
* 
*@since    2014-9-5 11:29
*@attention
* 
*/
GPublic void rbtree_free(RBTreeP_t pTree)
{
    GCMON_CHECK_NULL(pTree, ERROR);
    rbnode_free_tree(pTree->pRoot, pTree->pfnFree);
    GFREE(pTree);

ERROR:
    return;
}

/*!
*@brief        ָ�������������������͵ıȽϺ���
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnCompare �ȽϺ���ָ�룬�����������ͣ�����Int32_t�ıȽϽ��ֵ
*@retval
*@note
* 
*@since    2014-9-5 11:34
*@attention
* 
*/
GPublic RBTreeP_t rbtree_set_rbd_compare(RBTreeP_t pTree,
                                         Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t))
{
    GCMON_CHECK_NULL(pTree, ERROR);
    pTree->pfnCompare = pfnCompare;

ERROR:
    return  pTree;
}

/*!
*@brief        ָ�������������������͵��ڴ�ռ��ͷź���
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnFree  ������ͷ����ݿռ�Ľӿ�ָ��
*@retval
*@note
* 
*@since    2014-9-5 11:37
*@attention
* 
*/
GPublic RBTreeP_t rbtree_set_rbd_free(RBTreeP_t pTree,
                                      void (*pfnFree)(RBDataP_t))
{
    GCMON_CHECK_NULL(pTree, ERROR);
    pTree->pfnFree = pfnFree;

ERROR:
    return pTree;
}

/*!
*@brief        ָ��������ڵ����洢�����ݵ����Ϻ���
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnCombiner  �������Ϻ���ָ��
*@retval
*@note
* 
*@since    2014-9-5 11:39
*@attention
* 
*/
GPublic RBTreeP_t rbtree_set_rbd_combiner(RBTreeP_t pTree,
                                          RBDataP_t(*pfnCombiner)(RBDataP_t, RBDataP_t))
{
    GCMON_CHECK_NULL(pTree, ERROR);
    pTree->pfnCombiner = pfnCombiner;

ERROR:
    return pTree;
}

/*!
*@brief        ��ȡ��ǰ�����������ŵ��������͵ıȽϺ����ӿ�
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note
* 
*@since    2014-9-5 11:43
*@attention
* 
*/
GPublic Int32_t (*rbtree_get_rbd_compare(RBTreeP_t pTree))(RBDataP_t, RBDataP_t)
{
    Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t) = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pfnCompare = pTree->pfnCompare;

ERROR:
    return pfnCompare;
}


/*!
*@brief        ��ȡ��ǰ�����������ŵ��������͵��ڴ�ռ��ͷŽӿ�
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note
* 
*@since    2014-9-5 14:11
*@attention
* 
*/
GPublic void (*rbtree_get_rbd_free(RBTreeP_t pTree))(RBDataP_t)
{
    void (*pfnFree)(RBDataP_t) = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pfnFree = pTree->pfnFree;

ERROR:
    return pfnFree;
}

/*!
*@brief        ��ȡ������ڵ����洢�����ݵ����Ϻ���
*@author       zhaohm3
*@param[in]    rbtree_get_rbd_combiner
*@param[in]    pTree
*@retval
*@note
* 
*@since    2014-9-5 14:12
*@attention
* 
*/
GPublic RBDataP_t (*rbtree_get_rbd_combiner(RBTreeP_t pTree))(RBDataP_t, RBDataP_t)
{
    RBDataP_t(*pfnCombiner)(RBDataP_t, RBDataP_t) = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pfnCombiner = pTree->pfnCombiner;

ERROR:
    return pfnCombiner;
}

/*!
*@brief        ������ڵ����
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note         �˽ӿ�Ҫ�����pTree����Ϊ��
* 
*@since    2014-9-5 14:12
*@attention
* 
*/
GPublic Count32_t rbtree_node_count(RBTreeP_t pTree)
{
    GASSERT(pTree != NULL);
    return pTree->dwCount;
}


/*!
*@brief        ������Ƿ�Ϊ��
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note         �˽ӿ�Ҫ�����pTree���ܹ�Ϊ��
* 
*@since    2014-9-5 14:12
*@attention
* 
*/
GPublic Bool32_t rbtree_is_empty(RBTreeP_t pTree)
{
    GASSERT(pTree != NULL);
    return (Bool32_t)(0 == pTree->dwCount);
}

 /*!
 *@brief        �������������
 *@author       zhaohm3
 *@param[in]    pTree
 *@param[in]    pSubRoot
 *@return
 *@retval
 *@note
 *
 *      p          r
 *         r   p
 *      q          q
 * 
 *@since    2014-9-4 10:21
 *@attention
 * 
 */
GPrivate RBTreeP_t rbtree_rotate_left(RBTreeP_t pTree, RBNodeP_t pSubRoot)
{
    RBNodeP_t p = NULL, q = NULL, r = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pSubRoot, ERROR);

    //! ȡ�ýڵ�p�����ӽڵ�
    p = pSubRoot;
    r = p->pRight;
    q = r->pLeft;

    //! ��r�����ӽڵ����ӵ�p���ҽڵ���
    p->pRight = q;

    //! ��r�����ӽڵ��pParentָ��p�ڵ�
    if (q != RBNIL)
    {
        q->pParent = p;
    }

    if (r != RBNIL)
    {
        r->pParent = p->pParent;
    }

    //! ���p�Ѿ��Ǹ��ڵ�
    if (NULL == p->pParent)
    {
        pTree->pRoot = r;
    }
    //! ���p�Ǹ��ڵ�����ӽڵ�
    else if (p == p->pParent->pLeft)
    {
        //! ��r��Ϊp�ĸ��ڵ�����ӽڵ�
        p->pParent->pLeft = r;
    }
    else
    {
        //! ��r��Ϊp�ĸ��ڵ�����ӽڵ�
        p->pParent->pRight = r;
    }

    r->pLeft = p;

    if (p != RBNIL)
    {
        p->pParent = r;
    }

ERROR:
    return pTree;
}

 /*!
 *@brief        �������������
 *@author       zhaohm3
 *@param[in]    pTree
 *@param[in]    pSubRoot
 *@return
 *@retval
 *@note
 * 
 *         p   l
 *      l          p
 *         q   q
 *
 *@since    2014-9-4 14:21
 *@attention
 * 
 */
GPrivate RBTreeP_t rbtree_roatate_right(RBTreeP_t pTree, RBNodeP_t pSubRoot)
{
    RBNodeP_t p = NULL, q = NULL, l = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pSubRoot, ERROR);

    //! ȡ��p�����ӽڵ�
    p = pSubRoot;
    l = p->pLeft;
    q = l->pRight;

    //! ��l�����ӽڵ����ӵ�p����ڵ���
    p->pLeft = q;

    //! ��l�����ӽڵ��pParentָ��p�ڵ�
    if (q != RBNIL)
    {
        q->pParent = p;
    }

    if (l != RBNIL)
    {
        l->pParent = p->pParent;
    }

    //! ���p�Ѿ��Ǹ��ڵ�
    if (NULL == p->pParent)
    {
        pTree->pRoot = l;
    }
    //! ���p�Ǹ��ڵ�����ӽڵ�
    else if (p == p->pParent->pRight)
    {
        //! ��l��Ϊp�ĸ��ڵ�����ӽڵ�
        p->pParent->pRight = l;
    }
    else
    {
        //! ��l��Ϊp�ĸ��ڵ�����ӽڵ�
        p->pParent->pLeft = l;
    }

    l->pRight = p;

    if (p != RBNIL)
    {
        p->pParent = l;
    }

ERROR:
    return pTree;
}

/*!
*@brief        ����ڵ���޸������
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pSubRoot
*@retval
*@note
* 
*@since    2014-9-5 14:50
*@attention
* 
*/
GPrivate RBTreeP_t rbtree_insert_fixup(RBTreeP_t pTree, RBNodeP_t pSubRoot)
{
    RBNodeP_t pNode = pSubRoot;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pNode, ERROR);

    //! ֱ��pNode�ڵ㲻�Ǹ��ڵ㣬����pNode�ĸ��ڵ㲻�Ǻ�ɫ
    while (pNode != pTree->pRoot && RBC_RED == pNode->pParent->mColor)
    {
        //! ���pNode�ĸ��ڵ����丸�ڵ�����ӽڵ�
        if (pNode->pParent == pNode->pParent->pParent->pLeft)
        {
            //! ��ȡpNode�ڵ�ĸ��ڵ���ֵܽڵ�
            RBNodeP_t pBrother = pNode->pParent->pParent->pRight;

            //! ���pNode�ĸ��ڵ���ֵܽڵ��Ǻ�ɫ
            if (RBC_RED == pBrother->mColor)
            {
                //! ��pNode�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pNode�ĸ��ڵ���ֵܽڵ���ɫ����Ϊ��ɫ
                pBrother->mColor = RBC_BLACK;

                //! ��pNode���ڵ�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->pParent->mColor = RBC_RED;
                pNode = pNode->pParent->pParent;
            }
            //! ���pNode�ĸ��ڵ���ֵܽڵ��Ǻ�ɫ
            else
            {
                //! ���pNode���丸�ڵ�����ӽڵ�
                if (pNode == pNode->pParent->pRight)
                {
                    //! ��pNode����ΪpNode�ĸ��ڵ�
                    pNode = pNode->pParent;
                    rbtree_rotate_left(pTree, pNode);
                }

                //! ��pNode�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pNode�ĸ��ڵ�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->pParent->mColor = RBC_RED;
                rbtree_roatate_right(pTree, pNode->pParent->pParent);
            }
        }
        //! ���pNode�ĸ��ڵ����丸�ڵ�����ӽڵ�
        else
        {
            //! ��ȡpNode�ڵ�ĸ��ڵ���ֵܽڵ�
            RBNodeP_t pBrother = pNode->pParent->pParent->pLeft;

            //! ���pNode�ĸ��ڵ���ֵܽڵ��Ǻ�ɫ
            if (RBC_RED == pBrother->mColor)
            {
                //! ��pNode�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pNode�ĸ��ڵ���ֵܽڵ���ɫ����Ϊ��ɫ
                pBrother->mColor = RBC_BLACK;

                //! ��pNode���ڵ�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->pParent->mColor = RBC_RED;

                //! ��pNode����ΪpNode�ĸ��ڵ�ĸ��ڵ�
                pNode = pNode->pParent->pParent;
            }
            //! ���pNode�ĸ��ڵ���ֵܽڵ��Ǻ�ɫ
            else
            {
                //! ���pNode���丸�ڵ�����ӽڵ�
                if (pNode == pNode->pParent->pLeft)
                {
                    //! ��pNode����ΪpNode�ĸ��ڵ�
                    pNode = pNode->pParent;
                    rbtree_roatate_right(pTree, pNode);
                }

                //! ��pNode�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pNode�ĸ��ڵ�ĸ��ڵ�����Ϊ��ɫ
                pNode->pParent->pParent->mColor = RBC_RED;
                rbtree_rotate_left(pTree, pNode->pParent->pParent);
            }
        }
    }

    //! �����ڵ�����Ϊ��ɫ
    pTree->pRoot->mColor = RBC_BLACK;

ERROR:
    return pTree;
}

/*!
*@brief        ɾ���ڵ���޸������
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pSubRoot
*@retval
*@note
* 
*@since    2014-9-9 11:08
*@attention
* 
*/
GPrivate RBTreeP_t rbtree_delete_fixup(RBTreeP_t pTree, RBNodeP_t pSubRoot)
{
    RBNodeP_t pNode = pSubRoot;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pNode, ERROR);

    //! ֱ��pNode���Ǹ��ݵ㣬������ɫ�Ǻ�ɫ
    while (pNode != pTree->pRoot && RBC_BLACK == pNode->mColor)
    {
        //! ���pNode���丸�ڵ�����ӽڵ�
        if (pNode == pNode->pParent->pLeft)
        {
            //! ��ȡpNode�ڵ���ֵܽڵ�
            RBNodeP_t pBrother = pNode->pParent->pRight;

            //! ���pBrother��ɫΪ��ɫ
            if (RBC_RED == pBrother->mColor)
            {
                //! ��pBrother��ɫ��Ϊ��ɫ
                pBrother->mColor = RBC_BLACK;

                //! ��pNode���ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_RED;
                rbtree_rotate_left(pTree, pNode->pParent);

                //! �ٴν�pBrother����ΪpNode�ĸ��ڵ�����ӽڵ�
                pBrother = pNode->pParent->pRight;
            }

            //! ���pBrother�������ӽڵ㶼�Ǻ�ɫ
            if (RBC_BLACK == pBrother->pLeft->mColor && RBC_BLACK == pBrother->pRight->mColor)
            {
                //! ��pBrother����Ϊ��ɫ
                pBrother->mColor = RBC_RED;

                //! ��pNode����pNode�ĸ��ڵ�
                pNode = pNode->pParent;
            }
            else
            {
                //! ���pBrotherֻ���ҽڵ��Ǻ�ɫ
                if (RBC_BLACK == pBrother->pRight->mColor)
                {
                    //! ��pBrother�����ӽڵ�Ҳ����Ϊ��ɫ
                    pBrother->pLeft->mColor = RBC_BLACK;

                    //! ��pBrother����Ϊ��ɫ
                    pBrother->mColor = RBC_RED;
                    rbtree_roatate_right(pTree, pBrother);
                    pBrother = pNode->pParent->pRight;
                }

                //! ����pBrother����ɫ��pNode�ĸ��ڵ����ɫ��ͬ
                pBrother->mColor = pNode->pParent->mColor;

                //! ��pNode���ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pBrother���ӽڵ���Ϊ��ɫ
                pBrother->pRight->mColor = RBC_BLACK;
                rbtree_rotate_left(pTree, pNode->pParent);
                pNode = pTree->pRoot;
            }
        }
        //! ���pNode���丸�ڵ�����ӽڵ�
        else
        {
            //! ��ȡpNode�ڵ���ֵܽڵ�
            RBNodeP_t pBrother = pNode->pParent->pLeft;

            //! ���pBrother��ɫΪ��ɫ
            if (RBC_RED == pBrother->mColor)
            {
                //! ��pBrother��ɫ��Ϊ��ɫ
                pBrother->mColor = RBC_BLACK;

                //! ��pNode���ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_RED;
                rbtree_roatate_right(pTree, pNode->pParent);

                //! �ٴν�pBrother����ΪpNode�ĸ��ڵ�����ӽڵ�
                pBrother = pNode->pParent->pLeft;
            }

            //! ���pBrother�������ӽڵ㶼�Ǻ�ɫ
            if (RBC_BLACK == pBrother->pLeft->mColor && RBC_BLACK == pBrother->pRight->mColor)
            {
                //! ��pBrother����Ϊ��ɫ
                pBrother->mColor = RBC_RED;

                //! ��pNode����pNode�ĸ��ڵ�
                pNode = pNode->pParent;
            }
            else
            {
                //! ���pBrotherֻ����ڵ��Ǻ�ɫ
                if (RBC_BLACK == pBrother->pLeft->mColor)
                {
                    //! ��pBrother�����ӽڵ�Ҳ����Ϊ��ɫ
                    pBrother->pRight->mColor = RBC_BLACK;

                    //! ��pBrother����Ϊ��ɫ
                    pBrother->mColor = RBC_RED;
                    rbtree_rotate_left(pTree, pBrother);
                    pBrother = pNode->pParent->pLeft;
                }

                //! ����pBrother����ɫ��pNode�ĸ��ڵ����ɫ��ͬ
                pBrother->mColor = pNode->pParent->mColor;

                //! ��pNode���ڵ�����Ϊ��ɫ
                pNode->pParent->mColor = RBC_BLACK;

                //! ��pBrother���ӽڵ���Ϊ��ɫ
                pBrother->pLeft->mColor = RBC_BLACK;
                rbtree_roatate_right(pTree, pNode->pParent);
                pNode = pTree->pRoot;
            }
        }
    }

    pNode->mColor = RBC_BLACK;

ERROR:
    return pTree;
}

/*!
*@brief        ���ݸ�����pData��pTree�鵽��Ӧ�Ľڵ�
*@author       zhaohm3
*@param[in]    pTree    �����
*@param[in]    pData    �����ҵ�����
*@retval       �����ΪNULLʱ��ֱ�ӷ���NULL����ʾ����ʧ�ܣ��������ΪNULLʱ��������NULL�����ʾ������
*@note         �����pData����ΪNULL����Ϊ��������Դ���κ����͵����ݵ��κ�ֵ������NULL
* 
*@since    2014-9-5 15:12
*@attention
* 
*/
GPrivate RBNodeP_t rbtree_search_node(RBTreeP_t pTree, RBDataP_t pData)
{
    Int32_t sdwCompare = 0;
    RBNodeP_t pCurrent = NULL;
    RBNodeP_t pSearch = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pTree->pfnCompare, ERROR);

    pCurrent = pTree->pRoot;

    while (pCurrent != RBNIL)
    {
        sdwCompare = pTree->pfnCompare(pData, pCurrent->pData);

        if (0 == sdwCompare)
        {
            pSearch = pCurrent;
            break;
        }

        pCurrent = (sdwCompare > 0) ? pCurrent->pRight : pCurrent->pLeft;
    }

ERROR:
    return pSearch;
}

/*!
*@brief        �Ӻ������ɾ��ָ���Ľڵ�
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pNode
*@retval
*@note
*
*@since    2014-9-5 15:18
*@attention
*
*/
GPrivate RBTreeP_t rbtree_delete_node(RBTreeP_t pTree, RBNodeP_t pNode)
{
    RBNodeP_t pTarget = NULL;
    RBNodeP_t pReplace = NULL;

    GCMON_CHECK_NULL(pNode, ERROR);
    GCMON_CHECK_COND(pNode != RBNIL, ERROR);

    //! pTarget��ʾ������Ҫ�Ӻ������ɾ���Ľڵ�
    if (RBNIL == pNode->pLeft || RBNIL == pNode->pRight)
    {
        pTarget = pNode;
    }
    else
    {
        pTarget = pNode->pRight;

        //! �ҵ�pNode�������е���С�ڵ�
        while (pTarget->pLeft != RBNIL)
        {
            pTarget = pTarget->pLeft;
        }
    }

    //! ��pTarget�ڵ�������ӽڵ���ѡ��һ���滻�ڵ�
    if (pTarget->pLeft != RBNIL)
    {
        pReplace = pTarget->pLeft;
    }
    else
    {
        pReplace = pTarget->pRight;
    }

    //! ��ʼ�������������Ƴ�pTarget�ڵ�
    pReplace->pParent = pTarget->pParent;

    //! ��pReplace�ڵ㶥��pTarget�ڵ��λ��
    if (pTarget->pParent != NULL)
    {
        if (pTarget == pTarget->pParent->pLeft)
        {
            pTarget->pParent->pLeft = pReplace;
        }
        else
        {
            pTarget->pParent->pRight = pReplace;
        }
    }
    else
    {
        pTree->pRoot = pReplace;
    }

    //! ��pNode�ڵ��е����ݷŵ�pTarget�У�����pTarget�е����ݱ��浽pNode�У����pTarget�е����ݽ����ͷ�
    if (pTarget != pNode)
    {
        RBDataP_t pData = pNode->pData;
        pNode->pData = pTarget->pData;
        pTarget->pData = pData;
    }

    //! �޸������
    if (RBC_BLACK == pTarget->mColor)
    {
        rbtree_delete_fixup(pTree, pReplace);
    }

    //! �ͷ�pTarget�ڵ�
    GASSERT(pTarget != NULL);
    rbnode_free(pTarget, pTree->pfnFree);
    pTree->dwCount--;

ERROR:
    return pTree;
}

//! ��һ��ɾ���ڵ�ķ�ʽ
/*!
GPrivate RBTreeP_t rbtree_delete_node_back(RBTreeP_t pTree, RBNodeP_t pNode)
{
    RBNodeP_t pTarget = NULL;
    RBNodeP_t pReplace = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pNode, ERROR);

    pTarget = pNode;

    //! �����ɾ���Ľڵ�������������Ϊ��
    if (pTarget->pLeft != RBNIL && pTarget->pRight != RBNIL)
    {
        //! �ҵ�pTarget�ڵ����������ǰһ���ڵ�pLMax���ýڵ�ΪpTarget�ڵ�������������ֵ�Ľڵ�
        RBNodeP_t pLMax = pTarget->pLeft;
        RBDataP_t pTargetData = NULL;

        while (pLMax->pRight != RBNIL)
        {
            pLMax = pLMax->pRight;
        }

        //! ����pTarget��pLMax�����ݣ��൱�ڽ�pTarget�ƶ���pLMax��λ�ã��ȴ��Ӻ������ɾ��
        pTargetData = pTarget->pData;
        pTarget->pData = pLMax->pData;
        pLMax->pData = pTargetData;
        pTarget = pLMax;
    }

    //! ��ʼ�޸��滻�ڵ㣬����ýڵ㲻Ϊ��
    pReplace = (pTarget->pLeft != RBNIL) ? pTarget->pLeft : pTarget->pRight;

    if (pReplace != RBNIL)
    {
        //! ��pReplace��parentָ��pTarget��parent
        pReplace->pParent = pTarget->pParent;

        //! ���pTarget��parentΪ�գ�����pTarget�����Ǹ��ڵ�
        if (NULL == pTarget->pParent)
        {
            pTree->pRoot = pReplace;
        }
        //! ���pTarget���丸�ڵ�����ӽڵ�
        else if (pTarget == pTarget->pParent->pLeft)
        {
            //! ��pTarget�ĸ��ڵ��leftָ��pReplace
            pTarget->pParent->pLeft = pReplace;
        }
        //! ���pTarget���丸�ڵ�����ӽڵ�
        else
        {
            //! ��pTarget�ĸ��ڵ��rightָ��pReplace
            pTarget->pParent->pRight = pReplace;
        }

        //! �����Ƴ�pTarget�ڵ�
        pTarget->pLeft = pTarget->pRight = pTarget->pParent = NULL;

        //! �޸������
        if (RBC_BLACK == pTarget->mColor)
        {
            rbtree_delete_fixup(pTree, pReplace);
        }
    }
    //! pTarget�����Ǹ��ڵ�
    else if (NULL == pTarget->pParent)
    {
        pTree->pRoot = RBNIL;
    }
    else
    {
        //! pTargetû���ӽڵ㣬������������滻�ڵ㣬�޸������
        if (RBC_BLACK == pTarget->mColor)
        {
            rbtree_delete_fixup(pTree, pTarget);
        }

        if (pTarget->pParent != NULL)
        {
            //! ���pTarget���丸�ڵ�����ӽڵ�
            if (pTarget == pTarget->pParent->pLeft)
            {
                //! ��pTarget���ڵ��left����Ϊ��
                pTarget->pParent->pLeft = RBNIL;
            }
            //! ���pTarget���丸�ڵ�����ӽڵ�
            else if (pTarget == pTarget->pParent->pRight)
            {
                //! ��pTarget���ڵ��right����Ϊ��
                pTarget->pParent->pRight = RBNIL;
            }

            //! ��pTarget��parent����Ϊ��
            pTarget->pParent = NULL;
        }
    }

    //! GCMON_CHECK_NULL(pTarget, ERROR);
    GASSERT(pTarget != NULL);
    rbnode_free(pTarget, pTree->pfnFree);
    pTree->dwCount--;

ERROR:
    return pTree;
}
*/

/*!
*@brief        ��pData��������
*@author       zhaohm3
*@param[in]    pTree    �����
*@param[in]    pData    �����������
*@param[in]    pbExist  ������������Ƿ��Ѿ������ں������
*@retval
*@note
* 
*@since    2014-9-5 15:31
*@attention
* 
*/
GPublic RBTreeP_t rbtree_insert(RBTreeP_t pTree, RBDataP_t pData, Bool32P_t pbExist)
{
    Bool32_t bExist = FALSE;
    Int32_t sdwCompare = 0;
    RBNodeP_t pCurrent = NULL;
    RBNodeP_t pParent = NULL;
    RBNodeP_t pInsert = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pTree->pfnCompare, ERROR);

    pCurrent = pTree->pRoot;

    //! �Ӹ��ڵ㿪ʼѰ�Һ��ʵĲ���λ��
    while (pCurrent != RBNIL)
    {
        sdwCompare = pTree->pfnCompare(pData, pCurrent->pData);

        //! ������������Ѿ��ں�����д���
        if (0 == sdwCompare)
        {
            bExist = TRUE;

            //! ������Ӻ�����Ϊ�գ����Խ�pData��pCurrent->pData�������ӣ��ŵ�pCurrent�ڵ���
            if (pTree->pfnCombiner != NULL)
            {
                pCurrent->pData = pTree->pfnCombiner(pCurrent->pData, pData);

                //! ���Ӻ󣬾��൱�ڽ�pData���������ˣ������ټ������ҽڵ���
                goto ERROR;
            }
        }

        //! ��������
        pParent = pCurrent;
        pCurrent = (sdwCompare > 0) ? pCurrent->pRight : pCurrent->pLeft;
    }

    //! pData�����в����ڣ���Ҫ����ڵ㣬Ȼ�����
    pInsert = rbnode_new();
    GCMON_CHECK_NULL(pInsert, ERROR);

    //! ��������Ľڵ���ɫ����Ϊ��ɫ
    rbnode_set(pInsert, RBC_RED, pData, pParent, RBNIL, RBNIL);

    //! ��������Ľڵ���ӵ������
    if (pParent != NULL)
    {
        if (sdwCompare > 0)
        {
            pParent->pRight = pInsert;
        }
        else
        {
            pParent->pLeft = pInsert;
        }
    }
    else
    {
        pTree->pRoot = pInsert;
    }

    //! �޸��������ʹ֮����ƽ��
    rbtree_insert_fixup(pTree, pInsert);
    pTree->dwCount++;

ERROR:

    if (pbExist != NULL)
    {
        *pbExist = bExist;
    }

    return pTree;
}

/*!
*@brief        �Ӻ������ɾ���洢pData�Ľڵ�
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pData
*@retval
*@note
* 
*@since    2014-9-5 15:43
*@attention
* 
*/
GPublic RBTreeP_t rbtree_delete(RBTreeP_t pTree, RBDataP_t pData)
{
    RBNodeP_t pDelete = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pDelete = rbtree_search_node(pTree, pData);
    GCMON_CHECK_NULL(pDelete, ERROR);
    rbtree_delete_node(pTree, pDelete);

ERROR:
    return pTree;
}

/*!
*@brief        �Ӻ�����в���pData
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pData
*@retval
*@note
* 
*@since    2014-9-5 15:45
*@attention
* 
*/
GPublic RBDataP_t rbtree_search(RBTreeP_t pTree, RBDataP_t pData)
{
    RBNodeP_t pTarget = NULL;
    RBDataP_t pSearch = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    pTarget = rbtree_search_node(pTree, pData);
    GCMON_CHECK_NULL(pTarget, ERROR);
    pSearch = pTarget->pData;

ERROR:
    return pSearch;
}