//  tuz_enc.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_h
#define _tuz_enc_h
#include "tuz_enc_types.h"
#ifdef __cplusplus
extern "C" {
#endif

extern const tuz_TCompressProps tuz_kDefaultCompressProps;

hpatch_StreamPos_t tuz_maxCompressedSize(hpatch_StreamPos_t data_size);

hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props);

#ifdef __cplusplus
}
#endif
#endif //_tuz_enc_h
