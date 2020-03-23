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
#include "tuz_enc_private/tuz_types_private.h"
using namespace _tuz_private;

void tuz_defaultCompressProps(tuz_TCompressProps* out_props){
    out_props->dictSize=1024*32-1;
    out_props->maxSaveLength=1024*32-1;
    out_props->minDictMatchLen=4;
    out_props->threadNum=1;
}

hpatch_StreamPos_t tuz_maxCompressedSize(hpatch_StreamPos_t data_size){
    const hpatch_StreamPos_t _u_cout=(data_size+tuz_kMinSaveLength-1)/tuz_kMinSaveLength;
    const hpatch_StreamPos_t u_size=(_u_cout+7)/8 + (_u_cout*3+1)/2;
    hpatch_StreamPos_t c_count=(data_size+kMinClipLength-1)/kMinClipLength;
    return data_size+ 1 + kMaxPackedLenByteSize + (1+2)*c_count + u_size +1+2;
}

hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props){
    assert(out_code&&(out_code->write));
    assert(data&&(data->read));
    assert(props);
    checkv((props->dictSize>=1)&(props->dictSize<=16*1024*1024));
    checkv(props->dictSize==(tuz_dict_size_t)props->dictSize);
    checkv((props->maxSaveLength>=255)&(_uint_is_less_2g(props->maxSaveLength)));
    checkv(props->minDictMatchLen>=3);
    
    tuz_TCompressProps selfProps=*props;
    if (selfProps.dictSize>data->streamSize){
        selfProps.dictSize=(tuz_length_t)(data->streamSize);
        if (selfProps.dictSize==0)
            selfProps.dictSize=1;
    }
    
    std::vector<tuz_byte> code;
    {//head
        TTuzCode coder(code);
        coder.outLen(selfProps.minDictMatchLen);
        coder.outLen(selfProps.dictSize);
    }
    {
        //todo: clip streamSize
        TTuzCode coder(code);
        hpatch_StreamPos_t clipEnd=data->streamSize;
        compress_clip(coder,data,0,clipEnd,selfProps);
        //end tag
        if (clipEnd==data->streamSize)
            coder.outCtrl_streamEnd();
        else
            coder.outCtrl_clipEnd();
    }
    checkv(out_code->write(out_code,0,code.data(),code.data()+code.size()));
    return code.size();
}

