//  tuz_enc_code.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#ifndef _tuz_enc_code_h
#define _tuz_enc_code_h
#include "tuz_enc_types_private.h"
namespace _tuz_private{
    
    struct TTuzCode{
        explicit TTuzCode(std::vector<tuz_byte>& out_code)
        :code(out_code),type_count(0),_dictPos_back(1),_isHaveData_back(false){  }
        
        void outDictPos(size_t pos);
        void outDictSize(size_t dict_size);
        void outData(const tuz_byte* data,const tuz_byte* data_end);
        void outDict(size_t match_len,size_t dict_pos);
        void outCtrl_typesEnd();
        void outCtrl_streamEnd();
        void outCtrl_clipEnd();
        
        size_t getSavedDataBit(size_t data_len)const;
        size_t getSavedDictLenBit(size_t match_len)const;
        size_t getSavedDictPosBit(size_t pos,size_t back_pos,bool isHaveData)const;
        size_t getLiteralCostBit()const;
    private:
        std::vector<tuz_byte>& code;
        size_t    types_index;
        size_t    type_count;
        size_t    _dictPos_back;
        bool      _isHaveData_back;
        void outType(size_t bit1v);
        void outCtrl(tuz_TCtrlType ctrl);
        void outLen(size_t len,int packBit);
    };
    
}//end namespace
#endif //_tuz_enc_code_h
