//  tuz_dec.c
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_dec.h"
#include "tuz_types_private.h"

#if (_IS_RUN_MEM_SAFE_CHECK)
#   define __RUN_MEM_SAFE_CHECK
#endif

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

static tuz_try_inline tuz_fast_uint8 _cache_read_lowbits(tuz_TStream* self,tuz_fast_uint8 bitCount){
    tuz_fast_uint8 count=self->_state.type_count;
    tuz_fast_uint8 result=self->_state.types;
    if (count>=bitCount){
        self->_state.type_count=count-bitCount;
        self->_state.types=(result>>bitCount);
        return result;
    }else{
        tuz_fast_uint8 v=_cache_read_typeBits(&self->_code_cache);
        bitCount-=count;
        self->_state.type_count=tuz_kMaxTypeBitCount-bitCount;
        self->_state.types=v>>bitCount;
        return result|(v<<count);
    }
}

static tuz_force_inline tuz_fast_uint8 _cache_read_1bit(tuz_TStream* self){
    return _cache_read_lowbits(self,1)&0x1;
}

static tuz_force_inline void _cache_push_1bit(tuz_TStream* self,tuz_fast_uint8 bitv){
    //assert(self->_state.type_count<tuz_kMaxTypeBitCount);
    self->_state.types=(self->_state.types<<1)+bitv;
    ++self->_state.type_count;
}

//low to high bitmap: xx?xx?xx?xx? ...
static tuz_try_inline tuz_length_t _cache_unpack_len(tuz_TStream* self){
    tuz_length_t    v=0;
    tuz_fast_uint8  lowbit;
    do {
        lowbit=_cache_read_lowbits(self,3);
        v=(v<<(3-1))+(lowbit&0x3);
    }while(lowbit&0x4);
    return v;
}

static tuz_force_inline tuz_size_t _cache_unpack_dict_pos(tuz_TStream* self){
    tuz_size_t result=_cache_read_1byte(&self->_code_cache);
    if (result<=0x7F)
        return result;
    else
        return (result&0x7F)|(_cache_unpack_len(self)<<7);
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


#if (tuz_isNeedSaveDictSize)
tuz_size_t tuz_TStream_read_dict_size(tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code){
    tuz_size_t v=tuz_kDictSizeSavedCount;
    tuz_byte   saved[tuz_kDictSizeSavedCount];
    assert(read_code!=0);
    if ((read_code(inputStream,saved,&v))&&(v==tuz_kDictSizeSavedCount)){
        #if (tuz_kDictSizeSavedCount==1)
            v=saved[0];
            assert(v>0);
        #elif (tuz_kDictSizeSavedCount==2)
            v=saved[0]|(((tuz_size_t)saved[1])<<8);
            assert((v>0)&(((v>>8)&0xFF)==saved[1]));
        #elif (tuz_kDictSizeSavedCount==3)
            v=saved[0]|(((tuz_size_t)saved[1])<<8)|(((tuz_size_t)saved[2])<<16);
            assert((v>0)&(((v>>8)&0xFF)==saved[1])&((v>>16)==saved[2]));
        #elif (tuz_kDictSizeSavedCount==4)
            v=saved[0]|(((tuz_size_t)saved[1])<<8)|(((tuz_size_t)saved[2])<<16)|(((tuz_size_t)saved[3])<<24);
            assert((v>0)&(((v>>8)&0xFF)==saved[1])&((v>>16)==saved[2])&((v>>24)==saved[3]));
        #else
        #   error unsupport tuz_kDictSizeSavedCount
        #endif
        return v;
    }else{ //error 
        return 0;
    }
}
#endif

tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* cache,tuz_size_t cache_size,tuz_size_t dict_size){
    assert((read_code!=0)&&(cache!=0));
    if (dict_size==0) return tuz_READ_DICT_SIZE_ERROR;
    if (cache_size<dict_size) return tuz_CACHE_SIZE_ERROR;
    cache_size-=dict_size;
    self->_code_cache.cache_begin=cache_size;
    self->_code_cache.cache_end=cache_size;
    self->_code_cache.cache_buf=cache+dict_size;
    self->_code_cache.inputStream=inputStream;
    self->_code_cache.read_code=read_code;
    self->_dict.dict_cur=0;
    self->_dict.dict_size=dict_size;
    self->_dict.dict_buf=cache;
    
    self->_state.dictType_pos=0;
    self->_state.dictType_pos_inc=0;
    self->_state.dictType_len=0;
    self->_state.literalType_len=0;
    self->_state.types=0;
    self->_state.type_count=0;
    return tuz_OK;
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
                    if (_cache_read_1bit(self)==0)
                        self->_state.dictType_len=_cache_read_1bit(self)+tuz_kMinDictMatchLen;
                    else
                        self->_state.dictType_len=_cache_unpack_len(self)+(tuz_kMinDictMatchLen+2);
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
                }else{
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


typedef struct _mem_TStream{
    const tuz_byte* in_code;
    const tuz_byte* in_code_end;
    tuz_fast_uint8  types;
    tuz_fast_uint8  type_count;
} _mem_TStream;

#ifdef __RUN_MEM_SAFE_CHECK
#   define __SafeTest(t) t 
#else
#   define __SafeTest(t) tuz_TRUE 
#endif

#define _mem_read_1byte(result) {   \
    if (__SafeTest(self.in_code!=self.in_code_end)){ \
        result=*self.in_code++;     \
    }else{                          \
        return tuz_READ_CODE_ERROR; \
    } }

static tuz_fast_uint8 _mem_read_lowbits(_mem_TStream* self,tuz_fast_uint8 bitCount){
    tuz_fast_uint8 count=self->type_count;
    tuz_fast_uint8 result=self->types;
    if (count>=bitCount){
        self->type_count=count-bitCount;
        self->types=(result>>bitCount);
        return result;
    }else{
        if (__SafeTest(self->in_code!=self->in_code_end)){
            tuz_fast_uint8 v=*self->in_code++;
            bitCount-=count;
            self->type_count=tuz_kMaxTypeBitCount-bitCount;
            self->types=v>>bitCount;
            return result|(v<<count);
        }else{
            return 0;
        }
    }
}

static tuz_length_t _mem_unpack_len(_mem_TStream* self){
    tuz_length_t    v=0;
    tuz_fast_uint8  lowbit;
    do {
        lowbit=_mem_read_lowbits(self,3);
        v=(v<<(3-1))+(lowbit&0x3);
    }while(lowbit&0x4);
    return v;
}

#define _mem_unpack_dict_pos(result) { \
    if (__SafeTest(self.in_code!=self.in_code_end)){ \
        result=(*self.in_code++);   \
        if (result>0x7F)            \
            result=(result&0x7F)|(_mem_unpack_len(&self)<<7); \
    }else{ \
        return tuz_READ_CODE_ERROR; \
    } }

tuz_TResult tuz_decompress_mem(const tuz_byte* _in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size){
    _mem_TStream self={_in_code,_in_code+code_size,0,0};
    tuz_byte*  cur_out_data=out_data;
    tuz_size_t dsize=*data_size;
#if (tuz_isNeedSaveDictSize)
    {//dict_size
        self.in_code+=tuz_kDictSizeSavedCount; //skip tuz_kDictSizeSavedCount size
        if (__SafeTest(self.in_code<=self.in_code_end)){
        }else{
            return tuz_READ_CODE_ERROR;
        }
    }
#endif
    while(1){
        if ((_mem_read_lowbits(&self,1)&1)==tuz_codeType_dict){
            tuz_size_t saved_dict_pos; 
            _mem_unpack_dict_pos(saved_dict_pos);
            if (saved_dict_pos){
                tuz_size_t dictType_len;
                tuz_fast_uint8 low2bit=_mem_read_lowbits(&self,2)&0x3;
                if ((low2bit&0x1)==0){
                    dictType_len=(low2bit>>1)+tuz_kMinDictMatchLen;
                }else{
                    self.types=(self.types<<1)+(low2bit>>1);
                    ++self.type_count;
                    dictType_len=_mem_unpack_len(&self)+(tuz_kMinDictMatchLen+2);
                }
#ifdef __RUN_MEM_SAFE_CHECK
                if (saved_dict_pos>(tuz_size_t)(cur_out_data-out_data)) return tuz_DICT_POS_ERROR;
                if (dictType_len>dsize) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                _memmove_order(cur_out_data,cur_out_data-saved_dict_pos,dictType_len);
                cur_out_data+=dictType_len;
                dsize-=dictType_len;
            }else {  
                tuz_length_t literalType_len=_mem_unpack_len(&self);
                if (literalType_len){ //literalType
                    literalType_len+=(tuz_kMinLiteralLen-1);
#ifdef __RUN_MEM_SAFE_CHECK
                    if (literalType_len>(tuz_size_t)(self.in_code_end-self.in_code)) return tuz_READ_CODE_ERROR;
                    if (literalType_len>dsize) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                    _memcpy(cur_out_data,self.in_code,literalType_len);
                    cur_out_data+=literalType_len;
                    self.in_code+=literalType_len;
                    dsize-=literalType_len;
                }else{ // ctrlType
                    tuz_fast_uint8 ctrlType;
                    _mem_read_1byte(ctrlType);
                    if (tuz_ctrlType_clipEnd==ctrlType){ //clip end
                        self.type_count=0;
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