//  tuz_enc_match.cpp
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
#include "tuz_enc_match.h"
namespace _tuz_private{
    
    typedef TSuffixString::TInt     TInt;
    typedef TSuffixString::TLCPInt  TLCPInt;
    const TInt kNullCostValue=tuz_ui2G_sub_1;
    
    void TMatch::_cost_match(TInt it_inc,const TInt curString,const size_t curi,
                             TInt* curMinDictMatchLen){
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
    
void TMatch::_getCost(const tuz_byte* cur0,size_t unmatched_len){
    size_t costSize=(sstring.src_end-cur0);
    cost.resize(costSize,kNullCostValue);
    dictPos.resize(costSize,-1);
    saveLen.resize(costSize,1);
    
    cost[0]=(TInt)(coder.getSavedDataBit((tuz_length_t)unmatched_len+1)-coder.getSavedDataBit((tuz_length_t)unmatched_len));
    ++unmatched_len;
    for (size_t i=1;i<costSize;++i) {
        TInt curSaveDataCost;
        if (dictPos[i-1]>=0)
            unmatched_len=0;
        curSaveDataCost=(TInt)(cost[i-1]+coder.getSavedDataBit((tuz_length_t)unmatched_len+1)-coder.getSavedDataBit((tuz_length_t)unmatched_len));
        ++unmatched_len;
        if (curSaveDataCost<=cost[i]){
            cost[i]=curSaveDataCost;
            saveLen[i]=1;
            dictPos[i]=-1;
        }
        const TInt curString=(TInt)(cur0+i-sstring.src);
        TInt minDictMatchLen=tuz_kMinDictMatchLen;
        _cost_match( 1,curString,i,&minDictMatchLen);
        _cost_match(-1,curString,i,&minDictMatchLen);
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
                   const tuz_byte* cur,size_t unmatched_len){
    if (cost.empty())
        _getCost(cur,unmatched_len);
    
    size_t costi=cost.size()-(sstring.src_end-cur);
    if (dictPos[costi]>=0){
        *out_matched=cur-dictPos[costi]-1;
        *out_match_len=saveLen[costi];
        return true;
    }else{
        return false;
    }
}

}
