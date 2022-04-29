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
    static tuz_inline size_t _getOutCount(size_t v,int packBit,size_t* dec=0){ // v>=0
        size_t count=1;
        size_t _v=v;
        while (1){
            size_t m=1<<(count*packBit);
            if (v<m) break;
            v-=m;
            ++count;
        }
        if (dec) *dec=_v-v;
        return count;
    }
    static tuz_force_inline size_t _getSavedLenBit(size_t v,int packBit){
        return _getOutCount(v,packBit)*(packBit+1);
    }

void TTuzCode::outLen(size_t v,int packBit){ //v>=0
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
    assert(dict_size<=tuz_kMaxOfDictSize);
#if (tuz_isNeedSaveDictSize)
    for (size_t i=0;i<tuz_kDictSizeSavedCount;++i){
        code.push_back(dict_size&0xFF);
        dict_size>>=8;
    }
#endif
}

size_t TTuzCode::getLiteralCostBit()const{
    return tuz_kMinLiteralLen*2;
}

size_t TTuzCode::getSavedDataBit(size_t data_len)const{
    if (data_len<tuz_kMinLiteralLen)
        return data_len*(1+8);
    else
        return data_len*8+1+8+_getDictPosLenBit(data_len-(tuz_kMinLiteralLen-1)); //0 for ctrl
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
    if (len>=tuz_kMinLiteralLen){
        outType(tuz_codeType_dict);
        if (_isHaveData_back)
            outType(0);
        outDictPos(0); //dict_pos==0
        outDictPosLen(len-(tuz_kMinLiteralLen-1)); //0 is ctrl
        code.insert(code.end(),data,data_end);
        _isHaveData_back=false;
    }else{
        while (len--){
            outType(tuz_codeType_data);
            code.push_back(*data++);
        }
        _isHaveData_back=true;
    }
}


size_t TTuzCode::getSavedDictPosBit(size_t pos,size_t back_pos,bool isHaveData)const{
    if (isHaveData&&(pos==back_pos)){
        return 1+1;
    }else{
        //type bit + pos bit
        ++pos; //0 for ctrl or literal
        const tuz_BOOL isOutLen=(pos>=(1<<7))?1:0;
        if (isOutLen) pos-=(1<<7);
        return 1+(isHaveData?1:0)+8+(isOutLen?_getDictPosLenBit(pos>>7):0);
    }
}
void TTuzCode::outDictPos(size_t pos){
    const tuz_BOOL isOutLen=(pos>=(1<<7))?1:0;
    if (isOutLen) pos-=(1<<7);
    code.push_back((tuz_byte)((pos&((1<<7)-1))|(isOutLen<<7)));
    if (isOutLen)
        outDictPosLen(pos>>7);
}

size_t TTuzCode::getSavedDictLenBit(size_t match_len)const{
    return _getDictLenBit(match_len-tuz_kMinDictMatchLen);
}
void TTuzCode::outDict(size_t match_len,size_t dict_pos){
    size_t len=match_len-tuz_kMinDictMatchLen;
    outType(tuz_codeType_dict);
    if (_isHaveData_back&&(_dictPos_back==(dict_pos+1))){
        outType(1); //same pos
    }else{
        if (_isHaveData_back)
            outType(0);
        outDictPos(dict_pos+1); //>0
    }
    outDictLen(len);
    _isHaveData_back=false;
    _dictPos_back=dict_pos+1;
#if (_TEST_COUNT)
    mbits[2]+=1;
    mbits[2+1]+=getSavedDictLenBit(match_len);
    mbits[2+2]+=getSavedDictPosBit(dict_pos,0,false);
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
}

void TTuzCode::outCtrl_streamEnd(){
    outCtrl(tuz_ctrlType_streamEnd);
}
void TTuzCode::outCtrl_clipEnd(){
    outCtrl(tuz_ctrlType_clipEnd);
}

void TTuzCode::outCtrl(tuz_TCtrlType ctrl){
    outType(tuz_codeType_dict);
    if (_isHaveData_back)
        outType(0);
    outDictPos(0); //dict_pos==0
    outDictPosLen(0); //ctrl 
    code.push_back(ctrl);
    outCtrl_typesEnd();
}

void TTuzCode::outCtrl_typesEnd(){
    type_count=0;
    _dictPos_back=1;
    _isHaveData_back=false;
}

}//end namespace
