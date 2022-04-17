//  tuz_enc_code.cpp
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
#include "tuz_enc_code.h"
#define _TEST_COUNT     0

#if (_TEST_COUNT)
const int kMCount=256;
long  mdict_count=0;
double  mdict_len_bit[32]={0};
double  mdict_len[kMCount]={0};
double  mdict_pos_bit[32]={0};
double  mdict_pos[kMCount]={0};
long  mdata_count=0;
double  mdata_len_bit[32]={0};
double  mdata_len[kMCount]={0};

inline int get_bit(size_t len) { int result=0;  do {  ++result; len>>=1; }while(len>0); return result; }

struct _debug_log {
    ~_debug_log(){
        for (size_t i=0;i<32;++i){
            mdict_len_bit[i]=mdict_len_bit[i]/mdict_count+((i>0)?mdict_len_bit[i-1]:0);
            mdict_pos_bit[i]=mdict_pos_bit[i]/mdict_count+((i>0)?mdict_pos_bit[i-1]:0);
            mdata_len_bit[i]=mdata_len_bit[i]/mdata_count+((i>0)?mdata_len_bit[i-1]:0);
        }
        for (size_t i=0;i<kMCount;++i){
            mdict_len[i]=mdict_len[i]/mdict_count+((i>0)?mdict_len[i-1]:0);
            mdict_pos[i]=mdict_pos[i]/mdict_count+((i>0)?mdict_pos[i-1]:0);
            mdata_len[i]=mdata_len[i]/mdata_count+((i>0)?mdata_len[i-1]:0);
        }
    }
};
_debug_log _;
#endif

namespace _tuz_private{
    
    //low to high bitmap: xx?xx? xx?xx? ...
    static tuz_inline size_t _pack_v_count(tuz_length_t v){
        size_t count=0;
        do {
            v>>=2;
            ++count;
        }while(v>0);
        return count;
    }
    static tuz_inline size_t _getSavedLenBit(tuz_length_t len){
        return _pack_v_count(len)*3;
    }
    
void TTuzCode::outLen(tuz_length_t len){
    size_t c=_pack_v_count(len);
    while (c--) {
        outType((len>>(c*2))&1);
        outType((len>>(c*2+1))&1);
        outType((c>0)?1:0);
    }
}
void TTuzCode::outDictPos(tuz_dict_size_t pos){
    outLen(pos>>8);
    code.push_back(pos&255);
}


size_t TTuzCode::getSavedDataBit(tuz_length_t data_len)const{
    if (data_len<tuz_kMinLiteralLen)
        return data_len*(size_t)(1+8);
    else
        return data_len*(size_t)8+1+8+3+_getSavedLenBit(data_len-(tuz_kMinLiteralLen-1));
}
size_t TTuzCode::getSavedDictLenBit(tuz_length_t match_len)const{
    return _getSavedLenBit(match_len-tuz_kMinDictMatchLen);
}

size_t TTuzCode::getSavedDictPosBit(tuz_dict_size_t pos)const{
    // break bit + type bit + pos bit
    return 1+1+8+_getSavedLenBit((pos+1)>>8);
}

void TTuzCode::outType(tuz_byte bitv){
    if (type_count==0){
        types_index=code.size();
        code.push_back(0);
    }
    code[types_index]|=(bitv<<type_count);
    ++type_count;
    if (type_count==8)
        type_count=0;
}

void TTuzCode::outData(const tuz_byte* data,const tuz_byte* data_end){
    tuz_length_t len=(tuz_length_t)(data_end-data);
#if (_TEST_COUNT)
    mdata_count++;
    mdata_len_bit[get_bit(len-1)]++;
    if (len-1<kMCount) mdata_len[len-1]++;
#endif
    if (len>=tuz_kMinLiteralLen){
        outType(tuz_codeType_dict);
        outDictPos(0); //dict_pos==0
        outLen(len-(tuz_kMinLiteralLen-1));
        code.insert(code.end(),data,data_end);
    }else{
        while (len--){
            outType(tuz_codeType_data);
            code.push_back(*data++);
        }
    }
}
    
void TTuzCode::outDict(tuz_length_t match_len,tuz_dict_size_t dict_pos){
    tuz_length_t len=match_len-tuz_kMinDictMatchLen;
    outType(tuz_codeType_dict);
    outDictPos(dict_pos+1); //>0
    outLen(len);
#if (_TEST_COUNT)
    mdict_count++;
    mdict_len_bit[get_bit(len)]++;
    if (len<kMCount) mdict_len[len]++;
    mdict_pos_bit[get_bit(dict_pos)]++;
    if (dict_pos<kMCount) mdict_pos[dict_pos]++;
#endif
}


void TTuzCode::outCtrl_streamEnd(){
    outCtrl(tuz_ctrlType_streamEnd);
}
void TTuzCode::outCtrl_clipEnd(){
    outCtrl(tuz_ctrlType_clipEnd);
}

void TTuzCode::outCtrl(tuz_TCtrlType ctrl){
    outType(tuz_codeType_dict);
    outDictPos(0); //dict_pos==0
    outLen(0);
    code.push_back(ctrl);
    half_code_index=kNullIndex;
    type_count=0;
}

}
