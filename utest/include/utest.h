/*!**************************************************************
 *@file utest.h
 *@brief    ��Ԫ���Կ�ܽӿ�����
 *@author   zhaohm3
 *@date 2014-9-5 15:50
 *@note
 * 
 ****************************************************************/

#ifndef _utest_h__
#define _utest_h__

#include "share/type.h"
#include "share/share.h"

typedef struct tagUItem
{
    Int32_t     sdwId;          //!< ��Ԫ���Ժ���ID��
    Bool32_t    bShow;          //!< �Ƿ���ѡ��˵�����ʾ����
    void (*pfnRun)();          //!< ��Ԫ������ں���
    AnyP_t      pContext;       //!< ��Ԫ��������ʱ�����ģ���ͨ���˲������ݶ������Ϣ
    String_t    szDescribe;     //!< ������Ϣ�������ṩ���û�ѡ��
} UItem_t, *UItemP_t;

//! �����ṩ�ĵ�Ԫ������ں���
GPublic void utest_run(UItem_t aItems[], Int32_t sdwLen);

//! ��ȡ���鳤��
#define ARRAY_SIZE(a) ((sizeof(a))/(sizeof((a)[0])))

#endif
