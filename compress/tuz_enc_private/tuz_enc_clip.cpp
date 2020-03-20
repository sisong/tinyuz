//  tuz_enc_clip.cpp
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
#include "tuz_enc_clip.h"
#include "tuz_enc_match.h"
using namespace hdiff_private;
namespace _tuz_private{
    
void compress_clip(TTuzCode& coder,const hpatch_TStreamInput* data,hpatch_StreamPos_t clipBegin,
                       hpatch_StreamPos_t clipEnd,const tuz_TCompressProps& props){
    size_t mem_size;
    {
        hpatch_StreamPos_t _mem_size=props.dictSize+(clipEnd-clipBegin);
        mem_size=(size_t)_mem_size;
        checkv(mem_size==_mem_size);
    }
    TAutoMem data_buf(mem_size);
    { //    [    |clipBegin      endPos|
        if (clipBegin<props.dictSize){
            size_t empty_size=props.dictSize-clipBegin;
            memset(data_buf.data(),0,empty_size);
            checkv(data->read(data,0,data_buf.data()+empty_size,data_buf.data_end()));
        }else{
            checkv(data->read(data,clipBegin-props.dictSize,data_buf.data(),data_buf.data_end()));
        }
    }
    
    TMatch   matcher(data_buf.data(),data_buf.data_end(),coder,props);
    {//match loop
        const tuz_byte* cur=data_buf.data()+props.dictSize;
        const tuz_byte* back=cur;
        while (cur!=data_buf.data_end()){
            const tuz_byte*     matched;
            tuz_length_t        match_len;
            if (matcher.match(&matched,&match_len)){
                assert(matched<cur);
                assert(match_len<=props.maxSaveLength);
                if (cur!=back){
                    size_t data_len=cur-back;
                    while(data_len) {
                        tuz_length_t len=(data_len<=props.maxSaveLength)?(tuz_length_t)data_len:props.maxSaveLength;
                        coder.outData(len,back,back+len);
                        back+=len;
                        data_len-=len;
                    }
                }
                size_t dict_pos=cur-matched;
                assert(dict_pos<=props.dictSize);
                coder.outDict(match_len-props.minDictMatchLen,(tuz_length_t)dict_pos);
                back=cur+match_len;
            }else{
                ++cur;
            }
        }
    }
}

}
