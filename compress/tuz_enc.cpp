//  tuz_enc.cpp
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
#include "tuz_enc.h"
#include "tuz_enc_private/tuz_enc_clip.h"
using namespace _tuz_private;

const tuz_TCompressProps tuz_kDefaultCompressProps={1024*32-1,1024*32-1,4,1};

hpatch_StreamPos_t tuz_maxCompressedSize(hpatch_StreamPos_t data_size){
    const hpatch_StreamPos_t _u_cout=(data_size+tuz_kMinOfMaxSaveLength-1)/tuz_kMinOfMaxSaveLength;
    const hpatch_StreamPos_t u_size=(_u_cout+7)/8 + (_u_cout*3+1)/2;
    hpatch_StreamPos_t c_count=(data_size+kMinBestClipSize-1)/kMinBestClipSize+1;
    return data_size+ 1+kMaxPackedLenByteSize + (1+2)*c_count + u_size +1+2;
}

hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props){
    assert(out_code&&(out_code->write));
    assert(data&&(data->read));
    if (props){
        checkv((props->dictSize>=1)&(props->dictSize<=1024*1024*16));
        checkv(props->dictSize==(tuz_dict_size_t)props->dictSize);
        checkv((props->maxSaveLength>=255)&(_uint_is_less_2g(props->maxSaveLength)));
        checkv(props->minDictMatchLen>=3);
    }
    
    tuz_TCompressProps selfProps=(props)?*props:tuz_kDefaultCompressProps;
    if (selfProps.dictSize>data->streamSize){
        selfProps.dictSize=(tuz_length_t)(data->streamSize);
        if (selfProps.dictSize==0)
            selfProps.dictSize=1;
    }
    
    hpatch_StreamPos_t cur_out_pos=0;
    std::vector<tuz_byte> code;
    {//head
        TTuzCode coder(code);
        coder.outLen(selfProps.minDictMatchLen);
        coder.outDictPos(selfProps.dictSize);
    }
    {
        hpatch_StreamPos_t clipSize=selfProps.dictSize*16;
        if (clipSize<kMinBestClipSize) clipSize=kMinBestClipSize;
        if (clipSize>-kMaxBestClipSize) clipSize=kMaxBestClipSize;
        hpatch_StreamPos_t clipCount=(data->streamSize+clipSize)/clipSize;
        clipSize=(data->streamSize+clipCount-1)/clipCount;
        
        for (hpatch_StreamPos_t clipBegin=0;true;clipBegin+=clipSize) {
            hpatch_StreamPos_t clipEnd=clipBegin+clipSize;
            bool isToStreamEnd=(clipEnd>=data->streamSize);
            if (isToStreamEnd) clipEnd=data->streamSize;

            TTuzCode coder(code);
            if (clipBegin<clipEnd)
                compress_clip(coder,data,clipBegin,clipEnd,selfProps);
            if (!isToStreamEnd)
                coder.outCtrl_clipEnd();
            else
                coder.outCtrl_streamEnd();
            
            checkv(out_code->write(out_code,cur_out_pos,code.data(),code.data()+code.size()));
            cur_out_pos+=code.size();
            code.clear();
            if (isToStreamEnd) break;
        }
    }
    return cur_out_pos;
}

