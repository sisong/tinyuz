//  tuz_enc_code.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
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
size_t mdict_offs=0;
double  mdict_pos_offs_bit[32]={0};
double  mdict_pos_offs[kMCount]={0};
double  mdict_pos_offs_neg_bit[32]={0};
double  mdict_pos_offs_neg[kMCount]={0};
long  mdata_count=0;
double  mdata_len_bit[32]={0};
double  mdata_len[kMCount]={0};
long  msum_bits=0;
double mbits[1+1+ 1+1+1]={0};
size_t mLenMaxPos[8]={0};

inline int get_bit(size_t len) { int result=0;  do {  ++result; len>>=1; }while(len>0); return result; }

struct _debug_log {
    ~_debug_log(){
        msum_bits=0;
        for (size_t i=0;i<5;++i)
            msum_bits+=mbits[i];
        for (size_t i=0;i<5;++i){
            mbits[i]/=msum_bits;
            printf("  [%d]  %.8f\n",i,mbits[i]);
        }
        for (size_t i=0;i<32;++i){
            mdict_len_bit[i]=mdict_len_bit[i]/mdict_count+((i>0)?mdict_len_bit[i-1]:0);
            mdict_pos_bit[i]=mdict_pos_bit[i]/mdict_count+((i>0)?mdict_pos_bit[i-1]:0);
            mdata_len_bit[i]=mdata_len_bit[i]/mdata_count+((i>0)?mdata_len_bit[i-1]:0);
            mdict_pos_offs_bit[i]=mdict_pos_offs_bit[i]/mdict_count+((i>0)?mdict_pos_offs_bit[i-1]:0);
            mdict_pos_offs_neg_bit[i]=mdict_pos_offs_neg_bit[i]/mdict_count+((i>0)?mdict_pos_offs_neg_bit[i-1]:0);
        }
        for (size_t i=0;i<kMCount;++i){
            mdict_len[i]=mdict_len[i]/mdict_count+((i>0)?mdict_len[i-1]:0);
            mdict_pos[i]=mdict_pos[i]/mdict_count+((i>0)?mdict_pos[i-1]:0);
            mdata_len[i]=mdata_len[i]/mdata_count+((i>0)?mdata_len[i-1]:0);
            mdict_pos_offs[i]=mdict_pos_offs[i]/mdict_count+((i>0)?mdict_pos_offs[i-1]:0);
            mdict_pos_offs_neg[i]=mdict_pos_offs_neg[i]/mdict_count+((i>0)?mdict_pos_offs_neg[i-1]:0);
        }
    }
};
_debug_log _;
#endif

namespace _tuz_private{
    const int kDictLenPackBit    =1;
    const int kDictPosLenPackBit =2;

    //low to high bitmap: xx?xx? xx?xx? ...
    static tuz_inline size_t _getOutCount(size_t v,size_t packBit,size_t* dec=0){ // v>=0
        size_t count=1;
        size_t _v=v;
        while (1){
            size_t m=((size_t)1)<<(count*packBit);
            if (v<m) break;
            v-=m;
            ++count;
        }
        if (dec) *dec=_v-v;
        return count;
    }
    static tuz_force_inline size_t _getSavedLenBit(size_t v,size_t packBit){
        return _getOutCount(v,packBit)*(packBit+1);
    }

void TTuzCode::outLen(size_t v,size_t packBit){ //v>=0
    size_t dec;
    size_t c=_getOutCount(v,packBit,&dec);
    v-=dec;
    while(c--){
        for (size_t i=0;i<packBit;++i)
            outType((v>>(c*packBit+i))&1);
        outType((c>0)?1:0);
    }
}

#define outDictLen(len)             outLen(len,kDictLenPackBit)
#define outDictPosLen(len)          outLen(len,kDictPosLenPackBit)
#define _getDictLenBit(len)         _getSavedLenBit(len,kDictLenPackBit)
#define _getDictPosLenBit(len)      _getSavedLenBit(len,kDictPosLenPackBit)

void TTuzCode::outType(size_t bit1v){
    if (type_count==0){
        types_index=code.size();
        code.push_back(0);
    }
    tuz_byte* types=&code[types_index];
    (*types)|=(bit1v<<type_count);
    ++type_count;
    if (type_count==tuz_kMaxTypeBitCount)
        type_count=0;
}

void TTuzCode::outDictSize(size_t dict_size){
    checkv((dict_size>0)&&(dict_size<=tuz_kMaxOfDictSize));
    for (size_t i=0;i<tuz_kDictSizeSavedBytes;++i){
        code.push_back(dict_size&0xFF);
        dict_size>>=8;
    }
    checkv(dict_size==0);
}

void TTuzCode::outData(const tuz_byte* data,const tuz_byte* data_end){
    size_t len=(size_t)(data_end-data);
#if (_TEST_COUNT)
    size_t _bits=getSavedDataBit(len);
    mbits[0]+=_bits-8*len;
    mbits[1]+=8*len;
    mdata_count++;
    mdata_len_bit[get_bit(len-1)]++;
    if (len-1<kMCount) mdata_len[len-1]++;
#endif
    if (isNeedLiteralLine&&(len>=tuz_kMinLiteralLen)){
        outCtrl(tuz_ctrlType_literalLine);
        outDictPosLen(len-tuz_kMinLiteralLen);
        code.insert(code.end(),data,data_end);
    }else
    {
        while (len--){
            outType(tuz_codeType_data);
            code.push_back(*data++);
        }
    }
    _isHaveData_back=true;
}

size_t TTuzCode::__getDictPosLenBit(size_t len)const{
    return _getDictPosLenBit(len);
}
size_t TTuzCode::_getSavedDictPosBit(size_t pos)const{
    //type bit + pos bit
    ++pos; //0 for ctrl
    const tuz_BOOL isOutLen=(pos>=(1<<7))?1:0;
    if (isOutLen) pos-=(1<<7);
    return 1+8+(isOutLen?_getDictPosLenBit(pos>>7):0);
}
void TTuzCode::outDictPos(size_t pos){
    const tuz_BOOL isOutLen=(pos>=(1<<7))?1:0;
    if (isOutLen) pos-=(1<<7);
    code.push_back((tuz_byte)((pos&((1<<7)-1))|(isOutLen<<7)));
    if (isOutLen)
        outDictPosLen(pos>>7);
}

size_t TTuzCode::_getSavedDictLenBit(size_t len)const{
    return _getDictLenBit(len);
}
void TTuzCode::outDict(size_t match_len,size_t dict_pos){
    outType(tuz_codeType_dict);
    size_t saved_dict_pos=dict_pos+1; //0 for ctrl
    if (saved_dict_pos>_dict_size_max) _dict_size_max=saved_dict_pos;
    const size_t isSamePos=(_dictPos_back==saved_dict_pos)?1:0;
    const size_t isSavedSamePos=(isSamePos&&_isHaveData_back)?1:0;
    size_t len=match_len-tuz_kMinDictMatchLen;
    if (!isSavedSamePos){
        if (saved_dict_pos>tuz_kBigPosForLen) { checkv(match_len>=3); --len; } 
        //if (saved_dict_pos>((1<<17)+(1<<15)+(1<<13)+(1<<11)+(1<<9)+(1<<7)-1)) { checkv(match_len>=4); --len; } 
    }
    outDictLen(len);
    if (_isHaveData_back)
        outType(isSavedSamePos);
    if (!isSavedSamePos)
        outDictPos(saved_dict_pos);
#if (_TEST_COUNT)
    if (!isSavedSamePos){
        if (match_len<8)
            mLenMaxPos[match_len]=std::max(mLenMaxPos[match_len],dict_pos);
    }
    mbits[2]+=1;
    mbits[2+1]+=getSavedDictLenBit<false>(match_len,0);
    mbits[2+2]+=getSavedDictPosBit<false>(dict_pos,false);
    mdict_count++;
    mdict_len_bit[get_bit(len)]++;
    if (len<kMCount) mdict_len[len]++;
    mdict_pos_bit[get_bit(dict_pos)]++;
    if (dict_pos<kMCount) mdict_pos[dict_pos]++;

    if (mdict_offs>=dict_pos){
        len=mdict_offs-dict_pos;
        mdict_pos_offs_bit[get_bit(len)]++;
        if (len<kMCount) mdict_pos_offs[len]++;
    }else{
        len=dict_pos-1-mdict_offs;
        mdict_pos_offs_neg_bit[get_bit(len)]++;
        if (len<kMCount) mdict_pos_offs_neg[len]++;
    }
    mdict_offs=dict_pos;
#endif
    _isHaveData_back=false;
    _dictPos_back=dict_pos+1;
}

void TTuzCode::outCtrl_streamEnd(){
    outCtrl(tuz_ctrlType_streamEnd);
    outCtrl_typesEnd();
}
void TTuzCode::outCtrl_clipEnd(){
    outCtrl(tuz_ctrlType_clipEnd);
    outCtrl_typesEnd();
}

void TTuzCode::outCtrl(tuz_TCtrlType ctrl){
    outType(tuz_codeType_dict);
    outDictLen(ctrl);
    if (_isHaveData_back)
        outType(0);
    outDictPos(0); //dict_pos==0 ctrl
}

void TTuzCode::outCtrl_typesEnd(){
    type_count=0;
    _dictPos_back=1;
    _isHaveData_back=false;
}

}//end namespace
