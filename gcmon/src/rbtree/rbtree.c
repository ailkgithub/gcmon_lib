/*!**************************************************************
 *@file rbtree.cpp
 *@brief    红黑树的实现
 *@author   zhaohm3
 *@date 2014-9-3 18:27
 *@note
 * 
 ****************************************************************/


#include "rbtree/rbtree.h"


/************************************************************************/
/*                             结构体定义                                 */
/************************************************************************/

GTYPES(RBNode);
GTYPEE(RBColor);

//! 红黑树节点颜色枚举类
enum RBColor
{
    RBC_NONE = 0,                                   //!< 初始化默认色
    RBC_RED,                                        //!< 红色
    RBC_BLACK                                       //!< 黑色
};

//! 红黑树节点定义，此定义不对外提供，与此类相关的接口亦不对外提供
struct RBNode
{
    RBColor_t mColor;                               //!< 当前节点颜色
    RBDataP_t pData;                                //!< 当前节点存储的数据，可以使任意类型的指针
    RBNodeP_t pParent;                              //!< 当前节点的父节点
    RBNodeP_t pLeft;                                //!< 当前节点的左子节点
    RBNodeP_t pRight;                               //!< 当前节点的右子节点
};

//! 红黑树定义，此定义不对外提供，与此类型相关的接口仅提供部分必须的
struct RBTree
{
    RBNodeP_t pRoot;                                //!< 红黑树的跟节点
    Count32_t dwCount;                              //!< 红黑树节点个数
    Int32_t (*pfnCompare)(RBDataP_t, RBDataP_t);    //!< 红黑树节点所存储的数据的比较函数
    void (*pfnFree)(RBDataP_t);                     //!< 红黑树节点所存储的数据的空间释放函数
    RBDataP_t(*pfnCombiner)(RBDataP_t, RBDataP_t);  //!< 红黑树节点所存储的数据的连接函数
};

#define RBNODE_SIZE sizeof(RBNode_t)
#define RBTREE_SIZE sizeof(RBTree_t)

#define RBNIL ((RBNodeP_t)&gRBNIL)
GPrivate RBNode_t gRBNIL = { RBC_BLACK, NULL, NULL, RBNIL, RBNIL };

/************************************************************************/
/*                         红黑树节点接口声明                               */
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
/*                            红黑树接口声明                               */
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
/*                          红黑树节点接口实现                              */
/************************************************************************/

/*!
*@brief        申请一个空节点，节点颜色初始化为RBC_NONE
*@author       zhaohm3
*@retval       返回申请成功的节点指针
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

    //! 新创建的节点默认为黑色，左右子树为RBNIL，父节点为NULL
    rbnode_set(pNode, RBC_BLACK, NULL, NULL, RBNIL, RBNIL);

    return pNode;

ERROR:
    GFREE(pNode);
    return NULL;
}

/*!
*@brief        释放一个红黑树节点，节点所存储的数据空间由pfnFree函数释放
*@author       zhaohm3
*@param[in]    pNode    待释放的红黑树节点指针
*@param[in]    pfnFree  用于释放节点存储的数据的函数指针
*@retval
*@note         红黑树节点所存储的数据可以是任意类型，因此释放该类型的接口需要用户在创建红黑树的时候指定
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

    //! 由用户自定义接口释放节点存储的数据
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
*@brief        递归释放pRoot为根节点的二叉树所占用的空间
*@author       zhaohm3
*@param[in]    pRoot    根节点
*@param[in]    pfnFree  节点存储数据的释放接口指针
*@retval
*@note         树中每个节点所存储的数据占用的空间由pfnFree释放
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

    rbnode_free(pRoot, pfnFree);                   //!< 释放根节点所占用的空间
    rbnode_free_tree(pLeft, pfnFree);       //!< 递归释放左子树所占用的空间
    rbnode_free_tree(pRight, pfnFree);      //!< 递归释放右子树所占用的空间

ERROR:
    return;
}

/*!
*@brief        清空一个二叉树节点
*@author       zhaohm3
*@param[in]    pNode    待清空的节点
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
*@brief        设置节点颜色
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
*@brief        设置节点所存储的数据
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
*@brief        设置父节点
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
*@brief        设置左子节点
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
*@brief        设置右子节点
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
*@brief        设置节点内容
*@author       zhaohm3
*@param[in]    pNode    待设置的节点指针
*@param[in]    color    节点颜色
*@param[in]    pData    数据
*@param[in]    pParent  父节点
*@param[in]    pLeft    左子节点
*@param[in]    pRight   右子节点
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
*@brief        获取节点颜色
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
*@brief        获取节点所存储的数据
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
*@brief        获取父节点
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
*@brief        获取左子节点
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
*@brief        获取右子节点
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
/*                              红黑树接口实现                             */
/************************************************************************/

 /*!
 *@brief        获取红黑树的根节点
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
*@brief        设置红黑树的根节点
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
*@brief        创建一颗空的红黑树
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
*@brief        释放红黑树所占用的内存空间，包括红黑树所存放的数据所占用的空间
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
*@brief        指定红黑树所存放数据类型的比较函数
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnCompare 比较函数指针，接受任意类型，返回Int32_t的比较结果值
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
*@brief        指定红黑树所存放数据类型的内存空间释放函数
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnFree  红黑树释放数据空间的接口指针
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
*@brief        指定红黑树节点所存储的数据的联合函数
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    pfnCombiner  数据联合函数指针
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
*@brief        获取当前红黑树中所存放的数据类型的比较函数接口
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
*@brief        获取当前红黑树中所存放的数据类型的内存空间释放接口
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
*@brief        获取红黑树节点所存储的数据的联合函数
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
*@brief        红黑树节点个数
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note         此接口要求传入的pTree不能为空
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
*@brief        红黑树是否为空
*@author       zhaohm3
*@param[in]    pTree
*@retval
*@note         此接口要求传入的pTree不能够为空
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
 *@brief        红黑树左旋操作
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

    //! 取得节点p的右子节点
    p = pSubRoot;
    r = p->pRight;
    q = r->pLeft;

    //! 将r的左子节点连接到p的右节点上
    p->pRight = q;

    //! 让r的左子节点的pParent指向p节点
    if (q != RBNIL)
    {
        q->pParent = p;
    }

    if (r != RBNIL)
    {
        r->pParent = p->pParent;
    }

    //! 如果p已经是根节点
    if (NULL == p->pParent)
    {
        pTree->pRoot = r;
    }
    //! 如果p是父节点的左子节点
    else if (p == p->pParent->pLeft)
    {
        //! 将r设为p的父节点的左子节点
        p->pParent->pLeft = r;
    }
    else
    {
        //! 将r设为p的父节点的右子节点
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
 *@brief        红黑树右旋操作
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

    //! 取得p的左子节点
    p = pSubRoot;
    l = p->pLeft;
    q = l->pRight;

    //! 将l的右子节点连接到p的左节点上
    p->pLeft = q;

    //! 让l的右子节点的pParent指向p节点
    if (q != RBNIL)
    {
        q->pParent = p;
    }

    if (l != RBNIL)
    {
        l->pParent = p->pParent;
    }

    //! 如果p已经是根节点
    if (NULL == p->pParent)
    {
        pTree->pRoot = l;
    }
    //! 如果p是父节点的右子节点
    else if (p == p->pParent->pRight)
    {
        //! 将l设为p的父节点的右子节点
        p->pParent->pRight = l;
    }
    else
    {
        //! 将l设为p的父节点的左子节点
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
*@brief        插入节点后修复红黑树
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

    //! 直到pNode节点不是根节点，而且pNode的父节点不是红色
    while (pNode != pTree->pRoot && RBC_RED == pNode->pParent->mColor)
    {
        //! 如果pNode的父节点是其父节点的左子节点
        if (pNode->pParent == pNode->pParent->pParent->pLeft)
        {
            //! 获取pNode节点的父节点的兄弟节点
            RBNodeP_t pBrother = pNode->pParent->pParent->pRight;

            //! 如果pNode的父节点的兄弟节点是红色
            if (RBC_RED == pBrother->mColor)
            {
                //! 将pNode的父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pNode的父节点的兄弟节点颜色设置为黑色
                pBrother->mColor = RBC_BLACK;

                //! 将pNode父节点的父节点设置为红色
                pNode->pParent->pParent->mColor = RBC_RED;
                pNode = pNode->pParent->pParent;
            }
            //! 如果pNode的父节点的兄弟节点是黑色
            else
            {
                //! 如果pNode是其父节点的右子节点
                if (pNode == pNode->pParent->pRight)
                {
                    //! 将pNode设置为pNode的父节点
                    pNode = pNode->pParent;
                    rbtree_rotate_left(pTree, pNode);
                }

                //! 把pNode的父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pNode的父节点的父节点设置为红色
                pNode->pParent->pParent->mColor = RBC_RED;
                rbtree_roatate_right(pTree, pNode->pParent->pParent);
            }
        }
        //! 如果pNode的父节点是其父节点的右子节点
        else
        {
            //! 获取pNode节点的父节点的兄弟节点
            RBNodeP_t pBrother = pNode->pParent->pParent->pLeft;

            //! 如果pNode的父节点的兄弟节点是红色
            if (RBC_RED == pBrother->mColor)
            {
                //! 将pNode的父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pNode的父节点的兄弟节点颜色设置为黑色
                pBrother->mColor = RBC_BLACK;

                //! 将pNode父节点的父节点设置为红色
                pNode->pParent->pParent->mColor = RBC_RED;

                //! 将pNode设置为pNode的父节点的父节点
                pNode = pNode->pParent->pParent;
            }
            //! 如果pNode的父节点的兄弟节点是黑色
            else
            {
                //! 如果pNode是其父节点的左子节点
                if (pNode == pNode->pParent->pLeft)
                {
                    //! 将pNode设置为pNode的父节点
                    pNode = pNode->pParent;
                    rbtree_roatate_right(pTree, pNode);
                }

                //! 把pNode的父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pNode的父节点的父节点设置为红色
                pNode->pParent->pParent->mColor = RBC_RED;
                rbtree_rotate_left(pTree, pNode->pParent->pParent);
            }
        }
    }

    //! 将根节点设置为黑色
    pTree->pRoot->mColor = RBC_BLACK;

ERROR:
    return pTree;
}

/*!
*@brief        删除节点后修复红黑树
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

    //! 直到pNode不是根据点，且其颜色是黑色
    while (pNode != pTree->pRoot && RBC_BLACK == pNode->mColor)
    {
        //! 如果pNode是其父节点的左子节点
        if (pNode == pNode->pParent->pLeft)
        {
            //! 获取pNode节点的兄弟节点
            RBNodeP_t pBrother = pNode->pParent->pRight;

            //! 如果pBrother颜色为红色
            if (RBC_RED == pBrother->mColor)
            {
                //! 将pBrother颜色设为黑色
                pBrother->mColor = RBC_BLACK;

                //! 将pNode父节点设置为红色
                pNode->pParent->mColor = RBC_RED;
                rbtree_rotate_left(pTree, pNode->pParent);

                //! 再次将pBrother设置为pNode的父节点的右子节点
                pBrother = pNode->pParent->pRight;
            }

            //! 如果pBrother的两个子节点都是黑色
            if (RBC_BLACK == pBrother->pLeft->mColor && RBC_BLACK == pBrother->pRight->mColor)
            {
                //! 将pBrother设置为红色
                pBrother->mColor = RBC_RED;

                //! 让pNode等于pNode的父节点
                pNode = pNode->pParent;
            }
            else
            {
                //! 如果pBrother只有右节点是黑色
                if (RBC_BLACK == pBrother->pRight->mColor)
                {
                    //! 将pBrother的左子节点也设置为黑色
                    pBrother->pLeft->mColor = RBC_BLACK;

                    //! 将pBrother设置为红色
                    pBrother->mColor = RBC_RED;
                    rbtree_roatate_right(pTree, pBrother);
                    pBrother = pNode->pParent->pRight;
                }

                //! 设置pBrother的颜色与pNode的父节点的颜色相同
                pBrother->mColor = pNode->pParent->mColor;

                //! 将pNode父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pBrother右子节点设为黑色
                pBrother->pRight->mColor = RBC_BLACK;
                rbtree_rotate_left(pTree, pNode->pParent);
                pNode = pTree->pRoot;
            }
        }
        //! 如果pNode是其父节点的右子节点
        else
        {
            //! 获取pNode节点的兄弟节点
            RBNodeP_t pBrother = pNode->pParent->pLeft;

            //! 如果pBrother颜色为红色
            if (RBC_RED == pBrother->mColor)
            {
                //! 将pBrother颜色设为黑色
                pBrother->mColor = RBC_BLACK;

                //! 将pNode父节点设置为红色
                pNode->pParent->mColor = RBC_RED;
                rbtree_roatate_right(pTree, pNode->pParent);

                //! 再次将pBrother设置为pNode的父节点的右子节点
                pBrother = pNode->pParent->pLeft;
            }

            //! 如果pBrother的两个子节点都是黑色
            if (RBC_BLACK == pBrother->pLeft->mColor && RBC_BLACK == pBrother->pRight->mColor)
            {
                //! 将pBrother设置为红色
                pBrother->mColor = RBC_RED;

                //! 让pNode等于pNode的父节点
                pNode = pNode->pParent;
            }
            else
            {
                //! 如果pBrother只有左节点是黑色
                if (RBC_BLACK == pBrother->pLeft->mColor)
                {
                    //! 将pBrother的右子节点也设置为黑色
                    pBrother->pRight->mColor = RBC_BLACK;

                    //! 将pBrother设置为红色
                    pBrother->mColor = RBC_RED;
                    rbtree_rotate_left(pTree, pBrother);
                    pBrother = pNode->pParent->pLeft;
                }

                //! 设置pBrother的颜色与pNode的父节点的颜色相同
                pBrother->mColor = pNode->pParent->mColor;

                //! 将pNode父节点设置为黑色
                pNode->pParent->mColor = RBC_BLACK;

                //! 将pBrother右子节点设为黑色
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
*@brief        根据给定的pData在pTree查到相应的节点
*@author       zhaohm3
*@param[in]    pTree    红黑树
*@param[in]    pData    待查找的数据
*@retval       红黑树为NULL时，直接返回NULL，表示查找失败，红黑树不为NULL时，若返回NULL，则表示不存在
*@note         传入的pData可以为NULL，因为红黑树可以存放任何类型的数据的任何值，包括NULL
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
*@brief        从红黑树中删除指定的节点
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

    //! pTarget表示真正需要从红黑树中删除的节点
    if (RBNIL == pNode->pLeft || RBNIL == pNode->pRight)
    {
        pTarget = pNode;
    }
    else
    {
        pTarget = pNode->pRight;

        //! 找到pNode右子树中的最小节点
        while (pTarget->pLeft != RBNIL)
        {
            pTarget = pTarget->pLeft;
        }
    }

    //! 在pTarget节点的左右子节点中选择一个替换节点
    if (pTarget->pLeft != RBNIL)
    {
        pReplace = pTarget->pLeft;
    }
    else
    {
        pReplace = pTarget->pRight;
    }

    //! 开始从树中真正的移除pTarget节点
    pReplace->pParent = pTarget->pParent;

    //! 用pReplace节点顶替pTarget节点的位置
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

    //! 将pNode节点中的数据放到pTarget中，并将pTarget中的数据保存到pNode中，随后pTarget中的数据将被释放
    if (pTarget != pNode)
    {
        RBDataP_t pData = pNode->pData;
        pNode->pData = pTarget->pData;
        pTarget->pData = pData;
    }

    //! 修复红黑树
    if (RBC_BLACK == pTarget->mColor)
    {
        rbtree_delete_fixup(pTree, pReplace);
    }

    //! 释放pTarget节点
    GASSERT(pTarget != NULL);
    rbnode_free(pTarget, pTree->pfnFree);
    pTree->dwCount--;

ERROR:
    return pTree;
}

//! 另一种删除节点的方式
/*!
GPrivate RBTreeP_t rbtree_delete_node_back(RBTreeP_t pTree, RBNodeP_t pNode)
{
    RBNodeP_t pTarget = NULL;
    RBNodeP_t pReplace = NULL;

    GCMON_CHECK_NULL(pTree, ERROR);
    GCMON_CHECK_NULL(pNode, ERROR);

    pTarget = pNode;

    //! 如果被删除的节点左右子树都不为空
    if (pTarget->pLeft != RBNIL && pTarget->pRight != RBNIL)
    {
        //! 找到pTarget节点中序遍历的前一个节点pLMax，该节点为pTarget节点的左子树中最大值的节点
        RBNodeP_t pLMax = pTarget->pLeft;
        RBDataP_t pTargetData = NULL;

        while (pLMax->pRight != RBNIL)
        {
            pLMax = pLMax->pRight;
        }

        //! 交换pTarget和pLMax的数据，相当于将pTarget移动到pLMax的位置，等待从红黑树中删除
        pTargetData = pTarget->pData;
        pTarget->pData = pLMax->pData;
        pLMax->pData = pTargetData;
        pTarget = pLMax;
    }

    //! 开始修复替换节点，如果该节点不为空
    pReplace = (pTarget->pLeft != RBNIL) ? pTarget->pLeft : pTarget->pRight;

    if (pReplace != RBNIL)
    {
        //! 让pReplace的parent指向pTarget的parent
        pReplace->pParent = pTarget->pParent;

        //! 如果pTarget的parent为空，表明pTarget本身是根节点
        if (NULL == pTarget->pParent)
        {
            pTree->pRoot = pReplace;
        }
        //! 如果pTarget是其父节点的左子节点
        else if (pTarget == pTarget->pParent->pLeft)
        {
            //! 让pTarget的父节点的left指向pReplace
            pTarget->pParent->pLeft = pReplace;
        }
        //! 如果pTarget是其父节点的右子节点
        else
        {
            //! 让pTarget的父节点的right指向pReplace
            pTarget->pParent->pRight = pReplace;
        }

        //! 彻底移除pTarget节点
        pTarget->pLeft = pTarget->pRight = pTarget->pParent = NULL;

        //! 修复红黑树
        if (RBC_BLACK == pTarget->mColor)
        {
            rbtree_delete_fixup(pTree, pReplace);
        }
    }
    //! pTarget本身是根节点
    else if (NULL == pTarget->pParent)
    {
        pTree->pRoot = RBNIL;
    }
    else
    {
        //! pTarget没有子节点，把它当成虚的替换节点，修复红黑树
        if (RBC_BLACK == pTarget->mColor)
        {
            rbtree_delete_fixup(pTree, pTarget);
        }

        if (pTarget->pParent != NULL)
        {
            //! 如果pTarget是其父节点的左子节点
            if (pTarget == pTarget->pParent->pLeft)
            {
                //! 将pTarget父节点的left设置为空
                pTarget->pParent->pLeft = RBNIL;
            }
            //! 如果pTarget是其父节点的右子节点
            else if (pTarget == pTarget->pParent->pRight)
            {
                //! 将pTarget父节点的right设置为空
                pTarget->pParent->pRight = RBNIL;
            }

            //! 将pTarget的parent设置为空
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
*@brief        将pData插入红黑树
*@author       zhaohm3
*@param[in]    pTree    红黑树
*@param[in]    pData    待插入的数据
*@param[in]    pbExist  待插入的数据是否已经存在于红黑树中
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

    //! 从根节点开始寻找合适的插入位置
    while (pCurrent != RBNIL)
    {
        sdwCompare = pTree->pfnCompare(pData, pCurrent->pData);

        //! 待插入的数据已经在红黑树中存在
        if (0 == sdwCompare)
        {
            bExist = TRUE;

            //! 如果连接函数不为空，可以将pData和pCurrent->pData进行连接，放到pCurrent节点中
            if (pTree->pfnCombiner != NULL)
            {
                pCurrent->pData = pTree->pfnCombiner(pCurrent->pData, pData);

                //! 连接后，就相当于将pData插入红黑树了，不必再继续查找节点了
                goto ERROR;
            }
        }

        //! 继续查找
        pParent = pCurrent;
        pCurrent = (sdwCompare > 0) ? pCurrent->pRight : pCurrent->pLeft;
    }

    //! pData在树中不存在，需要申请节点，然后插入
    pInsert = rbnode_new();
    GCMON_CHECK_NULL(pInsert, ERROR);

    //! 将新申请的节点颜色设置为红色
    rbnode_set(pInsert, RBC_RED, pData, pParent, RBNIL, RBNIL);

    //! 将新申请的节点添加到红黑树
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

    //! 修复红黑树，使之保持平衡
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
*@brief        从红黑树中删除存储pData的节点
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
*@brief        从红黑树中查找pData
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