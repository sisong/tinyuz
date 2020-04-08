//  tuz_enc_match.h
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
#ifndef _tuz_enc_match_h
#define _tuz_enc_match_h
#include "tuz_sstring.h"
namespace _tuz_private{
    
    struct TMatch{
        explicit TMatch(const tuz_byte* data,const tuz_byte* data_end,
                        const ICode& _coder,const tuz_TCompressProps& _props)
        :sstring(data,data_end,_props.maxSaveLength),coder(_coder),
        props(_props){ }
        bool match(const tuz_byte** out_matched,tuz_length_t* out_match_len,
                   const tuz_byte* cur);
    private:
        TSuffixString               sstring;
        const ICode&                coder;
        const tuz_TCompressProps&   props;
        
        typedef TSuffixString::TInt     TInt;
        typedef TSuffixString::TLCPInt  TLCPInt;
        std::vector<TInt>           cost;
        std::vector<TInt>           dictPos;
        std::vector<TLCPInt>        saveLen;
        void _cost_match(TInt it_inc,const TInt curString,const size_t curi,TInt* curMinDictMatchLen);
        void _getCost(const tuz_byte* cur0);
    };
    
}
#endif //_tuz_enc_match_h
