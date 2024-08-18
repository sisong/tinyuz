//  tuz_sstring.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_sstring_h
#define _tuz_sstring_h
#include "tuz_enc_types_private.h"
namespace _tuz_private{
    
    struct TSuffixString{
        typedef int32_t  TInt;
        typedef uint16_t TLCPInt;
        TSuffixString(const tuz_byte* _src,const tuz_byte* _src_end,TLCPInt maxLCPValue,size_t _threadNum) //data size < 2G
        :src(_src),src_end(_src_end),threadNum(_threadNum){ _init(maxLCPValue);  }
        
        const tuz_byte* const src;
        const tuz_byte* const src_end;
        std::vector<TInt>     SA;     // suffix string
        std::vector<TInt>     R;      // rank of sstring
        std::vector<TLCPInt>  LCP;    // lcp(i,i+1), longest common prefix between adjacent sstring
        size_t                threadNum;
        inline size_t size()const { return src_end-src; }
        void clearMem();
    private:
        void _init(TLCPInt maxLCPValue);
    };
    
}
#endif //_tuz_sstring_h
