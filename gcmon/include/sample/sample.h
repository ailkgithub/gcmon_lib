/*!**************************************************************
 *@file sample.h
 *@brief    对PerfMemory的指定项进行数据采样的对外接口声明
 *@author   zhaohm3
 *@date 2014-9-20 20:12
 *@note
 * 
 ****************************************************************/

#ifndef _sample_h__
#define _sample_h__

#include "share/share.h"
#include "rbtree/rbtree.h"

GPublic Double_t s_ticks();
GPublic Double_t s_frequency();
GPublic Double_t s_timstamp();
GPublic Double_t s_s0c();
GPublic Double_t s_s1c();
GPublic Double_t s_s0u();
GPublic Double_t s_s1u();
GPublic Double_t s_ec();
GPublic Double_t s_eu();
GPublic Double_t s_oc();
GPublic Double_t s_ou();
GPublic Double_t s_pc();
GPublic Double_t s_pu();
GPublic Double_t s_s0f();
GPublic Double_t s_s1f();
GPublic Double_t s_ef();
GPublic Double_t s_of();
GPublic Double_t s_pf();
GPublic Double_t s_s0p();
GPublic Double_t s_s1p();
GPublic Double_t s_ep();
GPublic Double_t s_op();
GPublic Double_t s_pp();
GPublic jlong s_ygc();
GPublic jlong s_fgc();
GPublic Double_t s_ygct();
GPublic Double_t s_aygct();
GPublic Double_t s_fgct();
GPublic Double_t s_afgct();
GPublic Double_t s_gct();
GPublic Double_t s_ngcmn();
GPublic Double_t s_ngcmx();
GPublic Double_t s_ngc();
GPublic Double_t s_ogcmn();
GPublic Double_t s_ogcmx();
GPublic Double_t s_ogc();
GPublic Double_t s_pgcmn();
GPublic Double_t s_pgcmx();
GPublic Double_t s_pgc();
GPublic Double_t s_ygctp();
GPublic Double_t s_fgctp();
GPublic Double_t s_gctp();
GPublic void s_out_row(FILE *pFile, String_t szContext);
GPublic void s_out_line(FILE *pFile, String_t szContext);

GPublic void sample_init(RBTreeP_t pTree);
GPublic void sample_doit(String_t szContext);

#endif