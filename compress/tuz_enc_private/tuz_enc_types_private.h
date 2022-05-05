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

    static const size_t   kMinBestClipSize = 1024*256;
    static const size_t   kMaxBestClipSize = (size_t)(tuz_kMaxOfDictSize+1)/2;

}
#endif //_tuz_enc_types_private_h
