//  tuz_enc_clip.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_enc_clip.h"
#include "tuz_enc_match.h"
using namespace hdiff_private;
namespace _tuz_private{
    
    static void _outData(const tuz_byte* back,size_t unmatched_len,
                         _tuz_private::TTuzCode& coder,const tuz_TCompressProps& props){
        while (unmatched_len){
            size_t len=(unmatched_len<=props.maxSaveLength)?unmatched_len:props.maxSaveLength;
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
            size_t              match_len;
            if (matcher.match(&matched,&match_len,cur)){
                assert(matched<cur);
                assert(cur+match_len<=end);
                assert(match_len>=tuz_kMinDictMatchLen);
                assert(match_len<=props.maxSaveLength);
                const size_t unmatched_len=(cur-back);
                if (unmatched_len>0)
                    _outData(back,unmatched_len,coder,props);
                size_t dict_pos=(cur-matched)-1;
                assert(dict_pos<props.dictSize);
                coder.outDict(match_len,dict_pos);
                cur+=match_len;
                back=cur;
            }else{
                ++cur;
            }
        }
        const size_t unmatched_len=(cur-back);
        if (unmatched_len>0)
            _outData(back,unmatched_len,coder,props);
    }
}

}
