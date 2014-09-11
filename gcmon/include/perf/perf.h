/*!**************************************************************
 *@file perf.h
 *@brief    ����Hotspot PerfMemory
 *@author   zhaohm3
 *@date 2014-9-11 15:03
 *@note     ��Ҫ���ڴ���Hotspot�����ܼ���ڴ���
 * 
 ****************************************************************/

#ifndef _perf_h__
#define _perf_h__

#include "share/share.h"

typedef struct tagPerfDataPrologue PerfDataPrologue_t, *PerfDataPrologueP_t;
typedef struct tagPerfDataEntry PerfDataEntry_t, *PerfDataEntryP_t;
typedef enum tagBasicType BasicType_t, *BasicTypeP_t;
typedef enum tagVariability Variability_t, *VariabilityP_t;
typedef enum tagUnits Units_t, *UnitsP_t;
typedef enum tagFlags Flags_t, *FlagsP_t;

#endif