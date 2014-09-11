/*!**************************************************************
 *@file utest.cpp
 *@brief    单元测试框架相关接口实现
 *@author   zhaohm3
 *@date 2014-9-5 16:03
 *@note
 * 
 ****************************************************************/

#include "utest.h"


/*!
*@brief        打印单个测试项
*@author       zhaohm3
*@param[in]    pItem    待打印的测试项
*@retval
*@note
* 
*@since    2014-9-5 16:05
*@attention
* 
*/
GPrivate void uitem_print_single(UItemP_t pItem)
{
    printf("用例号：%d\t%s\n\n", pItem->sdwId, pItem->szDescribe);
}

/*!
*@brief        打印测试项数组
*@author       zhaohm3
*@param[in]    aItems   待打印的测试项数组
*@param[in]    sdwLen   数组元素个数
*@retval
*@note
* 
*@since    2014-9-5 16:10
*@attention
* 
*/
GPrivate void uitem_print_array(UItem_t aItems[], Int32_t sdwLen)
{
    Int32_t i = 0;
    UItemP_t pCurrent = NULL;

    printf("\n");

    for (i = 0; i < sdwLen; i++)
    {
        pCurrent = &aItems[i];

        if (pCurrent->bShow != FALSE)
        {
            uitem_print_single(pCurrent);
        }
    }

    printf("\n");
}

/*!
*@brief        通过ID号获取测试项数组中的项
*@author       zhaohm3
*@param[in]    aItems   测试项数组
*@param[in]    sdwLen   数组长度
*@param[in]    sdwId    需要获取的测试项ID号
*@retval
*@note
* 
*@since    2014-9-5 16:13
*@attention
* 
*/
GPrivate UItemP_t uitem_get_by_id(UItem_t aItems[], Int32_t sdwLen, Int32_t sdwId)
{
    UItemP_t pItem = NULL;
    Int32_t i = 0;

    for (i = 0; i < sdwLen; i++)
    {
        pItem = &aItems[i];

        if (pItem->sdwId == sdwId)
        {
            return pItem;
        }
    }

    return NULL;
}

/*!
*@brief        根据用户的输入运行指定的测试项
*@author       zhaohm3
*@param[in]    aItems   单元测试项数组
*@param[in]    sdwLen   数组长度
*@retval
*@note
* 
*@since    2014-9-5 16:19
*@attention
* 
*/
GPublic void utest_run(UItem_t aItems[], Int32_t sdwLen)
{
    UItemP_t pItem = NULL;
    Int32_t sdwId = 0;

    while (TRUE)
    {
        uitem_print_array(aItems, sdwLen);

        printf("%s", "请输入用例号: ");

        if (0 == scanf("%d", &sdwId))
        {
            fflush(stdin);
            continue;
        }

        pItem = uitem_get_by_id(aItems, sdwLen, sdwId);

        if (pItem != NULL)
        {
            if (NULL == pItem->pfnRun)
            {
                return;
            }

            pItem->pfnRun();
        }
    }
}
