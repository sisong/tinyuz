//  tuz_enc_match.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_enc_match.h"
namespace _tuz_private{
    
    typedef TSuffixString::TInt     TInt;
    typedef TSuffixString::TLCPInt  TLCPInt;
    const TInt kNullCostValue=tuz_ui2G_sub_1;
    
    void TMatch::_cost_match(TInt it_inc,const TInt curString,const size_t curi,
                             TInt* curMinDictMatchLen,std::vector<TInt>& cost){
        const TInt it_cur=sstring.R[curString];
        TInt it=it_cur+it_inc;
        TInt it_end;
        const TLCPInt* LCP;
        if (it_inc==1){
            it_end=(TInt)sstring.size();
            LCP=sstring.LCP.data()+it_cur;
        }else{
            assert(it_inc==-1);
            it_end=-1;
            LCP=sstring.LCP.data()+it_cur-1;
        }
        
        TInt minDictMatchLen=*curMinDictMatchLen;
        TInt curMinLcp=(TInt)tuz_ui2G_sub_1;
        const size_t props_dictSize=props.dictSize;
        const TInt* SA=sstring.SA.data();
        for (;(it!=it_end);it+=it_inc,LCP+=it_inc){
            TInt curLCP=*LCP;
            curMinLcp=(curLCP<curMinLcp)?curLCP:curMinLcp;
            if (curMinLcp<minDictMatchLen) break;
            TInt matchedString=SA[it];
            const size_t dict_pos=(curString-matchedString)-1;
            if (dict_pos>=props_dictSize) continue; //same as ((TInt)dict_pos<0)|(dict_pos>=props.dictSize)
            
            const TInt curSaveDictCost=cost[curi-1]+(TInt)coder.getSavedDictPosBit((tuz_size_t)dict_pos);
            for (TInt mLen=minDictMatchLen;mLen<=curMinLcp;++mLen){
                size_t ni=curi+mLen-1;
                const TInt dictCost=curSaveDictCost+(TInt)coder.getSavedDictLenBit(mLen);
                if (dictCost<cost[ni]){
                    cost[ni]=dictCost;
                    saveLen[ni]=mLen;
                    dictPos[ni]=(TInt)dict_pos;
                }
            }
            minDictMatchLen=curMinLcp;
        }
        *curMinDictMatchLen=minDictMatchLen;
    }
    
void TMatch::_getCost(const tuz_byte* cur0){
    const size_t _kNullValue=~0;
    std::vector<TInt> cost;
    size_t costSize=(sstring.src_end-cur0);
    cost.resize(costSize,kNullCostValue);
    dictPos.resize(costSize,-1);
    saveLen.resize(costSize,1);
    const TInt _kLiteralCost0=(TInt)coder.getLiteralCostBit();
    const TInt _kCost0=(TInt)coder.getSavedDataBit(1);
    cost[0]=_kCost0;
    size_t unmatched_len=1;
    size_t costUnmatchStill_len=_kNullValue;
    size_t costUnmatchStill_dict_i=_kNullValue;
    for (size_t i=1;i<costSize;++i){
        TInt curSaveDataCost;
        if (dictPos[i-1]>=0){
            curSaveDataCost=cost[i-1]+_kCost0;
            unmatched_len=1;
        }else{
            ++unmatched_len;
            curSaveDataCost=((i>=unmatched_len)?cost[i-unmatched_len]:0)
                            + (TInt)coder.getSavedDataBit((tuz_length_t)unmatched_len);
        }
        if (curSaveDataCost<=cost[i]){
            cost[i]=curSaveDataCost;
            saveLen[i]=1;
            dictPos[i]=-1;
            if (costUnmatchStill_len==_kNullValue){
                costUnmatchStill_len=unmatched_len-1;
                costUnmatchStill_dict_i=_kNullValue;
            }
        }

        if (costUnmatchStill_len!=_kNullValue){
            ++costUnmatchStill_len;
            if (costUnmatchStill_len>unmatched_len){
                TInt costUzStill=((i>=costUnmatchStill_len)?cost[i-costUnmatchStill_len]:0)
                                    +(TInt)coder.getSavedDataBit((tuz_length_t)costUnmatchStill_len);
                if ((costUzStill>cost[i]+_kLiteralCost0)){
                    costUnmatchStill_len=_kNullValue;
                }else{
                    if (costUzStill<=cost[i]){
                        cost[i]=costUzStill;
                        saveLen[i]=1;
                        dictPos[i]=-1;
                        cost[i-1]=costUzStill-(TInt)coder.getSavedDataBit((tuz_length_t)costUnmatchStill_len)
                                    +(TInt)coder.getSavedDataBit((tuz_length_t)(costUnmatchStill_len-1));
                        saveLen[i-1]=1;
                        dictPos[i-1]=-1;
                        for (size_t j=costUnmatchStill_dict_i;j<i-1;++j){
                            saveLen[j]=1;
                            dictPos[j]=-1;
                        }
                        costUnmatchStill_dict_i=_kNullValue;
                        unmatched_len=costUnmatchStill_len;
                    }
                }
            }
        }

        const TInt curString=(TInt)(cur0+i-sstring.src);
        TInt minDictMatchLen=tuz_kMinDictMatchLen;
        _cost_match( 1,curString,i,&minDictMatchLen,cost);
        _cost_match(-1,curString,i,&minDictMatchLen,cost);
        if ((costUnmatchStill_dict_i==_kNullValue)&&(dictPos[i]>=0))
            costUnmatchStill_dict_i=i;
    }
    size_t wi=costSize;
    while (wi>0) {
        size_t i=wi-1;
        if (dictPos[i]>=0){
            TInt mlen=saveLen[i];
            dictPos[i-mlen+1]=dictPos[i];
            saveLen[i-mlen+1]=mlen;
            wi-=mlen;
        }else{
            saveLen[i]=1;
            --wi;
        }
    }
}

bool TMatch::match(const tuz_byte** out_matched,tuz_length_t* out_match_len,
                   const tuz_byte* cur){
    if (dictPos.empty())
        _getCost(cur);
    
    size_t costi=dictPos.size()-(sstring.src_end-cur);
    if (dictPos[costi]>=0){
        *out_matched=cur-dictPos[costi]-1;
        *out_match_len=saveLen[costi];
        return true;
    }else{
        return false;
    }
}

}
