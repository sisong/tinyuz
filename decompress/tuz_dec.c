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

#if (_IS_RUN_MEM_SAFE_CHECK)
#   define __RUN_MEM_SAFE_CHECK
#endif

static tuz_force_inline void _memset(tuz_byte* dst,tuz_byte v,tuz_size_t len){
    while(len--) *dst++=v;
}

static tuz_try_inline void memmove_order(tuz_byte* dst,const tuz_byte* src,tuz_size_t len){
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

static void memcpy_tiny8(tuz_byte* dst,const tuz_byte* src,tuz_size_t len){
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

tuz_BOOL _tuz_cache_update(struct _tuz_TInputCache* self){
    //    [                    cache  buf                        ]
    tuz_size_t len=self->cache_end;
    assert(len==self->cache_begin); //empty
    if (!self->read_code(self->inputStream,self->cache_buf,&len))
        len=0;
    //    |                                   len|               |
    self->cache_begin=0;
    self->cache_end=len;
    return len!=0;
}

tuz_fast_uint8 _tuz_cache_read_1byte(struct _tuz_TInputCache* self){
    if (self->cache_begin!=self->cache_end){
__cache_read_1byte:
        return self->cache_buf[self->cache_begin++];
    }
    if(_tuz_cache_update(self))
        goto __cache_read_1byte;
    else
        return 0;
}

#define _cache_read_1byte       _tuz_cache_read_1byte
#define _cache_read_typeBits    _cache_read_1byte

static tuz_force_inline void _cache_read_bytes(_tuz_TInputCache* self,tuz_byte* dst,tuz_length_t readLen){
    while (readLen--)
        *dst++=_cache_read_1byte(self);
}

static tuz_force_inline tuz_fast_uint8 _cache_read_1bit(tuz_TStream* self){
    tuz_fast_uint8 result;
    if (self->_state.type_count){
        --self->_state.type_count;
        result=self->_state.types;
__cache_read_1bit:
        self->_state.types=result>>1;
        return result&1;
    }else{
        self->_state.type_count=tuz_kMaxTypeBitCount-1;
        result=_cache_read_typeBits(&self->_code_cache);
        goto __cache_read_1bit;
    }
}

static tuz_force_inline tuz_fast_uint8 _cache_read_low3bit(tuz_TStream* self){
    tuz_fast_uint8 count=self->_state.type_count;
    tuz_fast_uint8 result=self->_state.types;
    if (count>=3){
        self->_state.type_count=count-3;
        self->_state.types=(result>>3);
        return result;
    }else{
        tuz_fast_uint8 v=_cache_read_typeBits(&self->_code_cache);
        self->_state.type_count=count+(tuz_kMaxTypeBitCount-3);
        self->_state.types=v>>(3-count);
        return result|(v<<count);
    }
}

static tuz_force_inline void _cache_push_1bit(tuz_TStream* self,tuz_fast_uint8 bitv){
    //assert(self->_state.type_count<tuz_kMaxTypeBitCount);
    self->_state.types=(self->_state.types<<1)+bitv;
    ++self->_state.type_count;
}

//low to high bitmap: xx?xx?xx?xx? ...
static tuz_try_inline tuz_length_t _cache_unpack_len(tuz_TStream* self){
    tuz_length_t    v=0;
    tuz_fast_uint8   low3bit;
    do {
        low3bit=_cache_read_low3bit(self);
        v=(v<<2)+(low3bit&0x3);
    }while(low3bit&0x4);
    return v;
}

static tuz_force_inline tuz_size_t _cache_unpack_dict_pos(tuz_TStream* self){
    tuz_size_t result=_cache_unpack_len(self);
    return (result<<8)|_cache_read_1byte(&self->_code_cache);
}


static void _update_dict(tuz_TStream *self,const tuz_byte* out_data,const tuz_byte* cur_out_data) {
    //  [               dict buf                 ]|[          out buf        ]
    //           |dict_cur               dict_size|out_data      cur_out_data]
    //           [         out buf         ]
    //           [                      out buf                ]
    const tuz_size_t out_len=(tuz_size_t)(cur_out_data-out_data);
    const tuz_size_t dict_size=self->_dict.dict_size;
    tuz_byte*  dict=self->_dict.dict_buf;
    if (self->_state.dictType_pos_inc>=out_len){
        self->_state.dictType_pos_inc-=out_len;
    }else{
        self->_state.dictType_pos-=(tuz_size_t)(out_len-self->_state.dictType_pos_inc);
        self->_state.dictType_pos_inc=0;
    }
    if (out_len>=dict_size){
        _memcpy(dict,cur_out_data-dict_size,dict_size);
        self->_dict.dict_cur=0;
    }else{
        tuz_size_t dict_cur=self->_dict.dict_cur;
        const tuz_size_t sub_len=dict_size-dict_cur;
        if (out_len<=sub_len){
            _memcpy(dict+dict_cur,out_data,out_len);
        }else{
            _memcpy(dict+dict_cur,out_data,sub_len);
            _memcpy(dict,out_data+sub_len,out_len-sub_len);
        }
        self->_dict.dict_cur=(out_len<=sub_len)?
                             (out_len+dict_cur):(out_len-sub_len);
    }
}

static tuz_size_t _copy_from_dict(tuz_TStream *self,tuz_byte* cur_out_data,tuz_size_t dsize) {
    // [                       dict buf                      ]
    //              dict_cur+dictType_pos|   <-- len -->  |
    //                                     dict_cur+dictType_pos|   <-- len -->  |
    //                       dict_cur+dictType_pos|   <-- len -->  |
    tuz_size_t len,pos;
    len=self->_dict.dict_size-self->_state.dictType_pos;
    len=(len<dsize)?len:dsize;
    len=(self->_state.dictType_len<len)?(tuz_size_t)self->_state.dictType_len:len;
    pos=self->_dict.dict_size-self->_dict.dict_cur;
    if (self->_state.dictType_pos<pos)
        pos=self->_dict.dict_cur+self->_state.dictType_pos;
    else
        pos=self->_state.dictType_pos-pos;
    if (len<=(self->_dict.dict_size-pos)){
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,len);
    }else{
        tuz_size_t part_len=self->_dict.dict_size-pos;
        _memcpy(cur_out_data,self->_dict.dict_buf+pos,part_len);
        _memcpy(cur_out_data+part_len,self->_dict.dict_buf,len-part_len);
    }
    return len;
}

void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_size_t kCodeCacheSize,tuz_size_t* out_dictSize){
    assert((read_code!=0)&&(codeCache!=0)&&(kCodeCacheSize>0));
    _memset((tuz_byte*)self,0,sizeof(*self));
    {//init
        self->_code_cache.inputStream=inputStream;
        self->_code_cache.read_code=read_code;
        self->_code_cache.cache_begin=kCodeCacheSize;
        self->_code_cache.cache_end=kCodeCacheSize;
        self->_code_cache.cache_buf=codeCache;
    }
    {//dict_size
        const tuz_size_t saved_size=_cache_unpack_dict_pos(self);
        self->_dict.dict_size=saved_size+1;
        self->_state.type_count=0;
        assert(self->_state.types==0);
        if (out_dictSize)
            *out_dictSize=self->_dict.dict_size; //out
    }
}

tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size){
    tuz_byte*  cur_out_data=out_data;
    tuz_size_t dsize=*data_size;
#ifdef __RUN_MEM_SAFE_CHECK
    const tuz_BOOL isNeedOut=(dsize>0);
#endif
    while(1){
        copyDict_cmp_process:
        if (self->_state.dictType_len){ //copy from dict or out_data
        copyDict_process:
            if (dsize){
                //  [                 dict buf                 ]|[          out buf        |              ]
                //             |dict_cur               dict_size|out_data      cur_out_data| <-- dsize -->|
                //       dictType_pos| <-- dictType_len --> |
                //                        dictType_pos|                  <--  dictType_len -->                 |
                //                                     dictType_pos| <-  dictType_len -> |
                tuz_size_t len;
                if (self->_state.dictType_pos<self->_dict.dict_size){
                    len=_copy_from_dict(self,cur_out_data,dsize);
                    self->_state.dictType_pos+=len;
                }else{
                    len=(self->_state.dictType_len<dsize)?(tuz_size_t)self->_state.dictType_len:dsize;
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
                _cache_read_bytes(&self->_code_cache,cur_out_data,cpyLen);
                dsize-=cpyLen;
                cur_out_data+=cpyLen;
                goto copyLiteral_cmp_process;
            }else{
                break;
            }
        }
        
    type_process:
        {
            if (_cache_read_1bit(self)==tuz_codeType_dict){
                tuz_size_t saved_dict_pos=_cache_unpack_dict_pos(self);
                if (saved_dict_pos){
                    const tuz_size_t outed_size=(tuz_size_t)(cur_out_data-out_data);
                    self->_state.dictType_len=_cache_unpack_len(self)+tuz_kMinDictMatchLen;
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
                        const tuz_fast_uint8 ctrlType=_cache_read_1byte(&self->_code_cache);
                        if (tuz_ctrlType_clipEnd==ctrlType){ //clip end
                            self->_state.type_count=0;
                            goto type_process;
                        }else if (tuz_ctrlType_streamEnd==ctrlType){ //stream end
                            (*data_size)-=dsize;
                            return tuz_STREAM_END;
                        }else{
                            return _tuz_cache_success_finish(&self->_code_cache)?
                                        tuz_CTRLTYPE_UNKNOW_ERROR:tuz_READ_CODE_ERROR;
                        }
                    }
                }
            }else{
                if (dsize){
                    *cur_out_data++=_cache_read_1byte(&self->_code_cache);
                    --dsize;
                    goto type_process;
                }else{
                    _cache_push_1bit(self,tuz_codeType_data);
                    break;
                }
            }
        }
    }//end while

//return_process:
    {
        assert(dsize==0);
        if (out_data!=cur_out_data)
            _update_dict(self,out_data,cur_out_data);
        if (!_tuz_cache_success_finish(&self->_code_cache))
            return tuz_READ_CODE_ERROR;

        #ifdef __RUN_MEM_SAFE_CHECK
            return isNeedOut?tuz_OK:tuz_OUT_SIZE_OR_CODE_ERROR;
        #else
            return tuz_OK;
        #endif
    }
}


#ifdef __RUN_MEM_SAFE_CHECK
#   define __SafeTest(t) t 
#else
#   define __SafeTest(t) tuz_TRUE 
#endif

#define _mem_read_1byte(result) {   \
    if (__SafeTest(in_code!=in_code_end)){ \
        result=*in_code++;          \
    }else{                          \
        return tuz_READ_CODE_ERROR; \
    } }

#define _mem_read_codeType(result) { \
    if (type_count){        \
        result=types;       \
        --type_count;       \
        types>>=1;          \
    }else if (__SafeTest(in_code!=in_code_end)){    \
        result=*in_code++;  \
        type_count=tuz_kMaxTypeBitCount-1;          \
        types=result>>1;    \
    }else{                  \
        return tuz_READ_CODE_ERROR;                 \
    } }

#define _mem_read_low3bit(result) { \
    tuz_fast_uint8 count=type_count;\
    result=types;                   \
    if (count>=3){                  \
        type_count=count-3;         \
        types=(result>>3);          \
    }else{ \
        tuz_fast_uint8 v;           \
        _mem_read_1byte(v);         \
        type_count=count+(tuz_kMaxTypeBitCount-3);  \
        types=v>>(3-count);         \
        result|=(v<<count);         \
    } }

static tuz_TResult __mem_unpack_len(const tuz_byte** pin_code,const tuz_byte* in_code_end,
                                    tuz_fast_uint8* ptypes,tuz_fast_uint8* ptype_count,tuz_length_t* presult){
    const tuz_byte* in_code=*pin_code;
    tuz_fast_uint8  types=*ptypes;
    tuz_fast_uint8  type_count=*ptype_count;
    tuz_fast_uint8  low3bit;
    tuz_length_t    result=0;
    do {
        _mem_read_low3bit(low3bit);
        result=(result<<2)+(low3bit&0x3);
    }while(low3bit&0x4);
    *pin_code=in_code;
    *ptypes=types;
    *ptype_count=type_count;
    *presult=result;
    return tuz_OK;
}

#define _mem_unpack_len(result) { \
    if (tuz_OK!=__mem_unpack_len(&in_code,in_code_end,&types,&type_count,&result)) return tuz_READ_CODE_ERROR; }

#define _mem_unpack_dict_pos(result) {      \
    _mem_unpack_len(result);                \
    if (__SafeTest(in_code!=in_code_end)){  \
        result=(result<<8)|(*in_code++);    \
    }else{                                  \
        return tuz_READ_CODE_ERROR;         \
    } }

tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size){
    tuz_byte*  cur_out_data=out_data;
    const tuz_byte* in_code_end=in_code+code_size;
    tuz_size_t dsize=*data_size;
    tuz_fast_uint8  types=0;
    tuz_fast_uint8  type_count=0;
    {//dict_size
        tuz_size_t _dict_size;
        _mem_unpack_dict_pos(_dict_size);
        //_dict_size+=1;
        type_count=0;
        assert(types==0);
    }
    while(1){
        tuz_fast_uint8 codeType;
        _mem_read_codeType(codeType);
        if ((codeType&1)==tuz_codeType_dict){
            tuz_size_t saved_dict_pos; 
            _mem_unpack_dict_pos(saved_dict_pos);
            if (saved_dict_pos){
                tuz_size_t dictType_len;
                _mem_unpack_len(dictType_len);
                dictType_len+=tuz_kMinDictMatchLen;
#ifdef __RUN_MEM_SAFE_CHECK
                if (saved_dict_pos>(tuz_size_t)(cur_out_data-out_data)) return tuz_DICT_POS_ERROR;
                if (dictType_len>dsize) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                _memmove_order(cur_out_data,cur_out_data-saved_dict_pos,dictType_len);
                cur_out_data+=dictType_len;
                dsize-=dictType_len;
            }else {  
                tuz_length_t literalType_len;
                _mem_unpack_len(literalType_len);
                if (literalType_len){ //literalType
                    literalType_len+=(tuz_kMinLiteralLen-1);
#ifdef __RUN_MEM_SAFE_CHECK
                    if (literalType_len>(tuz_size_t)(in_code_end-in_code)) return tuz_READ_CODE_ERROR;
                    if (literalType_len>dsize) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                    _memcpy(cur_out_data,in_code,literalType_len);
                    cur_out_data+=literalType_len;
                    in_code+=literalType_len;
                    dsize-=literalType_len;
                }else{ // ctrlType
                    tuz_fast_uint8 ctrlType;
                    _mem_read_1byte(ctrlType);
                    if (tuz_ctrlType_clipEnd==ctrlType){ //clip end
                        type_count=0;
                    }else if (tuz_ctrlType_streamEnd==ctrlType){ //stream end
                        (*data_size)-=dsize;
                        return tuz_STREAM_END;
                    }else{
                        return tuz_CTRLTYPE_UNKNOW_ERROR;
                    }
                }
            }
        }else{
            if (__SafeTest(dsize)){
                tuz_fast_uint8 data;
                _mem_read_1byte(data);
                *cur_out_data++=data;
                --dsize;
            }else{
                return tuz_OUT_SIZE_OR_CODE_ERROR;
            }
        }
    }//end while
}