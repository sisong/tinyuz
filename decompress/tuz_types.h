//  tuz_types.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_types_h
#define _tuz_types_h
#ifdef NDEBUG
# ifndef assert
#   define  assert(expression) ((void)0)
# endif
#else
#   include <assert.h> //assert
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
#define TINYUZ_VERSION_MAJOR    0
#define TINYUZ_VERSION_MINOR    8
#define TINYUZ_VERSION_RELEASE  1
    
#define _TINYUZ_VERSION                 TINYUZ_VERSION_MAJOR.TINYUZ_VERSION_MINOR.TINYUZ_VERSION_RELEASE
#define _TINYUZ_QUOTE(str)              #str
#define _TINYUZ_EXPAND_AND_QUOTE(str)   _TINYUZ_QUOTE(str)
#define TINYUZ_VERSION_STRING           _TINYUZ_EXPAND_AND_QUOTE(_TINYUZ_VERSION)


#ifndef tuz_byte
    typedef  unsigned char      tuz_byte;
#endif
#ifndef tuz_length_t
    //tuz_length_t must can saved CompressProps.maxSaveLength & dictSize value
    //  if tuz_length_t==uint8_t, must CompressProps.maxSaveLength & dictSize <= 255 when compress;
    //  if tuz_length_t==int16_t, must CompressProps.maxSaveLength & dictSize <= (2<<15)-1 ; ...
    typedef  unsigned int       tuz_length_t;
#endif
#ifndef tuz_size_t
    typedef  tuz_length_t       tuz_size_t; //memory size type
#endif

#ifndef tuz_fast_uint8
    typedef  unsigned int       tuz_fast_uint8; //>= 8bit uint
#endif
#ifndef tuz_BOOL
    typedef  tuz_fast_uint8     tuz_BOOL;
#endif
#define      tuz_FALSE      0
#define      tuz_TRUE       1

#ifndef tuz_inline
#if (defined(_MSC_VER))
#   define tuz_inline __inline
#else
#   define tuz_inline inline
#endif
#endif
#ifndef tuz_force_inline
#if defined(_MSC_VER)
#   define tuz_force_inline __forceinline
#elif defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM)
#   define tuz_force_inline __attribute__((always_inline)) inline
#elif defined(__ICCARM__)
#   define tuz_force_inline _Pragma("inline=forced")
#else
#   define tuz_force_inline tuz_inline
#endif
#endif
    
#if 1
#   define tuz_try_inline
#else
#   define tuz_try_inline tuz_inline
#endif

#ifndef _IS_RUN_MEM_SAFE_CHECK 
#   define _IS_RUN_MEM_SAFE_CHECK  1
#endif

#ifndef _IS_USED_C_MEMCPY  // use memcpy() in <string.h>?
#   define _IS_USED_C_MEMCPY  1
#endif

#ifndef tuz_isNeedLiteralLine  // optimize for can not compress data
#   define tuz_isNeedLiteralLine 1
#endif

#ifndef tuz_kMaxOfDictSize
#   define tuz_kMaxOfDictSize   __tuz_kMaxOfDictSize_MAX
//#   define tuz_kMaxOfDictSize   ((1<<24)-1)   //3 bytes
//#   define tuz_kMaxOfDictSize   ((1<<16)-1)   //2 bytes
#endif

#define __tuz_kMaxOfDictSize_MAX  (1<<30) //now limit for uint32
#if (tuz_kMaxOfDictSize>__tuz_kMaxOfDictSize_MAX)
#   error tuz_kMaxOfDictSize error
#elif ((tuz_kMaxOfDictSize>>24)>=1)
#   define tuz_kDictSizeSavedBytes 4
#elif ((tuz_kMaxOfDictSize>>16)>=1)
#   define tuz_kDictSizeSavedBytes 3
#elif ((tuz_kMaxOfDictSize>>8)>=1)
#   define tuz_kDictSizeSavedBytes 2
#elif (tuz_kMaxOfDictSize>=1)
#   define tuz_kDictSizeSavedBytes 1
#else
#   error tuz_kMaxOfDictSize error
#endif


#ifndef tuz_TInputStreamHandle
    typedef void*   tuz_TInputStreamHandle;
#endif

//read (*data_size) data to out_data from sequence stream; if input stream end,set *data_size readed size; if read error return tuz_FALSE;
typedef tuz_BOOL (*tuz_TInputStream_read)(tuz_TInputStreamHandle inputStream,tuz_byte* out_data,tuz_size_t* data_size);

typedef struct _tuz_TInputCache{
    tuz_size_t      cache_begin;
    tuz_size_t      cache_end;
    tuz_byte*       cache_buf;
    tuz_TInputStreamHandle  inputStream;
    tuz_TInputStream_read   read_code;
} _tuz_TInputCache;
typedef struct _tuz_TDict{
    tuz_size_t      dict_cur;
    tuz_size_t      dict_size;
    tuz_byte*       dict_buf;
} _tuz_TDict;
typedef struct _tuz_TState{
    tuz_size_t      dictType_pos;
    tuz_size_t      dictType_pos_inc;
    tuz_size_t      dict_pos_back;
    tuz_length_t    dictType_len;
  #if tuz_isNeedLiteralLine
    tuz_length_t    literalType_len;
  #endif
    tuz_fast_uint8  types;
    tuz_fast_uint8  type_count;
    tuz_BOOL        isHaveData_back;
} _tuz_TState;

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_h
