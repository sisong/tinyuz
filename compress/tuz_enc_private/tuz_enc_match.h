//  tuz_enc_match.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_match_h
#define _tuz_enc_match_h
#include "tuz_sstring.h"
namespace _tuz_private{
    struct TTuzCode;

    struct TMatch{
        explicit TMatch(const tuz_byte* data,const tuz_byte* data_end,
                        const TTuzCode& _coder,const tuz_TCompressProps& _props)
        :sstring(data,data_end,(TLCPInt)_props.maxSaveLength), coder(_coder),
        props(_props){ }
        bool match(const tuz_byte** out_matched,size_t* out_match_len,
                   const tuz_byte* cur);
        typedef TSuffixString::TInt     TInt;
        typedef uint32_t                TUInt;
        typedef TSuffixString::TLCPInt  TLCPInt;
    private:
        TSuffixString               sstring;
        const TTuzCode&             coder;
        const tuz_TCompressProps&   props;
        
        std::vector<TInt>           dictPos;
        std::vector<TLCPInt>        matchLen;
        void _cost_match(const TInt curString,const size_t curi,
                         size_t* curMinDictMatchLen,std::vector<TUInt>& cost);
        void _getCost(const tuz_byte* cur0);
        void _getCostByMatch(const tuz_byte* cur0,std::vector<TUInt>& cost);
      #if tuz_isNeedLiteralLine
        void _getCostByLiteralLen(const tuz_byte* cur0,std::vector<TUInt>& cost);
      #endif
    };
    
}
#endif //_tuz_enc_match_h
