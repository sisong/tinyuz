//  tuz_types_private.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_types_private_h
#define _tuz_types_private_h
#include "tuz_types.h"
#ifdef __cplusplus
extern "C" {
#endif

    typedef enum tuz_TCodeType{
        tuz_codeType_dict=0,
        tuz_codeType_data=1,
    } tuz_TCodeType;
    
    typedef enum tuz_TCtrlType{
        //0 for error type
    #if tuz_isNeedLiteralLine
        tuz_ctrlType_literalLine=1,
    #endif
        tuz_ctrlType_clipEnd=2,
        tuz_ctrlType_streamEnd=3,
    } tuz_TCtrlType;

    #if tuz_isNeedLiteralLine
    #   define tuz_kMinLiteralLen   15
    #endif
    #define tuz_kMinDictMatchLen    2
    #define tuz_kMaxTypeBitCount    8
    #define tuz_kBigPosForLen       ((1<<11)+(1<<9)+(1<<7)-1)

    tuz_fast_uint8 _tuz_cache_read_1byte(struct _tuz_TInputCache* self);
    tuz_BOOL _tuz_cache_update(struct _tuz_TInputCache* self);

    static tuz_force_inline tuz_BOOL _tuz_cache_success_finish(const _tuz_TInputCache* self){
        return (self->cache_end!=0); }

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_private_h
