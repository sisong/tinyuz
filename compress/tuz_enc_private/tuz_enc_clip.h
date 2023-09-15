//  tuz_enc_clip.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_clip_h
#define _tuz_enc_clip_h
#include "tuz_enc_code.h"
namespace _tuz_private{
    
    struct TDictBuf{
        hpatch_StreamPos_t      dictEndPos;
        std::vector<tuz_byte>   dictBuf;
        inline TDictBuf():dictEndPos(0){}
    };

    void compress_clip(TTuzCode& out_code,const hpatch_TStreamInput* data,hpatch_StreamPos_t clipBegin,
                       hpatch_StreamPos_t clipEnd,const tuz_TCompressProps& props,TDictBuf& dict_buf);
    
}
#endif //_tuz_enc_clip_h
