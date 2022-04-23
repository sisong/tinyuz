//  tuz_enc_code.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_code_h
#define _tuz_enc_code_h
#include "tuz_enc_types_private.h"
namespace _tuz_private{
    
    struct TTuzCode:public ICode{
        explicit TTuzCode(std::vector<tuz_byte>& out_code)
        :code(out_code),type_count(0){  }
        
        void outLen(tuz_length_t len);
        void outDictPos(tuz_size_t pos);
        void outDictSize(tuz_size_t dict_size);
        void outData(const tuz_byte* data,const tuz_byte* data_end);
        void outDict(tuz_length_t match_len,tuz_size_t dict_pos);
        void outCtrl_typesEnd();
        void outCtrl_streamEnd();
        void outCtrl_clipEnd();
        
        virtual size_t getSavedDataBit(tuz_length_t data_len)const;
        virtual size_t getSavedDictLenBit(tuz_length_t match_len)const;
        virtual size_t getSavedDictPosBit(tuz_size_t pos)const;
    private:
        std::vector<tuz_byte>& code;
        size_t    types_index;
        size_t    type_count;
        void outType(size_t bit1v);
        void outCtrl(tuz_TCtrlType ctrl);
    };
    
}
#endif //_tuz_enc_code_h
