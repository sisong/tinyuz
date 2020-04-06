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
        const unsigned int props_dictSize=props.dictSize;
        const TInt* SA=sstring.SA.data();
        for (;(it!=it_end);it+=it_inc,LCP+=it_inc){
            TInt curLCP=*LCP;
            curMinLcp=(curLCP<curMinLcp)?curLCP:curMinLcp;
            if (curLCP<minDictMatchLen) break;
            TInt matchedString=SA[it];
            const unsigned int dict_pos=(curString-matchedString)-1;
            if (dict_pos>=props_dictSize) continue; //same as ((TInt)dict_pos<0)|(dict_pos>=props.dictSize)
            
            TInt curSaveDictCost=cost[curi-1]+1+coder.getSavedPosBit(dict_pos);
            for (TInt mLen=minDictMatchLen;mLen<=curMinLcp;++mLen) {
                size_t ni=curi+mLen-1;
                TInt dictCost=curSaveDictCost+coder.getSavedLenBit(mLen-props.minDictMatchLen);
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
    size_t costSize=(sstring.src_end-cur0);
    cost.resize(costSize,kNullCostValue);
    dictPos.resize(costSize,-1);
    saveLen.resize(costSize,1);
    const TInt kNewNoDictCost=1+8+coder.getSavedLenBit(1);
    
    cost[0]=kNewNoDictCost;
    for (size_t i=1;i<costSize;++i) {
        if (dictPos[i-1]<0){//no dict
            TInt frontLen=saveLen[i-1];
            TInt curSaveDataCost=cost[i-1]+8+coder.getSavedLenBit(frontLen+1)-coder.getSavedLenBit(frontLen);
            if (curSaveDataCost<cost[i]){
                cost[i]=curSaveDataCost;
                saveLen[i]=frontLen+1;
                dictPos[i]=-1;
            }
        }else{//new data
            TInt curSaveDataCost=cost[i-1]+kNewNoDictCost;
            if (curSaveDataCost<cost[i]){
                cost[i]=curSaveDataCost;
                saveLen[i]=1;
                dictPos[i]=-1;
            }
        }
        const TInt curString=(TInt)(cur0+i-sstring.src);
        TInt minDictMatchLen=props.minDictMatchLen;
        _cost_match( 1,curString,i,&minDictMatchLen);
        _cost_match(-1,curString,i,&minDictMatchLen);
    }
    //printf("costend: %d\n",(cost[costSize-1]+7)/8);
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
    
    
    void TMatch::_match(TInt it_inc,TInt* curBestBitScore,
                        const tuz_byte** curBestMatched,tuz_length_t* curBestMatchLen,
                        const TInt curString,size_t unmatched_len){
        const TInt it_cur=sstring.R[curString];
        TInt it=it_cur+it_inc;
        TInt it_end;
        const TLCPInt* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
        if (it_inc==1){
            it_end=(TInt)sstring.size();
            LCP=sstring.LCP.data()+it_cur;
        }else{
            assert(it_inc==-1);
            it_end=-1;
            LCP=sstring.LCP.data()+it_cur-1;
        }
        
        const TInt kBreakCodeScore=1+coder.minSavedLenBit;
        const TInt kBreakCodeScore_dict=1+((unmatched_len>0)?kBreakCodeScore:0);
        const TInt kBreakCodeScore_data=((unmatched_len>0)?0:kBreakCodeScore);
        TInt curMinLcp=(TInt)tuz_ui2G_sub_1;
        const TInt kMinDictStoreBit=1+coder.minSavedLenBit*2+((unmatched_len>0)?kBreakCodeScore:0);
        const TInt minDictMatchLen=props.minDictMatchLen;
        const unsigned int props_dictSize=props.dictSize;
        const TInt* SA=sstring.SA.data();
        TInt  bestBitScore=*curBestBitScore;
        for (;(it!=it_end);it+=it_inc,LCP+=it_inc){
            TInt curLCP=*LCP;
            if (curLCP<minDictMatchLen) break;
            curMinLcp=(curLCP<curMinLcp)?curLCP:curMinLcp;
            TInt matchedString=SA[it];
            const unsigned int dict_pos=(curString-matchedString)-1;
            if (dict_pos>=props_dictSize) continue; //same as ((TInt)dict_pos<0)|(dict_pos>=props.dictSize)

            TInt  dataStoreBit=curMinLcp*8+kBreakCodeScore_data;
            if (dataStoreBit<=bestBitScore+kMinDictStoreBit)
                break;
            
            TInt  dictStoreBit=coder.getSavedLenBit(curMinLcp-minDictMatchLen)
                                +coder.getSavedPosBit((tuz_dict_size_t)dict_pos)+kBreakCodeScore_dict;
            TInt  curBitScore=dataStoreBit-dictStoreBit;
            if (curBitScore<=bestBitScore) continue;
            bestBitScore=curBitScore;
            *curBestMatched=sstring.src+matchedString;
            *curBestMatchLen=curMinLcp;
        }
        *curBestBitScore=bestBitScore;
    }
    
bool TMatch::match(const tuz_byte** out_matched,tuz_length_t* out_match_len,
                   const tuz_byte* cur,size_t unmatched_len){
    if (cost.empty())
        _getCost(cur);
    
    size_t costi=cost.size()-(sstring.src_end-cur);
    if (dictPos[costi]>=0){
        *out_matched=cur-dictPos[costi]-1;
        *out_match_len=saveLen[costi];
        return true;
    }else{
        return false;
    }
    
    //*
    const TInt curString=(TInt)(cur-sstring.src);
    *out_matched=0;
    *out_match_len=0;
    TInt curBestBitScore=0;
    _match(-1,&curBestBitScore,out_matched,out_match_len,curString,unmatched_len);
    _match( 1,&curBestBitScore,out_matched,out_match_len,curString,unmatched_len);
    return (*out_matched)!=0;//*/
}

}
