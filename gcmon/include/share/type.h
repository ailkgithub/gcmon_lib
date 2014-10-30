/*!**************************************************************
 *@file type.h
 *@brief    跨平台类型定义
 *@author   zhaohm3
 *@date 2014-9-3 18:19
 *@note
 * 
 ****************************************************************/

#ifndef _type_h__
#define _type_h__

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#define FMTF "%.3f"

#ifdef WIN32
#define FMTL "%I64d"
#elif defined(LINUX) || defined(SOLARIS)
#define FMTL "%lld"
#else
#error UnsupportedSystem
#endif

typedef signed char     Int8_t, *Int8P_t;           //!< sby
typedef signed short    Int16_t, *Int16P_t;         //!< sw
typedef signed int      Int32_t, *Int32P_t;         //!< sdw

#ifdef WIN32
typedef signed __int64  Int64_t, *Int64P_t;         //!< slw
#elif defined(LINUX) || defined(SOLARIS)
typedef signed long long int Int64_t, *Int64P_t;    //!< slw
#else
#error UnsupportedSystem
#endif

typedef unsigned char   Uint8_t, *Uint8P_t;         //!< uby
typedef unsigned short  Uint16_t, *Uint16P_t;       //!< uw
typedef unsigned int    Uint32_t, *Uint32P_t;       //!< udw

#ifdef WIN32
typedef unsigned __int64 Uint64_t, *Uint64P_t;      //!< ulw
#elif defined(LINUX) || defined(SOLARIS)
typedef unsigned long long int Uint64_t, *Uint64P_t;    //!< ulw
#else
#error UnsupportedSystem
#endif

#ifdef OS_64BIT
typedef Int64_t     Int_t, *IntP_t;                 //!< dw
typedef Uint64_t    Uint_t, *UintP_t;               //!< udw
#else
typedef Int32_t     Int_t, *IntP_t;                 //!< dw
typedef Uint32_t    Uint_t, *UintP_t;               //!< udw
#endif

typedef Uint8_t     Bool8_t, *Bool8P_t;             //!< b
typedef Uint16_t    Bool16_t, *Bool16P_t;           //!< b
typedef Uint32_t    Bool32_t, *Bool32P_t;           //!< b
typedef Uint64_t    Bool64_t, *Bool64P_t;           //!< b
typedef Uint_t      Bool_t, *BoolP_t;               //!< b

typedef Uint8_t     Count8_t, *Count8P_t;           //!< uby
typedef Uint16_t    Count16_t, *Count16P_t;         //!< uw
typedef Uint32_t    Count32_t, *Count32P_t;         //!< udw
typedef Uint64_t    Count64_t, *Count64P_t;         //!< ulw
typedef Uint_t      Count_t, *CountP_t;             //!< udw

typedef Uint8_t     Size8_t, *Size8P_t;             //!< uby
typedef Uint16_t    Size16_t, *Size16P_t;           //!< uw
typedef Uint32_t    Size32_t, *Size32P_t;           //!< udw
typedef Uint64_t    Size64_t, *Size64P_t;           //!< ulw
typedef Uint_t      Size_t, *SizeP_t;               //!< udw

typedef void        *Handle_t, **HandleP_t;         //!< h
typedef char        Char_t, *CharP_t;               //!< c
typedef unsigned char UChar_t, *UcharP_t;           //!< uc
typedef Char_t      *String_t, **StringP_t;         //!< sz
typedef Char_t      *Addr_t, **AddrP_t;             //!< p

typedef float       Float_t, *FloatP_t;             //!< f
typedef double      Double_t, *DoubleP_t;           //!< df

typedef void        Any_t, *AnyP_t;                 //!< p

#endif
