/*!**************************************************************
 *@file utest.cpp
 *@brief    ��Ԫ���Կ����ؽӿ�ʵ��
 *@author   zhaohm3
 *@date 2014-9-5 16:03
 *@note
 * 
 ****************************************************************/

#include "utest.h"


/*!
*@brief        ��ӡ����������
*@author       zhaohm3
*@param[in]    pItem    ����ӡ�Ĳ�����
*@retval
*@note
* 
*@since    2014-9-5 16:05
*@attention
* 
*/
GPrivate void uitem_print_single(UItemP_t pItem)
{
    printf("�����ţ�%d\t%s\n\n", pItem->sdwId, pItem->szDescribe);
}

/*!
*@brief        ��ӡ����������
*@author       zhaohm3
*@param[in]    aItems   ����ӡ�Ĳ���������
*@param[in]    sdwLen   ����Ԫ�ظ���
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
*@brief        ͨ��ID�Ż�ȡ�����������е���
*@author       zhaohm3
*@param[in]    aItems   ����������
*@param[in]    sdwLen   ���鳤��
*@param[in]    sdwId    ��Ҫ��ȡ�Ĳ�����ID��
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
*@brief        �����û�����������ָ���Ĳ�����
*@author       zhaohm3
*@param[in]    aItems   ��Ԫ����������
*@param[in]    sdwLen   ���鳤��
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

        printf("%s", "������������: ");

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
