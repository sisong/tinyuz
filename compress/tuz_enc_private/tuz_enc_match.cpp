//  tuz_enc_match.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_enc_match.h"
#include "tuz_enc_code.h"
namespace _tuz_private{
    
    typedef TMatch::TInt     TInt;
    typedef TMatch::TUInt    TUInt;
    typedef TMatch::TLCPInt  TLCPInt;
    const TUInt kNullCostValue=~(TUInt)0;
    const TInt  kTInt_Max=(TInt)((~(uint32_t)0)>>1);

    static TInt _sstr_eqLen(const tuz_byte* ss_end,const  tuz_byte* matchedString,const tuz_byte* curString){
        TInt eqLen=0;
        while ((curString<ss_end)&&((*curString)==matchedString[eqLen])){
            ++eqLen; ++curString;
        }
        return eqLen;
    }
    
    void TMatch::_cost_match(TInt it_inc,const TInt curString,const size_t curi,
                             TInt* curMinDictMatchLen,std::vector<TUInt>& cost,bool isMatch_back_pos){
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
        
        const size_t back_pos=dictPos[curi-1];
        const size_t isHaveData=(saveLen[curi-1]==0)?1:0;
        TInt minDictMatchLen=*curMinDictMatchLen;
        const size_t props_dictSize=props.dictSize;
        const TInt* SA=sstring.SA.data();
        if (isMatch_back_pos&&isHaveData){
            TInt matchedString=curString-(TInt)(back_pos+1);
            TInt curLCP=_sstr_eqLen(sstring.src_end,sstring.src+matchedString,sstring.src+curString);
            if (curLCP>=minDictMatchLen){
                const size_t curSaveDictCost=cost[curi-1]+coder.getSavedDictPosBiti<true>(back_pos,1);
                for (TInt mLen=minDictMatchLen;mLen<=curLCP;++mLen){
                    size_t ni=curi+mLen-1;
                    const size_t dictCost=curSaveDictCost+coder.getSavedDictLenBiti(mLen);
                    if (dictCost<cost[ni]){
                        cost[ni]=(TUInt)dictCost;
                        saveLen[ni]=mLen;
                        dictPos[ni]=(TInt)back_pos;
                    }
                }
                minDictMatchLen=curLCP+1;
            }
        }

        TInt curMinLcp = kTInt_Max;
        for (;(it!=it_end);it+=it_inc,LCP+=it_inc){
            TInt curLCP=*LCP;
            curMinLcp=(curLCP<curMinLcp)?curLCP:curMinLcp;
            if (curMinLcp<minDictMatchLen) break;
            TInt matchedString=SA[it];
            const size_t dict_pos=(curString-matchedString)-1;
            if (dict_pos>=props_dictSize) continue; //same as ((TInt)dict_pos<0)|(dict_pos>=props.dictSize)
            if (dict_pos==back_pos) continue;

            const size_t curSaveDictCost=cost[curi-1]+coder.getSavedDictPosBiti<false>(dict_pos,isHaveData);
            for (TInt mLen=minDictMatchLen;mLen<=curMinLcp;++mLen){
                size_t ni=curi+mLen-1;
                const size_t dictCost=curSaveDictCost+coder.getSavedDictLenBiti(mLen);
                if (dictCost<cost[ni]){
                    cost[ni]=(TUInt)dictCost;
                    saveLen[ni]=mLen;
                    dictPos[ni]=(TInt)dict_pos;
                }
            }
            const size_t _kMStep=0; //0 1 16
            minDictMatchLen=(curMinLcp>minDictMatchLen+_kMStep)?curMinLcp-_kMStep:minDictMatchLen;
         }
        *curMinDictMatchLen=minDictMatchLen;
    }
    
void TMatch::_getCost(const tuz_byte* cur0){
    const size_t _kNullValue=~0;
    std::vector<TUInt> cost;
    size_t costSize=(sstring.src_end-cur0);
    cost.resize(costSize,kNullCostValue);
    dictPos.resize(costSize);
    saveLen.resize(costSize,0);
    const size_t _kLiteralCost0=coder.getLiteralCostBit();
    const size_t _kCost0=coder.getSavedDataBit(1);
    cost[0]=(TUInt)_kCost0;
    dictPos[0]=0;
    size_t unmatched_len=1;
    size_t costUnmatchStill_len=_kNullValue;
    size_t costUnmatchStill_dict_i=_kNullValue;
    for (size_t i=1;i<costSize;++i){
        size_t curSaveDataCost;
        if (saveLen[i-1]>0){
            curSaveDataCost=cost[i-1]+_kCost0;
            unmatched_len=1;
        }else{
            ++unmatched_len;
            curSaveDataCost=((i>=unmatched_len)?cost[i-unmatched_len]:0)
                            + coder.getSavedDataBit(unmatched_len);
        }
        if (curSaveDataCost<=cost[i]){
            cost[i]=(TUInt)curSaveDataCost;
            saveLen[i]=0;
            dictPos[i]=dictPos[i-1];
            if (costUnmatchStill_len==_kNullValue){
                costUnmatchStill_len=unmatched_len-1;
                costUnmatchStill_dict_i=_kNullValue;
            }
        }

        if (costUnmatchStill_len!=_kNullValue){
            ++costUnmatchStill_len;
            if (costUnmatchStill_len>unmatched_len){
                const size_t umstillLenCost=coder.getSavedDataBit(costUnmatchStill_len);
                size_t costUzStill=umstillLenCost+((i>=costUnmatchStill_len)?cost[i-costUnmatchStill_len]:0);
                if ((costUzStill>cost[i]+_kLiteralCost0)){
                    costUnmatchStill_len=_kNullValue;
                }else{
                    if (costUzStill<=cost[i]){
                        cost[i]=(TUInt)costUzStill;
                        saveLen[i]=0;
                        dictPos[i]=0;
                        if ((costUnmatchStill_dict_i>0)&&(costUnmatchStill_dict_i!=_kNullValue))
                            dictPos[i]=dictPos[costUnmatchStill_dict_i-1];
                        cost[i-1]=(TUInt)(costUzStill-umstillLenCost+coder.getSavedDataBit(costUnmatchStill_len-1));
                        saveLen[i-1]=0;
                        for (size_t j=costUnmatchStill_dict_i;j<i-1;++j){
                            saveLen[j]=0;
                            dictPos[j]=(costUnmatchStill_dict_i>0)?dictPos[costUnmatchStill_dict_i-1]:0;
                        }
                        costUnmatchStill_dict_i=_kNullValue;
                        unmatched_len=costUnmatchStill_len;
                    }
                }
            }
        }

        const TInt curString=(TInt)(cur0+i-sstring.src);
        TInt curDictMatchLen=tuz_kMinDictMatchLen;
        _cost_match(-1,curString,i,&curDictMatchLen,cost,true);
        _cost_match( 1,curString,i,&curDictMatchLen,cost,false);
        if ((costUnmatchStill_dict_i==_kNullValue)&&(saveLen[i]>0))
            costUnmatchStill_dict_i=i;
    }
    size_t i_inc=costSize;
    while (i_inc>0) {
        size_t i=i_inc-1;
        TLCPInt mlen=saveLen[i];
        if (mlen>0){
            dictPos[i-mlen+1]=dictPos[i];
            saveLen[i-mlen+1]=mlen;
            i_inc-=mlen;
        }else{
            --i_inc;
        }
    }
}

bool TMatch::match(const tuz_byte** out_matched,size_t* out_match_len,
                   const tuz_byte* cur){
    if (saveLen.empty())
        _getCost(cur);
    
    size_t costi=saveLen.size()-(sstring.src_end-cur);
    if (saveLen[costi]>0){
        *out_matched=cur-dictPos[costi]-1;
        *out_match_len=saveLen[costi];
        return true;
    }else{
        return false;
    }
}

}
