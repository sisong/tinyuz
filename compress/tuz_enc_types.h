//  tuz_enc_types.h
/*
 Copyright (c) 2012-2020 HouSisong All Rights Reserved.
 (The MIT License)
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
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
        size_t      maxSaveLength;   // >=255 & <64k;    default 64k-1;  1023,16k-1 ...
        int         threadNum;       // >=1;             default 1;
        tuz_byte    minDictMatchLen; // >=2;             default 4;      3,5,6,7,8,...
        tuz_BOOL    isLite; // tuz_FALSE
    } tuz_TCompressProps;
    
    const size_t    tuz_kMinOfMaxSaveLength = 255;
    const size_t    tuz_kMaxOfMaxSaveLength = 1024*64-1;
    const size_t    tuz_kMaxOfDictSize = 1024*1024*16-1;

#ifdef __cplusplus
}
#endif
#endif //_tuz_enc_types_h
