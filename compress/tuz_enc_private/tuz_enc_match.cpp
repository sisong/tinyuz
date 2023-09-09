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
    #define _k_kBMatchStep   512  // skip by a big match

    static TInt _sstr_eqLen(const tuz_byte* ss_end,size_t maxSaveLength,
                            const  tuz_byte* matchedString,const tuz_byte* curString){
        TInt eqLen=0;
        while ((curString<ss_end)&&((*curString)==matchedString[eqLen])&&(eqLen<(TInt)maxSaveLength)){
            ++eqLen; ++curString;
        }
        return eqLen;
    }
    #define _MatchedAt(isSamePos,dict_pos){ \
        const size_t saveDictCost=coder.getSavedDictPosBit<isSamePos>(dict_pos,isHaveData);         \
        const size_t curSaveCost=cost[curi-1]+saveDictCost;             \
        for (TInt mLen=curMinLcp;mLen>=tuz_kMinDictMatchLen;--mLen){    \
            const size_t dictCost=curSaveCost+coder.getSavedDictLenBit<isSamePos>(mLen,dict_pos);   \
            const size_t ni=curi+mLen-1;    \
            const size_t cost_ni=cost[ni];  \
            if (dictCost<=cost_ni){         \
                cost[ni]=(TUInt)dictCost;   \
                matchLen[ni]=mLen;          \
                dictPos[ni]=(TPosInt)dict_pos;  \
            }else if (dictCost>cost_ni+4){  \
                break;                      \
            }   \
        }       \
        (*out_maxMatchLen)=((size_t)curMinLcp>(*out_maxMatchLen))?curMinLcp:(*out_maxMatchLen); \
        if (curMinLcp>=_k_kBMatchStep) return; }
    
    void TMatch::_cost_match(const TInt curString,const size_t curi,
                             size_t* out_maxMatchLen,std::vector<TUInt>& cost){
        if (curString<=0) return;
        const size_t back_pos=dictPos[curi-1];
        const size_t isHaveData=((matchLen[curi-1]==0)&&(curi>1))?1:0;
        const TInt* SA=sstring.SA.data();
        const TInt it_cur=sstring.R[curString];

        if (isHaveData){ //same pos match
            TInt matchedString=curString-(TInt)(back_pos+1);
            TInt curMinLcp=_sstr_eqLen(sstring.src_end,props.maxSaveLength,
                                      sstring.src+matchedString,sstring.src+curString);
            if (curMinLcp>=tuz_kMinDictMatchLen)
                _MatchedAt(true,back_pos);
        }

        size_t maxDictPos=props.dictSize-1;
        if (maxDictPos>=(size_t)curString) maxDictPos=curString-1;
        {// short pos match
            #define _kShortPosForLen  (((1<<7)-1)-1)
            //#define _kShortPosForLen  (((1<<9)-1)-1) // better(0.2%) but slower(30%)
            const TInt kShortPos=(TInt)((maxDictPos>=_kShortPosForLen)?_kShortPosForLen:maxDictPos);
            for (TInt dict_pos=kShortPos;dict_pos>=0;dict_pos--){
                TInt matchedString=curString-(dict_pos+1);
                TInt curMinLcp=_sstr_eqLen(sstring.src_end,props.maxSaveLength,
                                           sstring.src+matchedString,sstring.src+curString);
                if (curMinLcp>=tuz_kMinDictMatchLen)
                    _MatchedAt(false,dict_pos);
            }
            if (maxDictPos<=_kShortPosForLen) 
                return;
        }

        {//match by LCP
            TInt it_left=it_cur-1;
            const TInt it_left_end=-1;
            TInt it_right=it_cur+1;
            const TInt it_right_end=(TInt)sstring.size();
            const TLCPInt* LCP=sstring.LCP.data();

            TInt matchCountLimit=1024*2; //limit match time
            while (matchCountLimit>0){
                TInt curMinLcp=0;
                size_t dict_pos=-1;
                if (it_left>it_left_end){
                    curMinLcp=LCP[it_left];
                    dict_pos=(curString-SA[it_left])-1;
                }
                if (it_right<it_right_end){
                    TInt curLcp=LCP[it_right-1];
                    if (curLcp<curMinLcp){
                        --it_left;
                    }else{
                        size_t curPos=(curString-SA[it_right])-1;
                        ++it_right;
                        if (curLcp>curMinLcp){
                            curMinLcp=curLcp;
                            dict_pos=curPos;
                        }else{
                            dict_pos=(dict_pos<curPos)?dict_pos:curPos;
                            --it_left;
                        }
                    }
                }else{
                    --it_left;
                }
                if (curMinLcp<tuz_kMinDictMatchLen) 
                    return;

                while ((it_left>it_left_end)&&(LCP[it_left]>=curMinLcp)){
                    size_t curPos=(curString-SA[it_left])-1;
                    dict_pos=(dict_pos<curPos)?dict_pos:curPos;
                    if ((matchCountLimit--)<=0) break;
                    --it_left;
                }
                while ((it_right<it_right_end)&&(LCP[it_right-1]>=curMinLcp)){
                    size_t curPos=(curString-SA[it_right])-1;
                    dict_pos=(dict_pos<curPos)?dict_pos:curPos;
                    if ((matchCountLimit--)<=0) break;
                    ++it_right;
                }
                if (dict_pos<=maxDictPos)
                    _MatchedAt(false,dict_pos);
            }
        }
    }

void TMatch::_getCostByMatch(const tuz_byte* cur0,std::vector<TUInt>& cost){
    size_t costSize=cost.size();
    size_t unmatched_len=0;
    
    size_t i=1;
    while (i<costSize){
        if (matchLen[i-1]>0)
            unmatched_len=1;
        else
            ++unmatched_len;
        size_t curSaveDataCost=cost[i-unmatched_len]+coder.getSavedDataBit(unmatched_len);
        if (curSaveDataCost<cost[i]){
            cost[i]=(TUInt)curSaveDataCost;
            matchLen[i]=0;
            dictPos[i]=dictPos[i-unmatched_len];
        }

        const TInt curString=(TInt)(cur0+(i-1)-sstring.src);
        size_t maxMatchLen=tuz_kMinDictMatchLen;
        _cost_match(curString,i,&maxMatchLen,cost);
        
        i+=(maxMatchLen<_k_kBMatchStep)?1:maxMatchLen;
    }
}

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
            const size_t curPos=dictPos[i];
            size_t nextPos=curPos-1; //not same
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
            if (props.isNeedLiteralLine)
                cost[i-mlen+1]=cost[i];
            i_inc-=mlen;
        }else{
            --i_inc;
        }
    }

    if (props.isNeedLiteralLine)
        _getCostByLiteralLen(cur0,cost);
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
