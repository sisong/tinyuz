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

static tuz_inline void _memset(tuz_byte* dst,tuz_byte v,tuz_dict_size_t len){
    while(len--) *dst++=v;
}

static tuz_inline void memmove_order(tuz_byte* dst,const tuz_byte* src,tuz_dict_size_t len){
    while (len--) *dst++=*src++;
}

#if 1
#   define  _memcpy        memmove_order
#   define  _memmove       memmove_order
#   define  _memmove_order memmove_order
#else

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
#endif

enum _TInputState{
    kInputState_default=0,
    kInputState_end,
    kInputState_error,
};

static tuz_BOOL _update_cache(tuz_TStream* self){
    //assert(self->_code_cache.cache_begin==self->_code_cache.cache_end);
    //    [                    cache  buf                        ]
    tuz_dict_size_t len=self->_code_cache.cache_begin;
    if (!self->read_code(self->inputStream,self->_code_cache.cache_buf,&len)){
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
    return len?tuz_TRUE:tuz_FALSE;
}

static tuz_inline tuz_byte _cache_read_1byte(tuz_TStream* self){
    if (self->_code_cache.cache_begin==self->_code_cache.cache_end){
        if(!_update_cache(self)) return 0;
    }
    return self->_code_cache.cache_buf[self->_code_cache.cache_begin++];
}

static tuz_try_inline tuz_byte _cache_read_1bit(tuz_TStream* self){
    if (self->_state.type_count==0){
        self->_state.type_count=8;
        self->_state.types=_cache_read_1byte(self);
    }
    {
        tuz_byte result=self->_state.types&1;
        self->_state.types>>=1;
        --self->_state.type_count;
        return result;
    }
}

static tuz_inline void _cache_push_1bit(tuz_TStream* self,tuz_byte bitv){
    //assert(self->_state.type_count<8);
    self->_state.types=(self->_state.types<<1)+bitv;
    ++self->_state.type_count;
}

//low to high bitmap: xx?xx?xx?xx? ...
static tuz_try_inline tuz_length_t _cache_unpack_len(tuz_TStream* self){
    tuz_length_t    v=0;
    do {
        v=(v<<2)+(_cache_read_1bit(self)<<1)+_cache_read_1bit(self);
    }while(_cache_read_1bit(self));
    return v;
}

static tuz_inline tuz_dict_size_t _cache_unpack_dict_pos(tuz_TStream* self){
    tuz_dict_size_t result=_cache_unpack_len(self);
    return (result<<8)|_cache_read_1byte(self);
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
    if (self->_state.dictType_pos<pos)
        pos=self->_dict.dict_cur+self->_state.dictType_pos;
    else
        pos=self->_state.dictType_pos-pos;
    if (len<=(self->_dict.dict_size-pos)){
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,len);
    }else{
        tuz_dict_size_t part_len=self->_dict.dict_size-pos;
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,part_len);
        _memcpy(cur_out_data+part_len,self->_dict.dict_buf,len-part_len);
    }
    return len;
}


void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_dict_size_t kCodeCacheSize,tuz_dict_size_t* out_dictSize){
    assert((read_code!=0)&&(codeCache!=0)&&(kCodeCacheSize!=0));
    _memset((tuz_byte*)self,0,sizeof(*self));
    {//init
        self->inputStream=inputStream;
        self->read_code=read_code;
        self->_code_cache.cache_begin=kCodeCacheSize;
        self->_code_cache.cache_end=kCodeCacheSize;
        self->_code_cache.cache_buf=codeCache;
    }
    {//dict_size
        const tuz_dict_size_t saved_size=_cache_unpack_dict_pos(self);
        self->_dict.dict_size=saved_size+1;        
        self->_state.type_count=0;
        if (out_dictSize)
            *out_dictSize=self->_dict.dict_size;  //out
    }
}

tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t* data_size){
    tuz_byte*  cur_out_data=out_data;
    tuz_dict_size_t dsize=*data_size;
#ifdef __RUN_MEM_SAFE_CHECK
    const tuz_BOOL isNeedOut=(dsize>0);
#endif
    do{
        copyDict_cmp_process:
        if (self->_state.dictType_len){ //copy from dict or out_data
        copyDict_process:
            if (dsize){
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
        
    copyLiteral_cmp_process:
        if (self->_state.literalType_len){
        copyLiteral_process:
            if (dsize){
                tuz_length_t cpyLen=(self->_state.literalType_len<dsize)?self->_state.literalType_len:dsize;
                self->_state.literalType_len-=cpyLen;
                dsize-=cpyLen;
                while (cpyLen--) {
                    *cur_out_data++=_cache_read_1byte(self);
                }
                goto copyLiteral_cmp_process;
            }else{
                break;
            }
        }
        
    type_process:
        {
            if (_cache_read_1bit(self)){//tuz_codeType_dict
                tuz_dict_size_t saved_dict_pos=_cache_unpack_dict_pos(self);
                if (saved_dict_pos){
                    tuz_dict_size_t   outed_size=(tuz_dict_size_t)(cur_out_data-out_data);
                    tuz_length_t      mlen=_cache_unpack_len(self);
                    self->_state.dictType_len=mlen+tuz_kMinDictMatchLen;
                    saved_dict_pos=(self->_dict.dict_size-saved_dict_pos);
#ifdef __RUN_MEM_SAFE_CHECK
                    if (saved_dict_pos>=self->_dict.dict_size) return tuz_DICT_POS_ERROR;
#endif
                    if (outed_size<self->_dict.dict_size-saved_dict_pos){
                        self->_state.dictType_pos=outed_size+saved_dict_pos;
                        self->_state.dictType_pos_inc=0;
                    }else{
                        self->_state.dictType_pos=self->_dict.dict_size;
                        self->_state.dictType_pos_inc=outed_size+saved_dict_pos-self->_dict.dict_size;
                    }
                    goto copyDict_process;
                }else {  
                    tuz_length_t saved_len=_cache_unpack_len(self);
                    if (saved_len){ //literalType
                        self->_state.literalType_len=saved_len+(tuz_kMinLiteralLen-1);
                        goto copyLiteral_process;
                    }else{ // ctrlType
                        const tuz_byte ctrlType=_cache_read_1byte(self);
                        if (tuz_ctrlType_streamEnd==ctrlType){ //stream end
                            self->_state.is_ctrlType_stream_end=tuz_TRUE;
                            //self->_state.type_count=0;
                            //if (self->_code_cache.cache_begin==self->_code_cache.cache_end)
                            //    _update_cache(self);
                            break;
                        }else if (tuz_ctrlType_clipEnd==ctrlType){ //clip end
                            self->_state.type_count=0;
                            goto type_process;
                        }else{
                            return tuz_CTRLTYPE_UNKNOW_ERROR;
                        }
                    }
                }
            }else{
                if (dsize){
                    *cur_out_data++=_cache_read_1byte(self);
                    --dsize;
                    goto type_process;
                }else{
                    _cache_push_1bit(self,tuz_codeType_data);
                    break;
                }
            }
        }
    }while(1);

//return_process:
    {
        if ((!self->_state.is_ctrlType_stream_end)&(out_data!=cur_out_data))
            _update_dict(self,out_data,cur_out_data);
        if (self->_code_cache.input_state==kInputState_error)
            return tuz_READ_CODE_ERROR;

        if (self->_state.is_ctrlType_stream_end){
            (*data_size)-=dsize;
            return tuz_STREAM_END;
        }else{
            return (dsize==0)?
            #ifdef __RUN_MEM_SAFE_CHECK
                (isNeedOut?tuz_OK:tuz_OUT_SIZE_OR_CODE_ERROR)
            #else
                tuz_OK
            #endif
                :tuz_CODE_ERROR;
        }
    }
}
