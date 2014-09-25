/*!**************************************************************
 *@file perf.c
 *@brief    ����Hotspot PerfMemory
 *@author   zhaohm3
 *@date 2014-9-11 15:03
 *@note
 * 
 ****************************************************************/

#include "perf/perf.h"
#include "os/os.h"

//! ���Hotspot��PerfDataEntry�����ݱ仯
#define PERFDATA_MAJOR_VERSION 2
#define PERFDATA_MINOR_VERSION 0

//! �ֽ���洢����
#define PERFDATA_BIG_ENDIAN     0
#define PERFDATA_LITTLE_ENDIAN  1

//! Hotspotԭ��PerfDataPrologue�ṹ����
typedef struct PerfDataPrologue PerfDataPrologue_t, *PerfDataPrologueP_t;
struct PerfDataPrologue{
    jint   magic;                       //!< magic number - 0xcafec0c0
    jbyte  byte_order;                  //!< byte order of the buffer
    jbyte  major_version;               //!< major version number
    jbyte  minor_version;               //!< minor version number
    jbyte  accessible;                  //!< ready to access
    jint   used;                        //!< number of PerfData memory bytes used
    jint   overflow;                    //!< number of bytes of overflow
    jlong  mod_time_stamp;              //!< time stamp of last structural modification
    jint   entry_offset;                //!< offset of the first PerfDataEntry
    jint   num_entries;                 //!< number of allocated PerfData entries
};

//! Hotspotԭ��PerfDataEntry�ṹ����
typedef struct PerfDataEntry PerfDataEntry_t, *PerfDataEntryP_t;
struct PerfDataEntry {
    jint entry_length;                  //!< entry length in bytes
    jint name_offset;                   //!< offset of the data item name
    jint vector_length;                 //!< length of the vector. If 0, then scalar
    jbyte data_type;                    //!< type of the data item -
                                        //!< 'B','Z','J','I','S','C','D','F','V','L','['
    jbyte flags;                        //!< flags indicating misc attributes
    jbyte data_units;                   //!< unit of measure for the data type
    jbyte data_variability;             //!< variability classification of data type
    jint  data_offset;                  //!< offset of the data item
};

//! ͨ��PerfDataEntry����PerfDataItem����������
struct PerfDataItem
{
    jint dwVecLength;                   //!< PerfDataEntry.vector_length
    jbyte byType;                       //!< PerfDataEntry.data_type
    jbyte byFlags;                      //!< PerfDataEntry.flags
    jbyte byDataUnits;                  //!< PerfDataEntry.data_units
    jbyte byDataVariability;            //!< PerfDataEntry.data_variability
    String_t szName;                    //!< PerfDataEntry + name_offset
    union                               //!< PerfDataEntry + data_offset
    {
        jlong *pdwValue;                //!< jlong data
        String_t szValue;               //!< jbyte data
        Addr_t pValue;                  //!< other data
    };
};

#define PDI_SIZE sizeof(PerfDataItem_t)

//! ��vm/agent/sun/jvm/hotspot/runtime/BasicType.java�еĶ��屣��ͬ��
typedef enum BasicType BasicType_t, *BasicTypeP_t;
enum BasicType {
    T_BOOLEAN = 4,
    T_CHAR = 5,
    T_FLOAT = 6,
    T_DOUBLE = 7,
    T_BYTE = 8,
    T_SHORT = 9,
    T_INT = 10,
    T_LONG = 11,
    T_OBJECT = 12,
    T_ARRAY = 13,
    T_VOID = 14,
    T_ADDRESS = 15,
    T_NARROWOOP = 16,
    T_CONFLICT = 17,
    T_ILLEGAL = 99
};

/*!
 * ��Java��com.sun.hotspot.perfdata.Variability����ͬ��
 * ���ڸ�PerfDataEntry.data_variability��ֵ
 */
typedef enum Variability Variability_t, *VariabilityP_t;
enum Variability {
    V_Constant = 1,
    V_Monotonic = 2,
    V_Variable = 3,
    V_last = V_Variable
};

/*!
 * ��Java��com.sun.hotspot.perfdata.Units����ͬ��
 * ���ڸ�PerfDataEntry.data_units��ֵ
 */
typedef enum Units Units_t, *UnitsP_t;
enum Units {
    U_None = 1,
    U_Bytes = 2,
    U_Ticks = 3,
    U_Events = 4,
    U_String = 5,
    U_Hertz = 6,
    U_Last = U_Hertz
};

//! Miscellaneous flags�����ڸ�PerfDataEntry.flags��ֵ
typedef enum Flags Flags_t, *FlagsP_t;
enum Flags {
    F_None = 0x0,
    F_Supported = 0x1
};

//! BasicType��ǩ����ӳ�䣬���ڸ�PerfDataEntry.data_type��ֵ
GPrivate Char_t gTtype2char[T_CONFLICT + 1] = { 0, 0, 0, 0,
    'Z', 'C', 'F', 'D', 'B', 'S', 'I', 'J', 'L', '[', 'V',
    0, 0, 0 };

//! BasicType��Java������������ӳ��
GPrivate const CharP_t gTtype2name[T_CONFLICT + 1] = {
    NULL, NULL, NULL, NULL,
    "boolean",
    "char",
    "float",
    "double",
    "byte",
    "short",
    "int",
    "long",
    "object",
    "array",
    "void",
    "*address*",
    "*narrowoop*",
    "*conflict*"
};

GPrivate Char_t type2char(BasicType_t t)
{
    return (Uint32_t)t < T_CONFLICT + 1 ? gTtype2char[t] : 0;
}

GPrivate String_t type2name(BasicType_t t)
{
    return (Uint32_t)t < T_CONFLICT + 1 ? gTtype2name[t] : NULL;
}

GPrivate BasicType_t name2type(String_t szName)
{
    Int32_t i = 0;

    for (i = T_BOOLEAN; i <= T_VOID; i++)
    {
        BasicType_t t = (BasicType_t)i;

        if (gTtype2name[i] != NULL && 0 == os_strcmp(gTtype2name[i], szName))
        {
            return t;
        }
    }

    return T_ILLEGAL;
}

GPrivate BasicType_t char2type(Char_t c) {
    switch (c) {
    case 'B': return T_BYTE;
    case 'C': return T_CHAR;
    case 'D': return T_DOUBLE;
    case 'F': return T_FLOAT;
    case 'I': return T_INT;
    case 'J': return T_LONG;
    case 'S': return T_SHORT;
    case 'Z': return T_BOOLEAN;
    case 'V': return T_VOID;
    case 'L': return T_OBJECT;
    case '[': return T_ARRAY;
    }

    return T_ILLEGAL;
}

GPrivate String_t char2name(Char_t c)
{
    return type2name(char2type(c));
}

/*!
*@brief        ����name_offset��ȡPerfDataEntry_t�е�����������
*@author       zhaohm3
*@param[in]    pEntry
*@retval
*@note
* 
*@since    2014-9-17 18:34
*@attention
* 
*/
GPrivate String_t pde_get_name(PerfDataEntryP_t pEntry)
{
    String_t szEntryName = NULL;
    Addr_t pStart = (Addr_t)pEntry;

    GCMON_CHECK_NULL(pEntry, ERROR);
    szEntryName = (String_t)(pStart + pEntry->name_offset);

ERROR:
    return szEntryName;
}

/*!
*@brief        ����data_offset��ȡPerfDataEntry_t�е�����������
*@author       zhaohm3
*@param[in]    pEntry
*@retval
*@note
* 
*@since    2014-9-17 18:34
*@attention
* 
*/
GPrivate Addr_t pde_get_data(PerfDataEntryP_t pEntry)
{
    Addr_t pData = NULL;
    Addr_t pStart = (Addr_t)pEntry;

    GCMON_CHECK_NULL(pEntry, ERROR);
    pData = (Addr_t)(pStart + pEntry->data_offset);

ERROR:
    return pData;
}

/*!
*@brief        ����data_offset��ȡPerfDataEntry_t�е�jlong����������
*@author       zhaohm3
*@param[in]    pEntry
*@retval
*@note
* 
*@since    2014-9-22 17:51
*@attention
* 
*/
GPrivate jlong pde_get_long(PerfDataEntryP_t pEntry)
{
    jlong lwValue = 0;
    Addr_t pData = pde_get_data(pEntry);

    GCMON_CHECK_NULL(pData, ERROR);
    lwValue = *(jlong *)pData;

ERROR:
    return lwValue;
}

/*!
*@brief        ����data_offset��ȡPerfDataEntry_t�е�jbyte����������
*@author       zhaohm3
*@param[in]    pEntry
*@retval
*@note
* 
*@since    2014-9-22 17:51
*@attention
* 
*/
GPrivate String_t pde_get_string(PerfDataEntryP_t pEntry)
{
    return (String_t)pde_get_data(pEntry);
}

/*!
*@brief        ���PerfDataItem_t
*@author       zhaohm3
*@param[in]    pItem
*@retval
*@note
* 
*@since    2014-9-17 18:37
*@attention
* 
*/
GPrivate PerfDataItemP_t pdi_clear(PerfDataItemP_t pItem)
{
    GCMON_CHECK_NULL(pItem, ERROR);
    os_memset(pItem, 0, PDI_SIZE);

ERROR:
    return pItem;
}

/*!
*@brief        ����һ���յ�PerfDataItem_t
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-18 11:00
*@attention
* 
*/
GPrivate PerfDataItemP_t pdi_new()
{
    PerfDataItemP_t pItem = NULL;

    GMALLOC(pItem, PerfDataItem_t, PDI_SIZE);
    GCMON_CHECK_NULL(pItem, ERROR);
    pdi_clear(pItem);

ERROR:
    return pItem;
}

/*!
*@brief        ͨ��PerfDataEntry_t����һ��PerfDataItem_t
*@author       zhaohm3
*@param[in]    pEntry
*@retval
*@note
* 
*@since    2014-9-18 11:00
*@attention
* 
*/
GPrivate PerfDataItemP_t pdi_create(PerfDataEntryP_t pEntry)
{
    PerfDataItemP_t pItem = NULL;
    String_t szName = NULL;
    Addr_t pData = NULL;

    GCMON_CHECK_NULL(pEntry, ERROR);
    pItem = pdi_new();
    GCMON_CHECK_NULL(pItem, ERROR);

    pItem->dwVecLength = pEntry->vector_length;
    pItem->byType = pEntry->data_type;
    pItem->byFlags = pEntry->flags;
    pItem->byDataUnits = pEntry->data_units;
    pItem->byDataVariability = pEntry->data_variability;

    szName = pde_get_name(pEntry);
    pData = pde_get_data(pEntry);

    pItem->szName = szName;

    //! ����Ϊ�����壬����ֱ�ӻ�ȡ����
    switch (pItem->byType)
    {
    case 'B':
        pItem->szValue = (String_t)pData;       //!< �洢jlong������
        break;
    case 'J':
        pItem->pdwValue = (jlong *)pData;       //!< �洢�ַ���
        break;
    default:
        pItem->pValue = pData;                  //!< ������������
        break;
    }

ERROR:
    return pItem;
}

/*!
*@brief        �ͷ�һ��PerfDataItem_t��ռ�õĴ洢�ռ�
*@author       zhaohm3
*@param[in]    pItem    ���ͷŵ�Item��
*@retval
*@note
* 
*@since    2014-9-18 11:03
*@attention
* 
*/
GPrivate void pdi_free(PerfDataItemP_t pItem)
{
    GCMON_CHECK_NULL(pItem, ERROR);
    pdi_clear(pItem);
    GFREE(pItem);

ERROR:
    return;
}

/*!
*@brief        PerfDataItem_t�ȽϺ�����ͨ��szName���бȽ�
*@author       zhaohm3
*@param[in]    pSrc
*@param[in]    pDes
*@retval
*@note
* 
*@since    2014-9-18 11:04
*@attention
* 
*/
GPrivate Int32_t pdi_compare(PerfDataItemP_t pSrc, PerfDataItemP_t pDes)
{
    String_t szSrcName = NULL;
    String_t szDesName = NULL;
    Int32_t sdwCompare = 0;

    GCMON_CHECK_COND(pSrc != NULL && pDes != NULL, ERROR);
    GCMON_CHECK_COND(pSrc->szName != NULL || pDes->szName != NULL, ERROR);

    szSrcName = pSrc->szName;
    szDesName = pDes->szName;

    if (szSrcName != NULL && szDesName != NULL)
    {
        sdwCompare = (Int32_t)os_strcmp(szSrcName, szDesName);
    }
    else if (szSrcName != NULL)
    {
        GASSERT(NULL == szDesName);
        sdwCompare = 1;
    }
    else
    {
        GASSERT(NULL == szSrcName && szDesName != NULL);
        sdwCompare = -1;
    }

ERROR:
    return sdwCompare;
}

/*!
*@brief        ��ȡPerfDataItem_t�д洢��jlongֵ
*@author       zhaohm3
*@param[in]    pItem
*@retval
*@note
* 
*@since    2014-9-18 14:52
*@attention
* 
*/
GPublic jlong pdi_get_jlong(PerfDataItemP_t pItem)
{
    GASSERT(pItem != NULL);
    return *pItem->pdwValue;
}

/*!
*@brief        ��PerfDataItem_t�д洢��jlongֵת����Double_tֵ����
*@author       zhaohm3
*@param[in]    pItem
*@retval
*@note
* 
*@since    2014-9-19 11:28
*@attention
* 
*/
GPublic Double_t pdi_get_double(PerfDataItemP_t pItem)
{
    GASSERT(pItem != NULL);
    return (Double_t)(*pItem->pdwValue);
}

/*!
*@brief        ��ȡPerfDataItem_t�д洢��String_tֵ
*@author       zhaohm3
*@param[in]    pItem
*@retval
*@note
* 
*@since    2014-9-18 14:52
*@attention
* 
*/
GPublic String_t pdi_get_string(PerfDataItemP_t pItem)
{
    GASSERT(pItem != NULL);
    return pItem->szValue;
}

/*!
*@brief        ��ȡPerfDataItem_t�д洢��Addr_tֵ
*@author       zhaohm3
*@param[in]    pItem
*@retval
*@note
* 
*@since    2014-9-18 14:52
*@attention
* 
*/
GPublic Addr_t pdi_get_other(PerfDataItemP_t pItem)
{
    GASSERT(pItem != NULL);
    return pItem->pValue;
}

/*!
*@brief        ͨ��PerfMemory����PerfDataItem_t�ĺ����
*@author       zhaohm3
*@param[in]    pPerfMemory
*@retval
*@note         ����JVM��PerfMemory��ͨ��ÿ��PerfDataEntry_t��
*              ����PerfDataItem_t��Ȼ��pdi���뵽�������
* 
*@since    2014-9-18 11:05
*@attention
* 
*/
GPublic RBTreeP_t pdi_build_tree(Addr_t pPerfMemory)
{
    RBTreeP_t pTree = NULL;
    PerfDataPrologueP_t pPerf = NULL;
    Int32_t i = 0;
    Addr_t pCurr = NULL;

    GCMON_CHECK_NULL(pPerfMemory, ERROR);
    pTree = rbtree_new();
    GCMON_CHECK_NULL(pTree, ERROR);
    rbtree_set_rbd_compare(pTree, pdi_compare);
    rbtree_set_rbd_free(pTree, pdi_free);

    pPerf = (PerfDataPrologueP_t)pPerfMemory;
    pCurr = (Addr_t)(((Addr_t)pPerf) + pPerf->entry_offset);
    GASSERT(pPerf->entry_offset == sizeof(PerfDataPrologue_t));

    for (i = 0; i < pPerf->num_entries; i++)
    {
        PerfDataEntryP_t pEntry = (PerfDataEntryP_t)pCurr;
        PerfDataItemP_t pItem = pdi_create(pEntry);
        rbtree_insert(pTree, (RBDataP_t)pItem, NULL);
        pCurr += pEntry->entry_length;
    }

ERROR:
    return pTree;
}

/*!
*@brief        ͨ��������ַ����ں�����в���PerfDataItem_t
*@author       zhaohm3
*@param[in]    pTree
*@param[in]    szName
*@retval
*@note
* 
*@since    2014-9-18 11:47
*@attention
* 
*/
GPublic PerfDataItemP_t pdi_search_item(RBTreeP_t pTree, String_t szName)
{
    PerfDataItemP_t pItem = NULL;
    PerfDataItem_t item = { 0 };

    GCMON_CHECK_NULL(pTree, ERROR);
    item.szName = szName;
    pItem = rbtree_search(pTree, &item);

ERROR:
    return pItem;
}

/*!
*@brief        ��ӡpPerfMemory�е����ܼ���������ϸ��Ϣ
*@author       zhaohm3
*@param[in]    address
*@retval
*@note
* 
*@since    2014-9-22 17:54
*@attention
* 
*/
GPrivate void perf_print_verbose(Addr_t address)
{
    PerfDataPrologueP_t pPerf = (PerfDataPrologueP_t)address;

    if (pPerf != NULL)
    {
        Int32_t i = 0;
        CharP_t pCurr = (CharP_t)(((CharP_t)(address)) + pPerf->entry_offset);

        GASSERT(pPerf->entry_offset == sizeof(PerfDataPrologue_t));

        for (i = 0; i < pPerf->num_entries; i++)
        {
            PerfDataEntryP_t pEntry = (PerfDataEntryP_t)pCurr;
            String_t szName = pde_get_name(pEntry);

            switch (pEntry->data_type)
            {
            case 'B':
                gcmon_debug_msg("%s --> %s --> %s \n",
                    szName, char2name(pEntry->data_type),
                    pde_get_string(pEntry));
                break;
            case 'J':
                gcmon_debug_msg("%s --> %s --> %lld \n",
                    szName, char2name(pEntry->data_type),
                    pde_get_long(pEntry));
                break;
            default:
                gcmon_debug_msg("%s --> %s \n", szName, char2name(pEntry->data_type));
                break;
            }
            pCurr += pEntry->entry_length;
        }
    }
}
