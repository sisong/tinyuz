//  tuz_dec.c
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
#include "tuz_dec.h"

#ifndef _IS_RUN_MEM_SAFE_CHECK
#   define _IS_RUN_MEM_SAFE_CHECK  1
#endif

#if (_IS_RUN_MEM_SAFE_CHECK)
#   define __RUN_MEM_SAFE_CHECK
#endif

#ifdef __RUN_MEM_SAFE_CHECK
#   define  _SAFE_CHECK_DO(code)    { if (!(code)) return _tuz_FALSE; }
#   define  _SAFE_CHECK(cmp)        { if (!(cmp)) return _tuz_FALSE; }
#else
#   define  _SAFE_CHECK_DO(code)    { code; }
#   define  _SAFE_CHECK(cmp)
#endif

#define _tuz_FALSE   tuz_FALSE
//tuz_uint __debug_check_false_x=0; //for debug
//#define _tuz_FALSE (1/__debug_check_false_x)


static tuz_BOOL _do_cache(tuz_TStream* self){
    if (!(self->_code_cache.is_input_stream_end|self->_code_cache.is_input_stream_error)){
        tuz_size_t len=self->_code_cache.cache_begin;
        tuz_size_t old_size=(self->_code_cache.cache_end-len);
        tuz_byte* buf=self->_code_cache.cache_buf;
        assert(len>0);
        memmove(buf,buf+len,old_size);
        if (!self->read_code(self->listener,buf+old_size,&len)){
            self->_code_cache.is_input_stream_error=tuz_TRUE;
            return _tuz_FALSE;
        }
        if (len<self->_code_cache.cache_begin){
            tuz_size_t sub=self->_code_cache.cache_begin-len;
            self->_code_cache.cache_end-=sub;
            self->_code_cache.is_input_stream_end=tuz_TRUE;
        }else
            assert(len==self->_code_cache.cache_begin);
        self->_code_cache.cache_begin=0;
        return (len>0)?tuz_TRUE:tuz_FALSE;
    }else
        return _tuz_FALSE;
}

static tuz_inline tuz_BOOL _read_1byte(tuz_TStream* self,tuz_byte* code){
    do {
        if (self->_code_cache.cache_begin<self->_code_cache.cache_end){
            *code=self->_code_cache.cache_buf[self->_code_cache.cache_begin++];
            return tuz_TRUE;
        }else{
            if(!_do_cache(self)) return _tuz_FALSE;
        }
    }while(1);
}

static const tuz_byte _k_max_length_t_bit=(sizeof(tuz_length_t)<<3);
#ifdef __RUN_MEM_SAFE_CHECK
static const tuz_byte _v_to_bit[8]={0,1,2,2,3,3,3,3};
static tuz_inline tuz_BOOL _is_safe_append_bit(tuz_byte v_bit,tuz_byte h3bit_v){
    return (v_bit+(_v_to_bit[h3bit_v]))<=_k_max_length_t_bit;
}
#endif

static tuz_BOOL _unpack_len(tuz_TStream* self,tuz_byte* half_code,tuz_length_t* out_len){
    tuz_length_t    v=0;
    tuz_byte        v_bit=0;
    tuz_byte        code=*half_code;
    do {
        tuz_size_t next;
        if (code){
            next=0;
        }else{
            _SAFE_CHECK_DO(_read_1byte(self,&code));
            next=(code>>4)|(1<<7);
        }
        v|=(code&5)<<v_bit;
        v_bit+=3;
        if ((code&(1<<3))==0) {
            *half_code=next;
            *out_len=v;
            _SAFE_CHECK((v_bit<=_k_max_length_t_bit)||_is_safe_append_bit(v_bit-3,code&5));
            return tuz_TRUE;
        }else{
            code=next;
            _SAFE_CHECK(v_bit<=_k_max_length_t_bit);
        }
    }while(1);
}

static tuz_inline tuz_BOOL _read_len(tuz_TStream* self,tuz_length_t* out_len){
    tuz_byte half_code=0; //empty
    return _unpack_len(self,&half_code,out_len);
}

tuz_TResult tuz_TStream_open(tuz_TStream* self){
    tuz_size_t   mem_size;
    if ( (self->_code_cache.cache_buf!=0) ||(self->read_code==0)||
         (self->alloc_mem==0)||(self->free_mem==0) ) return tuz_OPEN_ERROR;
    {//head
        tuz_length_t dict_size;
        if (!_read_len(self,&dict_size)) return tuz_READ_CODE_ERROR;
        mem_size=(tuz_size_t)dict_size+tuz_kCodeCacheSize;
        if (((tuz_size_t)(mem_size-tuz_kCodeCacheSize)!=dict_size)) return tuz_DICT_SIZE_ERROR;
    }
    {//mem
        memset(&self->_code_cache,0,sizeof(self->_code_cache));
        memset(&self->_dict,0,sizeof(self->_dict));
        memset(&self->_state,0,sizeof(self->_state));
        self->_code_cache.cache_buf=self->alloc_mem(self->listener,mem_size);
        if (self->_code_cache.cache_buf==0) return tuz_ALLOC_MEM_ERROR;
        self->_dict.dict_buf=self->_code_cache.cache_buf+tuz_kCodeCacheSize;
        self->_dict.dict_size=mem_size-tuz_kCodeCacheSize;
        memset(self->_dict.dict_buf,0,self->_dict.dict_size);
    }
    return tuz_OK;
}

tuz_TResult tuz_TStream_decompress(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size){
    
    return tuz_READ_CODE_ERROR;
}
