
#include "rbtree/rbtree.h"
#include "share/share.h"
#include "utest.h"

GPrivate Int32_t urbd_compare(RBDataP_t pSrc, RBDataP_t pDes)
{
    return (*(Int32P_t)pSrc) - (*(Int32P_t)pDes);
}

GPrivate void urbd_free(RBDataP_t pData)
{
    GFREE(pData);
}

GPrivate RBTreeP_t urbt_insert(RBTreeP_t pTree, Int32_t data)
{
    Int32P_t pData = NULL;
    do 
    {
        GMALLOC(pData, Int32_t, sizeof(Int32_t));
    } while (NULL == pData);
    *pData = data;
    /*printf("pData --> 0x%p : %d \n ", pData, data);*/
    return rbtree_insert(pTree, pData, NULL);
}

GPrivate RBTreeP_t urbt_delete(RBTreeP_t pTree, Int32_t data)
{
    Int32P_t pData = &data;
    return rbtree_delete(pTree, pData);
}

GPrivate Int32_t urbt_search(RBTreeP_t pTree, Int32_t data)
{
    Int32P_t pData = &data;
    Int32P_t pResult = NULL;

    pResult = (Int32P_t)rbtree_search(pTree, pData);

    if (pResult != NULL)
    {
        return *pResult;
    }

    return 0;
}

GPrivate RBTreeP_t urbt_new()
{
    RBTreeP_t pTree = NULL;
    pTree = rbtree_new();
    GCMON_CHECK_NULL(pTree, ERROR);
    rbtree_set_rbd_combiner(pTree, NULL);
    rbtree_set_rbd_compare(pTree, urbd_compare);
    rbtree_set_rbd_free(pTree, urbd_free);

ERROR:
    return pTree;
}

GPublic void utest_rbtree()
{
    RBTreeP_t pTree = NULL;
    Int32_t i = 0;
    Int32_t sdwCount = 1024000;

    gcmon_debug_fopen();

    pTree = urbt_new();

    for (i = 0; i < sdwCount; i++)
    {
        urbt_insert(pTree, i);
        urbt_insert(pTree, 0 - i);
    }

    for (i = 0; i < sdwCount; i++)
    {
        Int32_t j = 0;
        j = urbt_search(pTree, i);
        GASSERT(j == i);
        j = urbt_search(pTree, 0 - i);
        GASSERT(j == 0 - i);
    }

    for (i = 0; i < sdwCount / 4; i++)
    {
        urbt_delete(pTree, i);
        urbt_delete(pTree, 0 - i);
    }

    rbtree_free(pTree);
    gcmon_debug_fclose();
}

GPublic void utest_rbtree_run()
{
    UItem_t aItems[] =
    {
        { 0, TRUE, NULL, NULL, "返回上一级菜单"},
        { 1, TRUE, utest_rbtree, NULL, "测试红黑树"}
    };

    utest_run(aItems, ARRAY_SIZE(aItems));
}