//  tuz_enc_match.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_match_h
#define _tuz_enc_match_h
#include "tuz_sstring.h"
namespace _tuz_private{
    
    struct TMatch{
        explicit TMatch(const tuz_byte* data,const tuz_byte* data_end,
                        const ICode& _coder,const tuz_TCompressProps& _props)
        :sstring(data,data_end,(TLCPInt)_props.maxSaveLength), coder(_coder),
        props(_props){ }
        bool match(const tuz_byte** out_matched,tuz_length_t* out_match_len,
                   const tuz_byte* cur,size_t unmatched_len);
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
        void _getCost(const tuz_byte* cur0,size_t unmatched_len);
    };
    
}
#endif //_tuz_enc_match_h
