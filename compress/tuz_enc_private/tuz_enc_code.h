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
        explicit TTuzCode(std::vector<tuz_byte>& out_code,bool _isNeedLiteralLine)
        :code(out_code),isNeedLiteralLine(_isNeedLiteralLine),
        type_count(0),_dictPos_back(1),_dict_size_max(tuz_kMinOfDictSize),_isHaveData_back(false){ _init_2bit(); }
        
        void outDictSize(size_t dict_size);
        void outData(const tuz_byte* data,const tuz_byte* data_end);
        void outDict(size_t match_len,size_t dict_pos);
        void outCtrl_typesEnd();
        void outCtrl_streamEnd();
        void outCtrl_clipEnd();
        const size_t getCurDictSizeMax() { return _dict_size_max; }
        
        inline size_t getSavedDataBit(size_t data_len)const{
            if (isNeedLiteralLine&&(data_len>=tuz_kMinLiteralLen)){
                size_t len=data_len-tuz_kMinLiteralLen;
                return data_len*8+1+2+8+__getDictPosLenBit(len);
            }else
                return data_len*(1+8);
        }
        template<bool isSavedSamePos> inline 
        size_t getSavedDictLenBit(size_t match_len,size_t pos)const{
            size_t len=match_len-(((!isSavedSamePos)&&(pos>=tuz_kBigPosForLen))?1:0)-tuz_kMinDictMatchLen;
            if (len<_len2bit_count) return _len2bit[len];
            else if (len<match_len) return _getSavedDictLenBit(len);
            else return match_len*(1+8+1)+128; //fail pos,return a large bit count
        }
        template<bool isSamePos> inline 
        size_t getSavedDictPosBit(size_t pos,size_t isHaveData)const{
            if (isSamePos&&isHaveData) return 2;
            else if (pos<_pos2bit_count) return _pos2bit[pos]+isHaveData;
            else return _getSavedDictPosBit(pos)+isHaveData;
        }
    private:
        void outDictPos(size_t pos);
        enum { _len2bit_count=1024*8, _pos2bit_count=1024*32 };
        tuz_byte _pos2bit[_pos2bit_count];
        tuz_byte _len2bit[_len2bit_count];
        void _init_2bit(){
            for (size_t i=0;i<_pos2bit_count;++i) _pos2bit[i]=(tuz_byte)_getSavedDictPosBit(i); 
            for (size_t i=0;i<_len2bit_count;++i) _len2bit[i]=(tuz_byte)_getSavedDictLenBit(i); }
        std::vector<tuz_byte>& code;
        bool                   isNeedLiteralLine;
        size_t    types_index;
        size_t    type_count;
        size_t    _dictPos_back;
        size_t    _dict_size_max;
        bool      _isHaveData_back;
        void outType(size_t bit1v);
        void outCtrl(tuz_TCtrlType ctrl);
        void outLen(size_t len,size_t packBit);
        size_t _getSavedDictPosBit(size_t pos)const;
        size_t _getSavedDictLenBit(size_t len)const;
        size_t __getDictPosLenBit(size_t len)const;
    };
    
}//end namespace
#endif //_tuz_enc_code_h
