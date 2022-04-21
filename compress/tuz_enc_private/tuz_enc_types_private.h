//  tuz_enc_types_private.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
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
