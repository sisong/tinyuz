//  tuz_enc_code.h
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
#ifndef _tuz_enc_code_h
#define _tuz_enc_code_h
#include "tuz_enc_types_private.h"
namespace _tuz_private{
    
    struct TTuzCode:public ICode{
        explicit TTuzCode(std::vector<tuz_byte>& out_code)
        :code(out_code),type_count(0){  }
        
        void outLen(tuz_length_t len);
        void outDictPos(tuz_size_t len);
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
