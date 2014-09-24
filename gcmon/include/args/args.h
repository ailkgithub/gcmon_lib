/*!**************************************************************
 *@file args.h
 *@brief    �����������صĽӿ�����
 *@author   zhaohm3
 *@date 2014-9-22 9:54
 *@note
 * 
 ****************************************************************/

#ifndef _args_h__
#define _args_h__

#include "share/share.h"
#include "rbtree/rbtree.h"

GPublic String_t args_get_javacmd(RBTreeP_t pPdiTree);
GPublic String_t args_get_vmargs(RBTreeP_t pPdiTree);
GPublic String_t args_get_agentargs();

GPublic Size64_t vmargs_parse_size(String_t szVmArgs, String_t szPrefix, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_InitialHeapSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_MaxHeapSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_NewSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_MaxNewSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_OldSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_PermSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_MaxPermSize(String_t szVmArgs, StringP_t pszArgs);
GPublic Size64_t vmargs_parse_ThreadStackSize(String_t szVmArgs, StringP_t pszArgs);

GPublic String_t agentargs_get_outpath();
GPublic String_t agentargs_get_outname();

#endif