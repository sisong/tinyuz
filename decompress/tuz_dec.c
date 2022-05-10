//  tuz_dec.c
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_dec.h"
#include "tuz_types_private.h"
#if _IS_USED_C_MEMCPY
#   include <string.h> // memcpy
#endif

#if (_IS_RUN_MEM_SAFE_CHECK)
#   define __RUN_MEM_SAFE_CHECK
#endif

#define  _memmove_order    memmove_order
#if (!(_IS_USED_C_MEMCPY))
#   define  _memcpy        memmove_order
#else
#   define  _memcpy        memcpy
    tuz_force_inline
#endif
    static void memmove_order(tuz_byte* dst,const tuz_byte* src,tuz_size_t len){
        while (len--) *dst++=*src++;
    }

//low to high bitmap: xx?xx?xx?xx? ...
#define _def_unpack_len(self,readBit,_read_lowbits){ \
    tuz_length_t v=0;   \
    for(;;){            \
        tuz_fast_uint8 lowbit=_read_lowbits(self,readBit);  \
        v=(v<<(readBit-1))+(lowbit&((1<<(readBit-1))-1));   \
        if (!(lowbit&(1<<(readBit-1)))) return v;           \
        v+=1;           \
    } }

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

static tuz_force_inline tuz_length_t _cache_unpack_len(tuz_TStream* self){
    _def_unpack_len(self,2,_cache_read_lowbits);
}

#if tuz_isNeedLiteralLine
    tuz_try_inline
#else
    tuz_force_inline
#endif
static tuz_length_t _cache_unpack_pos_len(tuz_TStream* self){
    _def_unpack_len(self,3,_cache_read_lowbits);
}

static tuz_force_inline tuz_size_t _cache_unpack_dict_pos(tuz_TStream* self){
    tuz_size_t result=_cache_read_1byte(&self->_code_cache);
    if (result<(1<<7))
        return result;
    else
        return ((result&((1<<7)-1))|(_cache_unpack_pos_len(self)<<7))+(1<<7);
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


tuz_size_t tuz_TStream_read_dict_size(tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code){
    tuz_size_t v=tuz_kDictSizeSavedBytes;
    tuz_byte   saved[tuz_kDictSizeSavedBytes];
    assert(read_code!=0);
    if ((read_code(inputStream,saved,&v))&&(v==tuz_kDictSizeSavedBytes)){
        #if (tuz_kDictSizeSavedBytes==1)
            v=saved[0];
            assert(v>0);
        #elif (tuz_kDictSizeSavedBytes==2)
            v=saved[0]|(((tuz_size_t)saved[1])<<8);
            assert((v>0)&(((v>>8)&0xFF)==saved[1]));
        #elif (tuz_kDictSizeSavedBytes==3)
            v=saved[0]|(((tuz_size_t)saved[1])<<8)|(((tuz_size_t)saved[2])<<16);
            assert((v>0)&(((v>>8)&0xFF)==saved[1])&((v>>16)==saved[2]));
        #elif (tuz_kDictSizeSavedBytes==4)
            v=saved[0]|(((tuz_size_t)saved[1])<<8)|(((tuz_size_t)saved[2])<<16)|(((tuz_size_t)saved[3])<<24);
            assert((v>0)&(((v>>8)&0xFF)==saved[1])&((v>>16)==saved[2])&((v>>24)==saved[3]));
        #else
        #   error unsupport tuz_kDictSizeSavedBytes
        #endif
        return v;
    }else{ //error 
        return 0;
    }
}

tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* dict_and_cache,tuz_size_t dict_size,tuz_size_t cache_size){
    assert((read_code!=0)&&(dict_and_cache!=0));
    if (dict_size==0) return tuz_READ_DICT_SIZE_ERROR;
    if (cache_size==0) return tuz_CACHE_SIZE_ERROR;
    self->_code_cache.cache_begin=cache_size;
    self->_code_cache.cache_end=cache_size;
    self->_code_cache.cache_buf=dict_and_cache+dict_size;
    self->_code_cache.inputStream=inputStream;
    self->_code_cache.read_code=read_code;
    self->_dict.dict_cur=0;
    self->_dict.dict_size=dict_size;
    self->_dict.dict_buf=dict_and_cache;
    
    self->_state.dictType_pos=0;
    self->_state.dictType_pos_inc=0;
    self->_state.dict_pos_back=1;
    self->_state.dictType_len=0;    
  #if tuz_isNeedLiteralLine
    self->_state.literalType_len=0;
  #endif
    self->_state.types=0;
    self->_state.type_count=0;
    self->_state.isHaveData_back=tuz_FALSE;
    return tuz_OK;
}

tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size){
    tuz_byte*  cur_out_data=out_data;
    tuz_size_t dsize=*data_size;
#ifdef __RUN_MEM_SAFE_CHECK
    const tuz_BOOL isNeedOut=(dsize>0);
#endif
    for(;;){
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

  #if tuz_isNeedLiteralLine
    copyLiteral_cmp_process:
        if (self->_state.literalType_len){
        copyLiteral_process:
            if (dsize){
                tuz_length_t cpyLen=(self->_state.literalType_len<dsize)?self->_state.literalType_len:dsize;
                self->_state.literalType_len-=cpyLen;
                dsize-=cpyLen;
                while (cpyLen--)
                    *cur_out_data++=_cache_read_1byte(&self->_code_cache);
                goto copyLiteral_cmp_process;
            }else{
                break;
            }
        }
  #endif
  
    type_process:
        {
            if (_cache_read_1bit(self)==tuz_codeType_dict){
                tuz_size_t saved_len=_cache_unpack_len(self);
                tuz_size_t saved_dict_pos;
                if ((self->_state.isHaveData_back)&&(_cache_read_1bit(self))){
                    saved_dict_pos=self->_state.dict_pos_back;
                }else{
                    saved_dict_pos=_cache_unpack_dict_pos(self);
                    if (saved_dict_pos>tuz_kBigPosForLen) ++saved_len;
                }
                self->_state.isHaveData_back=tuz_FALSE;

                if (saved_dict_pos){
                    const tuz_size_t outed_size=(tuz_size_t)(cur_out_data-out_data);
                    self->_state.dict_pos_back=saved_dict_pos;
                    self->_state.dictType_len=saved_len+tuz_kMinDictMatchLen;
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
                }else{ // ctrlType
                  #if tuz_isNeedLiteralLine
                    if (tuz_ctrlType_literalLine==saved_len){
                        self->_state.isHaveData_back=tuz_TRUE;
                        self->_state.literalType_len=_cache_unpack_pos_len(self)+tuz_kMinLiteralLen;
                        goto copyLiteral_process;
                    }
                  #endif

                    self->_state.dict_pos_back=1;
                    self->_state.type_count=0;
                    if (tuz_ctrlType_clipEnd==saved_len){ //clip end
                        goto type_process;
                    }else if (tuz_ctrlType_streamEnd==saved_len){ //stream end
                        *data_size=(tuz_size_t)(cur_out_data-out_data);
                        return tuz_STREAM_END;
                    }else{
                        return _tuz_cache_success_finish(&self->_code_cache)?
                                    tuz_CTRLTYPE_UNKNOW_ERROR:tuz_READ_CODE_ERROR;
                    }
                }
            }else{
                if (dsize){                
                    self->_state.isHaveData_back=tuz_TRUE;
                    *cur_out_data++=_cache_read_1byte(&self->_code_cache);
                    --dsize;
                    goto type_process;
                }else{
                    _cache_push_1bit(self,tuz_codeType_data);
                    break;
                }
            }
        }
    }//end for

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


//---------------------------------------------------------------------------------------------

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
    if (__SafeTest(self.in_code<self.in_code_end)){ \
        result=*self.in_code++;     \
    }else{                          \
        return tuz_READ_CODE_ERROR; \
    } }

static tuz_try_inline tuz_fast_uint8 _mem_read_lowbits(_mem_TStream* self,tuz_fast_uint8 bitCount){
    tuz_fast_uint8 count=self->type_count;
    tuz_fast_uint8 result=self->types;
    if (count>=bitCount){
        self->type_count=count-bitCount;
        self->types=(result>>bitCount);
        return result;
    }else{
        if (__SafeTest(self->in_code<self->in_code_end)){
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

static tuz_force_inline tuz_length_t _mem_unpack_len(_mem_TStream* self){
    _def_unpack_len(self,2,_mem_read_lowbits);
}

#if tuz_isNeedLiteralLine
    tuz_try_inline
#else
    tuz_force_inline
#endif
static tuz_length_t _mem_unpack_pos_len(_mem_TStream* self){
    _def_unpack_len(self,3,_mem_read_lowbits);
}

#define _mem_unpack_dict_pos(result) {  \
    if (__SafeTest(self.in_code<self.in_code_end)){ \
        result=(*self.in_code++);       \
        if (result>=(1<<7))             \
            result=((result&((1<<7)-1))|(_mem_unpack_pos_len(&self)<<7))+(1<<7); \
    }else{ \
        return tuz_READ_CODE_ERROR;     \
    } }

tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size){
    _mem_TStream self={in_code+tuz_kDictSizeSavedBytes,in_code+code_size,0,0};
    tuz_byte*  cur_out_data=out_data;
    tuz_byte*  out_data_end=out_data+(*data_size);
    tuz_size_t dict_pos_back=1;
    tuz_BOOL   isHaveData_back=tuz_FALSE;
    for(;;){
        if ((_mem_read_lowbits(&self,1)&1)==tuz_codeType_dict){
            tuz_size_t saved_len=_mem_unpack_len(&self);
            tuz_size_t saved_dict_pos; 
            if ((isHaveData_back)&&(_mem_read_lowbits(&self,1)&1)){
                saved_dict_pos=dict_pos_back;
            }else{
                _mem_unpack_dict_pos(saved_dict_pos);
                if (saved_dict_pos>tuz_kBigPosForLen) ++saved_len;
            }
            isHaveData_back=tuz_FALSE;
            if (saved_dict_pos){
                tuz_size_t dictType_len=saved_len+tuz_kMinDictMatchLen;
                dict_pos_back=saved_dict_pos;
#ifdef __RUN_MEM_SAFE_CHECK
                if (saved_dict_pos>(tuz_size_t)(cur_out_data-out_data)) return tuz_DICT_POS_ERROR;
                if (dictType_len>(tuz_size_t)(out_data_end-cur_out_data)) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                {
                    const tuz_byte* src=cur_out_data-saved_dict_pos;
                    while (dictType_len--)
                        *cur_out_data++=*src++;
                }
            }else {// ctrlType
            #if tuz_isNeedLiteralLine
                if (tuz_ctrlType_literalLine==saved_len){
                    tuz_size_t literalType_len=_mem_unpack_pos_len(&self)+tuz_kMinLiteralLen;
                    const tuz_byte* src=self.in_code;
#ifdef __RUN_MEM_SAFE_CHECK
                    if (literalType_len>(tuz_size_t)(self.in_code_end-src)) return tuz_READ_CODE_ERROR;
                    if (literalType_len>(tuz_size_t)(out_data_end-cur_out_data)) return tuz_OUT_SIZE_OR_CODE_ERROR;
#endif
                    while (literalType_len--)//copy literal line
                        *cur_out_data++=*src++;
                    self.in_code=src;
                    isHaveData_back=tuz_TRUE;
                    continue;
                }
            #endif
                dict_pos_back=1;
                self.type_count=0;
                if (tuz_ctrlType_clipEnd==saved_len){ //clip end
                }else if (tuz_ctrlType_streamEnd==saved_len){ //stream end
                    *data_size=(tuz_size_t)(cur_out_data-out_data);
                    return tuz_STREAM_END;
                }else{
                    return tuz_CTRLTYPE_UNKNOW_ERROR;
                }
            }
        }else{
            if (__SafeTest(cur_out_data<out_data_end)){
                isHaveData_back=tuz_TRUE;
                _mem_read_1byte(*cur_out_data++); 
            }else{
                return tuz_OUT_SIZE_OR_CODE_ERROR;
            }
        }
    }//end for
}