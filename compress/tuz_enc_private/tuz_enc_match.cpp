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
    
    typedef TSuffixString::TInt TInt;
    
    void TMatch::_match(TInt it_inc,size_t& curBestBitScore,
                        const tuz_byte** curBestMatched,tuz_length_t* curBestMatchLen,
                        const TInt curString,size_t unmatched_len){
        const TInt it_cur=sstring.R[curString];
        TInt it=it_cur+it_inc;
        TInt it_end;
        const TInt* LCP;//当前的后缀字符串和下一个后缀字符串的相等长度.
        if (it_inc==1){
            it_end=(TInt)sstring.size();
            LCP=sstring.LCP.data()+it_cur;
        }else{
            assert(it_inc==-1);
            it_end=-1;
            LCP=sstring.LCP.data()+it_cur-1;
        }
        
        const size_t kBreakCodeScore=1+coder.minSavedLenBit;
        TInt curMinLcp=(TInt)tuz_ui2G_sub_1;
        const size_t kMinDictStoreBit=1+coder.minSavedLenBit*2+((unmatched_len>0)?kBreakCodeScore:0);
        const TInt minDictMatchLen=props.minDictMatchLen;
        const int kMaxSearchDeep=1024*32; //todo: change for data size
        for (size_t deep=kMaxSearchDeep;(deep>0)&(it!=it_end);it+=it_inc,LCP+=it_inc,--deep){
            TInt curLCP=*LCP;
            if (curLCP<minDictMatchLen)
                break;
            if (curLCP<curMinLcp)
                curMinLcp=curLCP;
            size_t  dataStoreBit=(size_t)curMinLcp*8+((unmatched_len>0)?0:kBreakCodeScore);
            if (dataStoreBit<=curBestBitScore+kMinDictStoreBit)
                break;
            TInt matchString=sstring.SA[it];
            const TInt dict_pos=(curString-matchString)-1;
            if ((dict_pos<0)|(dict_pos>=props.dictSize)) continue;
            
            --deep;//for speed
            size_t  dictStoreBit=(size_t)1+coder.getSavedLenBit(curMinLcp-props.minDictMatchLen)
                        +coder.getSavedPosBit(dict_pos)+((unmatched_len>0)?kBreakCodeScore:0);
            if (dataStoreBit<=curBestBitScore+dictStoreBit) continue;
            size_t  curBitScore=dataStoreBit-dictStoreBit;
            
            deep-=(kMaxSearchDeep/16); //for speed
            curBestBitScore=curBitScore;
            *curBestMatched=sstring.src+matchString;
            *curBestMatchLen=curMinLcp;
        }
    }
    
bool TMatch::match(const tuz_byte** out_matched,tuz_length_t* out_match_len,
                   const tuz_byte* cur,size_t unmatched_len){
    const TInt curString=(TInt)(cur-sstring.src);
    *out_matched=0;
    *out_match_len=0;
    size_t curBestBitScore=0;
    _match(-1,curBestBitScore, out_matched,out_match_len,curString,unmatched_len);
        _match(1,curBestBitScore, out_matched,out_match_len,curString,unmatched_len);
    return (*out_matched)!=0;
}

}
