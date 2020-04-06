//  tuz_sstring.h
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
#ifndef _tuz_sstring_h
#define _tuz_sstring_h
#include "tuz_enc_types_private.h"
namespace _tuz_private{
    
    struct TSuffixString{
        typedef int32_t  TInt;
        typedef uint16_t TLCPInt;
        TSuffixString(const tuz_byte* _src,const tuz_byte* _src_end,TLCPInt maxLCPValue) //data size < 2G
        :src(_src),src_end(_src_end) { _init(maxLCPValue);  }
        
        const tuz_byte* const src;
        const tuz_byte* const src_end;
        std::vector<TInt>   SA;     // suffix string
        std::vector<TInt>   R;      // rank of sstring
        std::vector<TLCPInt> LCP;    // lcp(i,i+1), longest common prefix between adjacent sstring
        inline size_t size()const { return src_end-src; }
    private:
        void _init(TLCPInt maxLCPValue);
    };
    
}
#endif //_tuz_sstring_h
