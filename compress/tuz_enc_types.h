//  tuz_enc_types.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_types_h
#define _tuz_enc_types_h
#include "../decompress/tuz_types.h"
#include "../../HDiffPatch/libHDiffPatch/HPatch/patch_types.h"
#include <stdint.h> //for int type
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct tuz_TCompressProps{
        //memory requires for decompress: kCodeCacheSize + dictSize
        size_t      dictSize;        // >=1 & <=16m-1;   default 64k-1;  220,255,1k,4k,64k-1,1m,...
        size_t      maxSaveLength;   // >=127 & <64k;    default 64k-1;  1023,16k-1 ...
        size_t      threadNum;       // default 1;
    } tuz_TCompressProps;
    
    static const size_t tuz_kMinOfMaxSaveLength = 127;
    static const size_t tuz_kMaxOfMaxSaveLength = 1024*64-1;
    static const size_t tuz_kMaxOfDictSize      = 1024*1024*16-1;

#ifdef __cplusplus
}
#endif
#endif //_tuz_enc_types_h
