//  tuz_enc.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_enc.h"
#include "tuz_enc_private/tuz_enc_clip.h"
#include <algorithm> //std::max
using namespace _tuz_private;

#define tuz_kDefaultDictSize  (1<<24)

const tuz_TCompressProps tuz_kDefaultCompressProps={tuz_kDefaultDictSize,tuz_kMaxOfMaxSaveLength,1,
                                                    tuz_isNeedLiteralLine?true:false};

static const size_t   kMaxPackedPosByteSize =sizeof(hpatch_StreamPos_t)*3/2+1;

hpatch_StreamPos_t tuz_maxCompressedSize(hpatch_StreamPos_t data_size){
    const hpatch_StreamPos_t u_size=data_size/8+1;
    hpatch_StreamPos_t c_count=data_size/kMinBestClipSize+1;
    return data_size + u_size + 1+kMaxPackedPosByteSize + 4*c_count + 4;
}

inline void _flush_code(const hpatch_TStreamOutput* out_code,hpatch_StreamPos_t& cur_out_pos,std::vector<tuz_byte>& code){
    checkv(out_code->write(out_code,cur_out_pos,code.data(),code.data()+code.size()));
    cur_out_pos+=code.size();
    code.clear();
}

hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props){
    checkv(out_code&&(out_code->write));
    checkv(data&&(data->read));
    if (props){
        checkv((props->dictSize>=1)&(props->dictSize<=tuz_kMaxOfDictSize));
        checkv(props->dictSize==(tuz_size_t)props->dictSize);
        checkv(props->maxSaveLength==(tuz_length_t)props->maxSaveLength);
        checkv((props->maxSaveLength>=tuz_kMinOfMaxSaveLength)&&(props->maxSaveLength<=tuz_kMaxOfMaxSaveLength));
    }
    
    tuz_TCompressProps selfProps=(props)?*props:tuz_kDefaultCompressProps;
    if (selfProps.dictSize>data->streamSize){
        selfProps.dictSize=(size_t)(data->streamSize);
        if (selfProps.dictSize==0)
            selfProps.dictSize=1;
    }
    
    hpatch_StreamPos_t cur_out_pos=0;
    std::vector<tuz_byte> code;
    {//head
        assert(code.empty());
        TTuzCode coder(code,selfProps.isNeedLiteralLine);
		checkv(selfProps.dictSize==(tuz_size_t)selfProps.dictSize);
        checkv(selfProps.maxSaveLength==(tuz_length_t)selfProps.maxSaveLength);
        coder.outDictSize(selfProps.dictSize);
        _flush_code(out_code,cur_out_pos,code);
    }
    size_t curDictSizeMax=tuz_kMinOfDictSize;
    {
        hpatch_StreamPos_t clipSize;
        {
            clipSize=((hpatch_StreamPos_t)selfProps.dictSize+1)/3;
            if (clipSize<kMinBestClipSize) clipSize=kMinBestClipSize;
            if (clipSize>kMaxBestClipSize) clipSize=kMaxBestClipSize;
            hpatch_StreamPos_t clipCount=(data->streamSize+clipSize)/clipSize;
            clipSize=(data->streamSize+clipCount-1)/clipCount;
        }
        
        hdiff_private::TAutoMem dict_buf;
        for (hpatch_StreamPos_t clipBegin=0;true;clipBegin+=clipSize) {
            hpatch_StreamPos_t clipEnd=clipBegin+clipSize;
            bool isToStreamEnd=(clipEnd>=data->streamSize);
            if (isToStreamEnd) clipEnd=data->streamSize;

            assert(code.empty());
            TTuzCode coder(code,selfProps.isNeedLiteralLine);
            if (clipBegin<clipEnd){
                compress_clip(coder,data,clipBegin,clipEnd,selfProps,dict_buf);
            }
            if (!isToStreamEnd)
                coder.outCtrl_clipEnd();
            else
                coder.outCtrl_streamEnd();
            
            curDictSizeMax=std::max(curDictSizeMax,coder.getCurDictSizeMax());
            _flush_code(out_code,cur_out_pos,code);
            if (isToStreamEnd) break;
        }
    }

    {//update dictSize
        checkv(curDictSizeMax<=selfProps.dictSize);
        if (curDictSizeMax<selfProps.dictSize){
            assert(code.empty());
            TTuzCode coder(code,selfProps.isNeedLiteralLine);
            coder.outDictSize(curDictSizeMax);
            hpatch_StreamPos_t dict_out_pos=0;
            _flush_code(out_code,dict_out_pos,code);
        }
    }

    return cur_out_pos;
}

