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
                   hpatch_StreamPos_t clipEnd,const tuz_TCompressProps& props,TDictBuf& dict_buf){
    //    [           |clipBegin       clipEnd|                   ]
    //     |   dict   |
    //                           |  new dict  |
    std::vector<tuz_byte>& data_buf(dict_buf.dictBuf);
    hpatch_StreamPos_t dictBeginPos;
    const size_t dictSizeBack=data_buf.size();
    {
        dictBeginPos=(clipBegin<=props.dictSize)?0:(clipBegin-props.dictSize);
        hpatch_StreamPos_t _mem_size=clipEnd-dictBeginPos;
        checkv(_mem_size==(size_t)_mem_size);
        checkv(_mem_size>=data_buf.size());
        data_buf.resize((size_t)_mem_size);
    }
    {//read data
        hpatch_StreamPos_t readPos=dictBeginPos;
        if (dict_buf.dictEndPos>readPos){
            checkv(dict_buf.dictEndPos<=clipBegin);
            size_t movLen=(size_t)(dict_buf.dictEndPos-readPos);
            checkv(dictSizeBack>=movLen);
            if (dictSizeBack-movLen>0)
                memmove(data_buf.data(),data_buf.data()+dictSizeBack-movLen,movLen);
            readPos=dict_buf.dictEndPos;
        }
        checkv(data->read(data,readPos,data_buf.data()+(size_t)(readPos-dictBeginPos),data_buf.data()+data_buf.size()));
    }
    
    TMatch   matcher(data_buf.data(),data_buf.data()+data_buf.size(),coder,props);
    {//match loop
        const tuz_byte* end=data_buf.data()+data_buf.size();
        const tuz_byte* cur=end-(clipEnd-clipBegin);
        const tuz_byte* back=cur;
        while (cur!=end){
            const tuz_byte*     matched;
            size_t              match_len;
            if (matcher.match(&matched,&match_len,cur)){
                checkv(matched<cur);
                checkv(matched>=data_buf.data());
                checkv(cur+match_len<=end);
                checkv(match_len>=tuz_kMinDictMatchLen);
                checkv(match_len<=props.maxSaveLength);
                const size_t unmatched_len=(cur-back);
                if (unmatched_len>0)
                    _outData(back,unmatched_len,coder,props);
                size_t dict_pos=(cur-matched)-1;
                checkv(dict_pos<props.dictSize);
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
    
    { //update dict
        size_t newDictSize=(props.dictSize<=clipEnd)?props.dictSize:(size_t)clipEnd;
        checkv(data_buf.size()>=newDictSize);
        dict_buf.dictEndPos=clipEnd;
        if (data_buf.size()>newDictSize){
            memmove(data_buf.data(),data_buf.data()+(data_buf.size()-newDictSize),newDictSize);
            data_buf.resize(newDictSize);
        }
    }
}

}
