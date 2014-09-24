#include "list/list.h"
#include "share/share.h"
#include "utest.h"

GPrivate Int32_t uld_compare(LDataP_t pSrc, LDataP_t  pDes)
{
    return (*(Int32P_t)pSrc) - (*(Int32P_t)pDes);
}

GPrivate void uld_free(LDataP_t pData)
{
    GFREE(pData);
}

GPrivate ListP_t ul_append(ListP_t pList, Int32_t data)
{
    Int32P_t pData = NULL;

    do
    {
        GMALLOC(pData, Int32_t, sizeof(Int32_t));
    } while (NULL == pData);

    *pData = data;
    list_append(pList, pData);
    return pList;
}

GPrivate ListP_t ul_cons(ListP_t pList, Int32_t data)
{
    Int32P_t pData = NULL;

    do
    {
        GMALLOC(pData, Int32_t, sizeof(Int32_t));
    } while (NULL == pData);

    *pData = data;
    list_cons(pList, pData);
    return pList;
}

GPrivate ListP_t ul_remove(ListP_t pList, Int32_t data)
{
    Int32P_t pData = &data;

    list_remove(pList, pData);
    return pList;
}

GPrivate Int32_t ul_find_data(ListP_t pList, Int32_t data, Bool32P_t pbExist)
{
    Bool32_t bExist = FALSE;
    Int32P_t pData = NULL;

    pData = list_find_data(pList, &data, &bExist);

    GASSERT(bExist ? pData != NULL && *pData == data : !bExist);

    if (pbExist != NULL)
    {
        *pbExist = bExist;
    }

    if (bExist)
    {
        return *pData;
    }
    else
    {
        return 0;
    }
}

GPrivate ListP_t ul_new()
{
    ListP_t pList = NULL;

    pList = list_new();
    GCMON_CHECK_NULL(pList, ERROR);
    list_set_ldfree(pList, uld_free);
    list_set_ldcompare(pList, uld_compare);

ERROR:
    return pList;
}

GPublic void utest_list()
{
    ListP_t pList = NULL;
    Int32_t i = 0;
    Int32_t sdwCount = 1024;

    gcmon_debug_fopen();

    pList = ul_new();
    GASSERT(pList != NULL);

    for (i = 0; i < sdwCount; i++)
    {
        ul_append(pList, i);
        ul_cons(pList, i);
        ul_append(pList, 0 - i);
        ul_cons(pList, 0 - i);
    }

    for (i = 0; i < sdwCount; i++)
    {
        Int32_t j = 0;
        Bool32_t bExist = FALSE;

        j = ul_find_data(pList, i, &bExist);
        GASSERT(bExist && j == i);

        bExist = FALSE;
        j = ul_find_data(pList, 0 - i, &bExist);
        GASSERT(bExist && i + j == 0);
    }

    for (i = 3 * sdwCount / 4; i < sdwCount; i++)
    {
        Int32_t j = 0;
        Bool32_t bExist = FALSE;

        bExist = FALSE;
        ul_remove(pList, i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(bExist && j == i);

        bExist = FALSE;
        ul_remove(pList, i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(!bExist);

        bExist = FALSE;
        ul_remove(pList, 0 - i);
        j = ul_find_data(pList, 0 - i, &bExist);
        GASSERT(bExist && j == 0 - i);

        bExist = FALSE;
        ul_remove(pList, 0 - i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(!bExist);
    }

    for (i = 1; i < sdwCount / 4; i++)
    {
        Int32_t j = 0;
        Bool32_t bExist = FALSE;

        bExist = FALSE;
        ul_remove(pList, i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(bExist && j == i);

        bExist = FALSE;
        ul_remove(pList, i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(!bExist);

        bExist = FALSE;
        ul_remove(pList, 0 - i);
        j = ul_find_data(pList, 0 - i, &bExist);
        GASSERT(bExist && j == 0 - i);

        bExist = FALSE;
        ul_remove(pList, 0 - i);
        j = ul_find_data(pList, i, &bExist);
        GASSERT(!bExist);
    }

    list_free(pList);
    gcmon_debug_fclose();
}

GPrivate void utest_strcmp()
{
    Int32_t sdwCompare = 0;

    sdwCompare = (Int32_t)os_strcmp(NULL, NULL);
    sdwCompare = (Int32_t)os_strcmp(NULL, "");
    sdwCompare = (Int32_t)os_strcmp("", NULL);
    sdwCompare = (Int32_t)os_strcmp("", "");
    sdwCompare = (Int32_t)os_strcmp("", "  ");
    sdwCompare = (Int32_t)os_strcmp(" ", "  ");
    sdwCompare = (Int32_t)os_strcmp("  ", "");
    sdwCompare = (Int32_t)os_strcmp("  ", " ");
}

GPublic void utest_list_run()
{
    UItem_t aItems[] =
    {
        { 0, TRUE, NULL, NULL, "返回上一级菜单" },
        { 1, TRUE, utest_list, NULL, "测试链表" },
        /* { 2, TRUE, utest_strcmp, NULL, "测试strcmp" },*/
    };

    utest_run(aItems, ARRAY_SIZE(aItems));
}
