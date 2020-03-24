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
    
    static void _outData(const tuz_byte* back,size_t unmatched_len,
                         _tuz_private::TTuzCode& coder,const tuz_TCompressProps& props){
        while (unmatched_len){
            tuz_length_t len=(unmatched_len<=props.maxSaveLength)?(tuz_length_t)unmatched_len:props.maxSaveLength;
            coder.outData(back,back+len);
            back+=len;
            unmatched_len-=len;
        }
    }
    
void compress_clip(TTuzCode& coder,const hpatch_TStreamInput* data,hpatch_StreamPos_t clipBegin,
                   hpatch_StreamPos_t clipEnd,const tuz_TCompressProps& props){
    //            [           |clipBegin       endPos|
    //       |     dict       |
    //               | dict   |
    size_t mem_size;
    {
        hpatch_StreamPos_t _mem_size;
        if (props.dictSize>=clipBegin)
            _mem_size=clipEnd;
        else
            _mem_size=props.dictSize+(clipEnd-clipBegin);
        mem_size=(size_t)_mem_size;
        checkv(mem_size==_mem_size);
    }
    TAutoMem data_buf(mem_size);
    {
        checkv(data->read(data,clipEnd-mem_size,data_buf.data(),data_buf.data_end()));
    }
    
    TMatch   matcher(data_buf.data(),data_buf.data_end(),coder,props);
    {//match loop
        const tuz_byte* end=data_buf.data_end();
        const tuz_byte* cur=end-(clipEnd-clipBegin);
        const tuz_byte* back=cur;
        while (cur!=end){
            const tuz_byte*     matched;
            tuz_length_t        match_len;
            size_t              unmatched_len=(cur-back);
            if (matcher.match(&matched,&match_len,cur,unmatched_len)){
                assert(matched<cur);
                assert(cur+match_len<=end);
                assert(match_len<=props.maxSaveLength);
                if (unmatched_len>0)
                    _outData(back,unmatched_len,coder,props);
                size_t dict_pos=(cur-matched)-1;
                assert(dict_pos<props.dictSize);
                coder.outDict(match_len-props.minDictMatchLen,(tuz_length_t)dict_pos);
                cur+=match_len;
                back=cur;
            }else{
                ++cur;
            }
        }
        size_t unmatched_len=(cur-back);
        if (unmatched_len>0)
            _outData(back,unmatched_len,coder,props);
    }
}

}
