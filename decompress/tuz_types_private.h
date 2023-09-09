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
        tuz_ctrlType_literalLine=1,
        tuz_ctrlType_clipEnd=2,
        tuz_ctrlType_streamEnd=3,
    } tuz_TCtrlType;

    #define tuz_kMinLiteralLen      15
    #define tuz_kMinDictMatchLen    2
    #define tuz_kMaxTypeBitCount    8
    #define tuz_kBigPosForLen       ((1<<11)+(1<<9)+(1<<7)-1)

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_private_h
