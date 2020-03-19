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
#include "libHDiffPatch/HPatch/patch_types.h"
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct tuz_TCompressProps{
        //memory requires for decompress: kDecodeCacheSize + dictSize
        tuz_size_t      dictSize;        // >=1;  default 16k;    250,1k,4k,64k,1m ...
        tuz_length_t    maxStepLength;   // >=63; default 64k-1;  255,4k,64k-1,1m ...
        tuz_byte        minDictMatchLen; // >=2;  default 2;      3,4,...
        tuz_size_t      threadNum;       // >=1;  default 1;
    } tuz_TCompressProps;

#ifdef __cplusplus
}
#endif
#endif //_tuz_enc_types_h
