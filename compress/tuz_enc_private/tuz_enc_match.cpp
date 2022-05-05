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
    
    void TMatch::_cost_match(const TInt curString,const size_t curi,
                             size_t* curMinMatchLen,std::vector<TUInt>& cost){
        const size_t back_pos=dictPos[curi-1];
        const size_t isHaveData=((matchLen[curi-1]==0)&&(curi>1))?1:0;
        const TInt* SA=sstring.SA.data();
        const TInt it_cur=sstring.R[curString];
        const size_t _kMinSaveNoSamePosCost=coder.getSavedDictPosBit<false>(0,0);

        size_t minDictMatchLen=(*curMinMatchLen);
        if (isHaveData){
            TInt matchedString=curString-(TInt)(back_pos+1);
            TInt curMinLcp=_sstr_eqLen(sstring.src_end,sstring.src+matchedString,sstring.src+curString);
            if (curMinLcp>=tuz_kMinDictMatchLen){
                const size_t curSaveDictCost=cost[curi-1]+coder.getSavedDictPosBit<true>(back_pos,1);
                for (TInt mLen=curMinLcp;mLen>=tuz_kMinDictMatchLen;--mLen){
                    const size_t dictCost=curSaveDictCost+coder.getSavedDictLenBit<true>(mLen,0);
                    const size_t ni=curi+mLen-1;
                    const size_t cost_ni=cost[ni];
                    if (dictCost<=cost_ni){
                        cost[ni]=(TUInt)dictCost;
                        matchLen[ni]=mLen;
                        dictPos[ni]=(TInt)back_pos;
                    }
                }
                (*curMinMatchLen)=(curMinLcp>minDictMatchLen)?curMinLcp:minDictMatchLen;
            }
       }

        for (TInt it_inc=-1;it_inc<=1;it_inc+=2){
            TInt it=it_cur+it_inc;
            TInt it_end;
            const TLCPInt* LCP;
            if (it_inc==1){
                it_end=(TInt)sstring.size();
                LCP=sstring.LCP.data()-1;
            }else{
                it_end=-1;
                LCP=sstring.LCP.data();
            }

            minDictMatchLen=(*curMinMatchLen);
            size_t maxDictPos=props.dictSize;
            while (it!=it_end){
                TInt curMinLcp=LCP[it];
                if (curMinLcp<minDictMatchLen) break;
                size_t dict_pos=(curString-SA[it])-1;
                it+=it_inc;
                while ((it!=it_end)&&(LCP[it]>=curMinLcp)){
                    size_t curPos=(curString-SA[it])-1;
                    dict_pos=(dict_pos<curPos)?dict_pos:curPos;
                    it+=it_inc;
                }
                if (dict_pos>=maxDictPos) continue; //same as ((TInt)dict_pos<0)|(dict_pos>=maxDictPos)

                const size_t saveDictCost=coder.getSavedDictPosBit<false>(dict_pos,isHaveData);
                const size_t curSaveCost=cost[curi-1]+saveDictCost;
                for (TInt mLen=curMinLcp;mLen>=tuz_kMinDictMatchLen;--mLen){
                    const size_t dictCost=curSaveCost+coder.getSavedDictLenBit<false>(mLen,dict_pos);
                    const size_t ni=curi+mLen-1;
                    const size_t cost_ni=cost[ni];
                    if (dictCost<=cost_ni){
                        cost[ni]=(TUInt)dictCost;
                        matchLen[ni]=mLen;
                        dictPos[ni]=(TInt)dict_pos;
                    }else if (dictCost>cost_ni){
                        break;
                    }
                }
                
                size_t _kSMStep=(saveDictCost-_kMinSaveNoSamePosCost+2+7)>>3;
                maxDictPos=(dict_pos+_kSMStep<maxDictPos)?dict_pos+_kSMStep:maxDictPos;
                minDictMatchLen=(curMinLcp>minDictMatchLen+_kSMStep)?curMinLcp-_kSMStep:minDictMatchLen;
            }

            #define _kSafeMatchStep  32
            if (minDictMatchLen<128) //short match, search more for next
                minDictMatchLen=(minDictMatchLen>(*curMinMatchLen)+_kSafeMatchStep)?minDictMatchLen-_kSafeMatchStep:(*curMinMatchLen);
            *curMinMatchLen=minDictMatchLen;
        }
    }

void TMatch::_getCostByMatch(const tuz_byte* cur0,std::vector<TUInt>& cost){
    size_t costSize=cost.size();
    size_t unmatched_len=0;
    
    size_t curMinMatchLen=tuz_kMinDictMatchLen;
    size_t i=1;
    while (i<costSize){
        if (matchLen[i-1]>0)
            unmatched_len=1;
        else
            ++unmatched_len;
        size_t curSaveDataCost=cost[i-unmatched_len]+coder.getSavedDataBit(unmatched_len);
        if (curSaveDataCost<cost[i]){
            curMinMatchLen=tuz_kMinDictMatchLen;
            cost[i]=(TUInt)curSaveDataCost;
            matchLen[i]=0;
            dictPos[i]=dictPos[i-unmatched_len];
        }

        const TInt curString=(TInt)(cur0+(i-1)-sstring.src);
        _cost_match(curString,i,&curMinMatchLen,cost);
        
        #define _k_kBMatchStep   512  // skip by a big match
        if (curMinMatchLen<_k_kBMatchStep){
            ++i;
            curMinMatchLen=(curMinMatchLen>tuz_kMinDictMatchLen+1)?(curMinMatchLen-1):tuz_kMinDictMatchLen;
        }else
            i+=curMinMatchLen;
            curMinMatchLen=tuz_kMinDictMatchLen;
    }
}

#if tuz_isNeedLiteralLine
void TMatch::_getCostByLiteralLen(const tuz_byte* cur0,std::vector<TUInt>& cost){
    size_t costSize=cost.size();
    size_t unmatched_len=0;
    size_t unmatched_fill0_i=0;

    size_t i=1;
    while (i<costSize){
        size_t mlen=matchLen[i];
        if (mlen==0){
            ++unmatched_len;
        }else{
            const TInt curPos=dictPos[i];
            TInt nextPos=curPos-1; //not same
            size_t j=i+mlen;
            while (j<costSize){
                if (matchLen[j]>0){
                    nextPos=dictPos[j];
                    break;
                }
                ++j;
            }
            if (curPos==nextPos){
                size_t nexti=j;
                unmatched_len+=nexti-i;
                i=nexti;
                continue; //next
            }

            unmatched_len+=mlen;
            i+=mlen-1;
        }

        size_t curSaveDataCost=cost[i-unmatched_len]+coder.getSavedDataBit(unmatched_len);
        if (curSaveDataCost<cost[i]){
            for (size_t j=unmatched_fill0_i+1;j<=i;++j)
                matchLen[j]=0;
            unmatched_fill0_i=i;
        }else if (curSaveDataCost>=cost[i]+tuz_kMinLiteralLen){
            unmatched_len=0;
            unmatched_fill0_i=i;
        }
        ++i;
    }    
}
#endif
void TMatch::_getCost(const tuz_byte* cur0){
    std::vector<TUInt> cost;
    size_t costSize=(sstring.src_end-cur0)+1;
    cost.resize(costSize,kNullCostValue);
    dictPos.resize(costSize);
    matchLen.resize(costSize,0);
    cost[0]=0;
    dictPos[0]=0;
    matchLen[0]=0;
    
    _getCostByMatch(cur0,cost);

    size_t i_inc=costSize;
    while (i_inc>0) {
        size_t i=i_inc-1;
        TLCPInt mlen=matchLen[i];
        if (mlen>0){
            dictPos[i-mlen+1]=dictPos[i];
            matchLen[i-mlen+1]=mlen;
#if tuz_isNeedLiteralLine
            cost[i-mlen+1]=cost[i];
#endif
            i_inc-=mlen;
        }else{
            --i_inc;
        }
    }

#if tuz_isNeedLiteralLine
    _getCostByLiteralLen(cur0,cost);
#endif
}

bool TMatch::match(const tuz_byte** out_matched,size_t* out_match_len,
                   const tuz_byte* cur){
    if (matchLen.empty())
        _getCost(cur);
    
    size_t costi=matchLen.size()-(sstring.src_end-cur);
    if (matchLen[costi]>0){
        *out_match_len=matchLen[costi];
        *out_matched=cur-dictPos[costi]-1;
        return true;
    }else{
        return false;
    }
}

}
