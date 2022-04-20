//  tuz_enc_types_private.h
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
#ifndef _tuz_enc_types_private_h
#define _tuz_enc_types_private_h
#include "../tuz_enc_types.h"
#include "../../decompress/tuz_types_private.h"
#include "../../../HDiffPatch/libHDiffPatch/HDiff/private_diff/mem_buf.h"
#include <vector>
#include <stdexcept>

#define check(value,info) do { if (!(value)) { throw std::runtime_error(info); } } while(0)
#define checkv(value)     do { check(value,"check "#value" error!"); } while(0)

namespace _tuz_private{

    static const size_t   kMinBestClipSize = 1024*128;
    static const size_t   kMaxBestClipSize = tuz_kMaxOfDictSize *2;

    static const uint32_t tuz_ui2G_sub_1=(~(uint32_t)0)>>1;
    #define _uint_is_less_2g(v) ((v)<=tuz_ui2G_sub_1)  // < 2G ?
    
    struct ICode{
        virtual size_t getSavedDataBit(tuz_length_t data_len)const=0;
        virtual size_t getSavedDictLenBit(tuz_length_t match_len)const=0;
        virtual size_t getSavedDictPosBit(tuz_size_t pos)const=0;
    };

}
#endif //_tuz_enc_types_private_h
