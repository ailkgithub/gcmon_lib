/*!**************************************************************
 *@file perf.h
 *@brief    解析Hotspot PerfMemory
 *@author   zhaohm3
 *@date 2014-9-11 15:03
 *@note     主要用于处理Hotspot的性能监控内存区
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