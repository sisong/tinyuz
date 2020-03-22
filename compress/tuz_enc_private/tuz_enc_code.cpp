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
namespace _tuz_private{
    
    //low to high bitmap: xxx?xxx? xxx?xxx? ...
    static tuz_byte _pack_v(tuz_byte* half_code,tuz_byte* code,tuz_length_t v){
        tuz_byte half_count=0;
        do {
            tuz_byte halfv=v&7;
            v>>=3;
            halfv|=(v>0)?8:0;
            if (half_code){
                (*half_code)|=(halfv<<4);
                half_code=0;
            }else{
                *code=halfv;
                half_code=code;
                ++code;
            }
            ++half_count;
        }while(v>0);
        return half_count;
    }
    
    static tuz_inline tuz_byte _pack_v_half_count(tuz_length_t v){
        tuz_byte half_count=0;
        do {
            v>>=3;
            ++half_count;
        }while(v>0);
        return half_count;
    }

void TTuzCode::outLen(tuz_length_t len){
    tuz_byte* half_code=(half_code_index!=kNullIndex)?&code[half_code_index]:0;
    tuz_byte code_buf[kMaxPackedLenByteSize];
    tuz_byte half_count=_pack_v(half_code,code_buf,len);
    if (half_code) --half_count;
    if (half_count>0)
        code.insert(code.end(),code_buf,code_buf+(half_count+1)/2);
    half_code_index=(half_count&1)?(code.size()-1):kNullIndex;
}

tuz_byte TTuzCode::getSavedLenBit(tuz_length_t len)const{
    return _pack_v_half_count(len)*4;
}
    
void TTuzCode::outType(tuz_TCodeType type){
    if (type_count==0){
        types_index=code.size();
        code.push_back(0);
    }
    code[types_index]|=(type<<type_count);
    ++type_count;
    if (type_count==8)
        type_count=0;
}

void TTuzCode::outData(tuz_length_t len,const tuz_byte* data,const tuz_byte* data_end){
    assert(len>0);
    outType(tuz_codeType_data);
    outLen(len);
    code.insert(code.end(),data,data_end);
}
    
void TTuzCode::outDict(tuz_length_t len,tuz_length_t dict_pos){
    outType(tuz_codeType_dict);
    outLen(len);
    outLen(dict_pos);
}

void TTuzCode::outCtrl(tuz_TCtrlType ctrl){
    outType(tuz_codeType_data);
    outLen(0); //len==0
    code.push_back(ctrl);
}

void TTuzCode::outCtrl_streamEnd(){
    outCtrl(tuz_ctrlType_streamEnd);
    half_code_index=kNullIndex;
    type_count=0;
}
    
void TTuzCode::outCtrl_clipEnd(){
    outCtrl(tuz_ctrlType_clipEnd);
    half_code_index=kNullIndex;
    type_count=0;
}

}
