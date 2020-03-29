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
#include "tuz_types_private.h"

#ifndef _IS_RUN_MEM_SAFE_CHECK
#   define _IS_RUN_MEM_SAFE_CHECK  1
#endif

#if (_IS_RUN_MEM_SAFE_CHECK)
#   define __RUN_MEM_SAFE_CHECK
#endif

#ifdef __RUN_MEM_SAFE_CHECK
#   define  _SAFE_CHECK_DO(code)    { if (!(code)) return tuz_FALSE; }
#   define  _SAFE_CHECK(cmp)        { if (!(cmp)) return tuz_FALSE; }
#else
#   define  _SAFE_CHECK_DO(code)    { code; }
#   define  _SAFE_CHECK(cmp)
#endif

#define  _memcpy    memcpy_tiny8
#define  _memmove(dst,_src,len){  \
    const tuz_byte* src=_src;   \
    if (src-dst>=8) \
        memcpy_tiny8(dst,src,len); \
    else    \
        memmove_order(dst,src,len); }
#define  _memmove_order(dst,_src,len){  \
    const tuz_byte* src=_src;   \
    if (dst-src>=8) \
        memcpy_tiny8(dst,src,len); \
    else    \
        memmove_order(dst,src,len); }

static tuz_inline void _memset(tuz_byte* dst,tuz_byte v,tuz_dict_size_t len){
    while(len--) *dst++=v;
}

static tuz_inline void memmove_order(tuz_byte* dst,const tuz_byte* src,tuz_dict_size_t len){
    while (len--) *dst++=*src++;
}

struct _t_data8{ tuz_byte _[8]; };
struct _t_data4{ tuz_byte _[4]; };
struct _t_data2{ tuz_byte _[2]; };
#define _COPY_t(dst,src,T)  { *(T*)(dst)=*(const T*)(src); }
#define _COPY_8(dst,src)    _COPY_t(dst,src,struct _t_data8)
#define _COPY_4(dst,src)    _COPY_t(dst,src,struct _t_data4)
#define _COPY_2(dst,src)    _COPY_t(dst,src,struct _t_data2)

static void memcpy_tiny8(tuz_byte* dst,const tuz_byte* src,tuz_dict_size_t len){
case_process:
    switch (len) {
        /*case 16: case 15: case 14: case 13:{ len-=8;
            _COPY_8(dst,src); _COPY_8(dst+len,src+len); } return;
        case 12: case 11: case 10: case 9:{ len-=4;
            _COPY_8(dst,src); _COPY_4(dst+len,src+len); } return;*/
        case 8: _COPY_8(dst,src); return;
        case 7: case 6: case 5:{ len-=4;
            _COPY_4(dst,src); _COPY_4(dst+len,src+len); } return;
        case 4: _COPY_4(dst,src); return;
        case 3: *dst++=*src++;
        case 2: _COPY_2(dst,src); return;
        case 1: *dst=*src;
        case 0: return;
        default: {
            do {
                _COPY_8(dst,src);
                dst+=8; src+=8; len-=8;
            }while (len>=8);
            goto case_process;
        }
    }
}

enum _TInputState{
    kInputState_default=0,
    kInputState_end,
    kInputState_error,
};

static tuz_BOOL _update_cache(tuz_TStream* self){
    if (self->_code_cache.input_state==kInputState_default){
        //    [                      cache  buf                           ]
        //    | <-- len --> |cache_begin                         cache_end|
        tuz_dict_size_t len=self->_code_cache.cache_begin;
        tuz_dict_size_t old_size=(self->_code_cache.cache_end-len);
        tuz_byte* buf=self->_code_cache.cache_buf;
        //assert(len>0);
        if (old_size>0)
            _memmove(buf,buf+len,old_size);
        //    |                                             | <-- len --> |
        if (!self->read_code(self->inputStream,buf+old_size,&len)){
            self->_code_cache.input_state=kInputState_error;
            return tuz_FALSE;
        }
        if (len<self->_code_cache.cache_begin){
            //|                                             |        |
            tuz_dict_size_t sub=self->_code_cache.cache_begin-len;
            self->_code_cache.cache_end-=sub;
            self->_code_cache.input_state=kInputState_end;
        }//else assert(len==self->_code_cache.cache_begin);
        self->_code_cache.cache_begin=0;
        return (len>0)?tuz_TRUE:tuz_FALSE;
    }else
        return tuz_FALSE;
}

static tuz_inline tuz_BOOL _cache_read_bytes(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t len){
    while (len>0) {
        tuz_dict_size_t clen=self->_code_cache.cache_end-self->_code_cache.cache_begin;
        if (clen>0){
            if (clen>len) clen=len;
            _memcpy(out_data,self->_code_cache.cache_buf+self->_code_cache.cache_begin,clen);
            self->_code_cache.cache_begin+=clen;
            out_data+=clen;
            len-=clen;
        }else{
            if(!_update_cache(self)) return tuz_FALSE;
        }
    }
    return tuz_TRUE;
}

static tuz_inline tuz_BOOL _cache_read_1byte(tuz_TStream* self,tuz_byte* code){
    do {
        if (self->_code_cache.cache_begin<self->_code_cache.cache_end){
            *code=self->_code_cache.cache_buf[self->_code_cache.cache_begin++];
            return tuz_TRUE;
        }else{
            if(!_update_cache(self)) return tuz_FALSE;
        }
    }while(1);
}

/*
#ifdef __RUN_MEM_SAFE_CHECK
static const tuz_byte _k_max_length_t_bit=(sizeof(tuz_length_t)<<3);
static const tuz_byte _v_to_bit[8]={0,1,2,2,3,3,3,3};
static tuz_inline tuz_BOOL _is_safe_append_bit(tuz_byte v_bit,tuz_byte h3bit_v){
    return (v_bit+(_v_to_bit[h3bit_v]))<=_k_max_length_t_bit;
}
#endif
*/

//low to high bitmap: xxx?xxx? xxx?xxx? ...
static tuz_BOOL _cache_unpack_len(tuz_TStream* self,tuz_byte* half_code,tuz_length_t* out_len){
    tuz_length_t    v=0;
    tuz_byte        v_bit=0;
    tuz_byte        code=*half_code;
    do {
        tuz_dict_size_t next;
        if (code){
            next=0;
        }else{
            _SAFE_CHECK_DO(_cache_read_1byte(self,&code));
            next=(code>>4)|(1<<7);
        }
        v|=(code&7)<<v_bit;
        v_bit+=3;
        if ((code&(1<<3))==0) {
            *half_code=next;
            *out_len=v;
            //_SAFE_CHECK((v_bit<=_k_max_length_t_bit)||_is_safe_append_bit(v_bit-3,code&7));
            return tuz_TRUE;
        }else{
            code=next;
            //_SAFE_CHECK(v_bit<=_k_max_length_t_bit);
        }
    }while(1);
}

static void _update_dict(tuz_TStream *self,const tuz_byte* out_data,const tuz_byte* cur_out_data) {
    //  [               dict buf                 ]|[          out buf        ]
    //           |dict_cur               dict_size|out_data      cur_out_data]
    //           [         out buf         ]
    //           [                      out buf                ]
    const tuz_dict_size_t out_len=(tuz_dict_size_t)(cur_out_data-out_data);
    const tuz_dict_size_t dict_size=self->_dict.dict_size;
    tuz_byte*  dict=self->_dict.dict_buf;
    if (self->_state.dictType_pos_inc>=out_len){
        self->_state.dictType_pos_inc-=out_len;
    }else{
        self->_state.dictType_pos-=(tuz_dict_size_t)(out_len-self->_state.dictType_pos_inc);
        self->_state.dictType_pos_inc=0;
    }
    if (out_len>=dict_size){
        _memcpy(dict,cur_out_data-dict_size,dict_size);
        self->_dict.dict_cur=0;
    }else{
        tuz_dict_size_t dict_cur=self->_dict.dict_cur;
        if (out_len<=(dict_size-dict_cur)){
            _memcpy(dict+dict_cur,out_data,out_len);
        }else{
            const tuz_dict_size_t sub_len=dict_size-dict_cur;
            _memcpy(dict+dict_cur,out_data,sub_len);
            _memcpy(dict,out_data+sub_len,out_len-sub_len);
        }
        self->_dict.dict_cur=(out_len<(dict_size-dict_cur))?
                             (out_len+dict_cur):(out_len-(dict_size-dict_cur));
    }
}

static tuz_dict_size_t _copy_from_dict(tuz_TStream *self,tuz_byte* cur_out_data,tuz_dict_size_t dsize) {
    // [                       dict buf                      ]
    //              dict_cur+dictType_pos|   <-- len -->  |
    //                                     dict_cur+dictType_pos|   <-- len -->  |
    //                       dict_cur+dictType_pos|   <-- len -->  |
    tuz_dict_size_t max_len,len,pos;
    max_len=self->_dict.dict_size-self->_state.dictType_pos;
    max_len=(max_len<dsize)?max_len:dsize;
    len=(self->_state.dictType_len<max_len)?(tuz_dict_size_t)self->_state.dictType_len:max_len;
    pos=self->_dict.dict_size-self->_dict.dict_cur;
    if (self->_state.dictType_pos>=pos)
        pos=self->_state.dictType_pos-pos;
    else
        pos=self->_dict.dict_cur+self->_state.dictType_pos;
    if (len<=(self->_dict.dict_size-pos)){
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,len);
    }else{
        tuz_dict_size_t part_len=self->_dict.dict_size-pos;
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,part_len);
        _memcpy(cur_out_data+part_len,self->_dict.dict_buf,len-part_len);
    }
    return len;
}


tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* codeCache,tuz_dict_size_t kCodeCacheSize,tuz_dict_size_t* out_dictSize){
    if ((read_code==0)||(codeCache==0)||(kCodeCacheSize==0))
        return tuz_OPEN_ERROR;
    {//init
        _memset((tuz_byte*)self,0,sizeof(*self));
        self->inputStream=inputStream;
        self->read_code=read_code;
        self->_code_cache.cache_buf=codeCache;
        self->_code_cache.cache_begin=kCodeCacheSize;
        self->_code_cache.cache_end=kCodeCacheSize;
    }
    {//head
        tuz_byte half_code=0; //empty
        //kMinDictMatchLen
        tuz_length_t saved_len;
        if (!_cache_unpack_len(self,&half_code,&saved_len)) return tuz_OPEN_ERROR;
        self->kMinDictMatchLen=(tuz_byte)saved_len;
        if ((saved_len==0)||(self->kMinDictMatchLen!=saved_len)) return tuz_OPEN_ERROR;
        //dict_size
        if (!_cache_unpack_len(self,&half_code,&saved_len)) return tuz_OPEN_ERROR;
        self->_dict.dict_size=(tuz_dict_size_t)saved_len;
        if ((saved_len==0)||(self->_dict.dict_size!=saved_len)) return tuz_OPEN_ERROR;
    }
    if (out_dictSize)
        *out_dictSize=self->_dict.dict_size;  //out
    return tuz_OK;
}

tuz_TResult tuz_TStream_decompress_begin(tuz_TStream* self,tuz_byte* dict_buf,tuz_dict_size_t dictSize){
    if ((dict_buf==0)||(dictSize==0)||(dictSize<self->_dict.dict_size)||
        (self->_dict.dict_buf!=0)) return tuz_DICT_BUF_ERROR;
    self->_dict.dict_buf=dict_buf;
    self->_dict.dict_size=dictSize;
    return tuz_OK;
}


#define _check_return(v)  { if (!(v)) return tuz_CODE_ERROR; }

tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t* data_size){
    tuz_byte*  cur_out_data=out_data;
    tuz_dict_size_t dsize=*data_size;
#ifdef __RUN_MEM_SAFE_CHECK
    if ((dsize==0)&&((self->_state.dictType_len|self->_state.codeType_len)>0)) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
    do{
        copyDict_cmp_process:
        if (self->_state.dictType_len>0){ //copy from dict or out_data
        copyDict_process:
            if (dsize>0){
                //  [                 dict buf                 ]|[          out buf        |              ]
                //             |dict_cur               dict_size|out_data      cur_out_data| <-- dsize -->|
                //       dictType_pos| <-- dictType_len --> |
                //                        dictType_pos|                  <--  dictType_len -->                 |
                //                                     dictType_pos| <-  dictType_len -> |
                tuz_dict_size_t len;
                if (self->_state.dictType_pos<self->_dict.dict_size){
                    len=_copy_from_dict(self,cur_out_data,dsize);
                    self->_state.dictType_pos+=len;
                }else{
                    len=(self->_state.dictType_len<dsize)?(tuz_dict_size_t)self->_state.dictType_len:dsize;
                    _memmove_order(cur_out_data,out_data+self->_state.dictType_pos_inc,len);
                    self->_state.dictType_pos_inc+=len;
                }
                self->_state.dictType_len-=len;
                cur_out_data+=len;
                dsize-=len;
                goto copyDict_cmp_process;
            }else{
                break;
            }
        }
    copyCode_cmp_process:
        if (self->_state.codeType_len>0){// copy from code
        copyCode_process:
            if (dsize>0){
                const tuz_dict_size_t len=(self->_state.codeType_len<dsize)?(tuz_dict_size_t)self->_state.codeType_len:dsize;
                _check_return(_cache_read_bytes(self,cur_out_data,len));
                self->_state.codeType_len-=len;
                cur_out_data+=len;
                dsize-=len;
                goto copyCode_cmp_process;
            }else{
                break;
            }
        }
        if (self->_state.type_count>0){ //next type
        type_process: {
            tuz_length_t saved_len;
            const tuz_TCodeType type=(tuz_TCodeType)(self->_state.types&1);
            self->_state.types>>=1;
            --self->_state.type_count;
            _check_return(_cache_unpack_len(self,&self->_state.half_code,&saved_len));
            if (type==tuz_codeType_dict){
                tuz_dict_size_t   outed_size=(tuz_dict_size_t)(cur_out_data-out_data);
                tuz_length_t saved_dict_pos;
                _check_return(_cache_unpack_len(self,&self->_state.half_code,&saved_dict_pos));
#ifdef __RUN_MEM_SAFE_CHECK
                if (saved_dict_pos>=self->_dict.dict_size) return tuz_DICT_POS_ERROR;
#endif
                self->_state.dictType_len=saved_len+self->kMinDictMatchLen;
                saved_dict_pos=(self->_dict.dict_size-1-saved_dict_pos);
                if (outed_size<self->_dict.dict_size-saved_dict_pos){
                    self->_state.dictType_pos=outed_size+saved_dict_pos;
                    self->_state.dictType_pos_inc=0;
                }else{
                    self->_state.dictType_pos=self->_dict.dict_size;
                    self->_state.dictType_pos_inc=outed_size+saved_dict_pos-self->_dict.dict_size;
                }
                goto copyDict_process;
            }else{ //==tuz_codeType_data or ctrlType
                if (saved_len!=0){
                    self->_state.codeType_len=saved_len;
                    goto copyCode_process;
                }else{ // ctrlType
                    tuz_byte ctrlType;
                    _check_return(_cache_read_1byte(self,&ctrlType));
                    if (tuz_ctrlType_streamEnd==ctrlType){ //stream end
#ifdef __RUN_MEM_SAFE_CHECK
                        if (self->_state.is_ctrlType_stream_end) return tuz_CTRLTYPE_STREAM_END_ERROR;
#endif
                        self->_state.type_count=0;
                        self->_state.half_code=0;
                        self->_state.is_ctrlType_stream_end=tuz_TRUE;
                        _update_cache(self); //for is_input_stream_end
                        break;
                    }else if (tuz_ctrlType_clipEnd==ctrlType){ //clip end
                        self->_state.half_code=0;
                        goto types_process;
                    }else{
                        return tuz_CTRLTYPE_UNKNOW_ERROR;
                    }
                }
            } }
        }else{
        types_process:
            _check_return(_cache_read_1byte(self,&self->_state.types));
            self->_state.type_count=8;
            goto type_process;
        }
    }while(1);

//return_process:
    {
        if ((!self->_state.is_ctrlType_stream_end)&(out_data!=cur_out_data))
            _update_dict(self,out_data,cur_out_data);

        if (self->_code_cache.input_state==kInputState_error){
            return tuz_READ_CODE_ERROR;
        }else if (self->_state.is_ctrlType_stream_end){
            if ((self->_code_cache.input_state==kInputState_end)
                &&(self->_code_cache.cache_begin==self->_code_cache.cache_end)){
                (*data_size)-=dsize;
                return tuz_STREAM_END;
            }else{
                return tuz_CTRLTYPE_STREAM_END_ERROR;
            }
        }else if (dsize==0){
            return tuz_OK;
        }else{
            return tuz_CODE_ERROR;
        }
    }
}
