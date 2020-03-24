//  tuz_dec.h
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
#ifndef _tuz_dec_h
#define _tuz_dec_h
#include "tuz_types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum tuz_TResult{
    tuz_OK=0,
    tuz_STREAM_END,
    
    tuz_CTRLTYPE_UNKNOW_ERROR=5,
    tuz_CTRLTYPE_STREAM_END_ERROR,
    
    tuz_OPEN_ERROR=10,
    tuz_ALLOC_MEM_ERROR,
    tuz_READ_CODE_ERROR,
    tuz_DICT_POS_ERROR,
    tuz_OUT_SIZE_OR_CODE_ERROR,
    tuz_CODE_ERROR, //unknow code ,or decode len(tuz_length_t) overflow
} tuz_TResult;
    
    typedef struct _tuz_TInputCache{
        tuz_byte*       cache_buf;
        tuz_dict_size_t cache_begin;
        tuz_dict_size_t cache_end;
        tuz_BOOL        is_input_stream_end;
        tuz_BOOL        is_input_stream_error;
    } _tuz_TInputCache;
    typedef struct _tuz_TDict{
        tuz_byte*       dict_buf;
        tuz_dict_size_t dict_cur;
        tuz_dict_size_t dict_size;
    } _tuz_TDict;
    typedef struct _tuz_TState{
        tuz_length_t    codeType_len;
        tuz_length_t    dictType_len;
        tuz_dict_size_t dictType_pos;
        tuz_dict_size_t dictType_pos_inc;
        tuz_byte        types;
        tuz_byte        type_count;
        tuz_byte        half_code;
        tuz_BOOL        is_ctrlType_stream_end;
    } _tuz_TState;

//data_size: input out_data buf's size,output readed data size,
//     if output size < input size means input stream end;
//if read error return tuz_FALSE;
typedef tuz_BOOL (*tuz_TInputstream)(void* listener,tuz_byte* out_data,tuz_dict_size_t* data_size);
    
typedef void*    (*tuz_TAllocMem)   (void* listener,tuz_dict_size_t mem_size);
typedef void     (*tuz_TFreeMem)    (void* listener,void* pmem);

typedef struct tuz_TStream{
//listener parameters
    void*               listener;
    //code_size: input out_code buf size,output readed code size,
    //     if output size < input size means input stream end;
    //if read error return tuz_FALSE;
    tuz_TInputstream    read_code;
    tuz_TAllocMem       alloc_mem; //alloc mem size==dict size
    tuz_TFreeMem        free_mem;
    
    _tuz_TInputCache    _code_cache;
    _tuz_TDict          _dict;
    _tuz_TState         _state;
    tuz_byte            kMinDictMatchLen;
} tuz_TStream;

void  tuz_TStream_init(tuz_TStream* self,void* listener,tuz_TInputstream read_code,
                       tuz_TAllocMem alloc_mem,tuz_TFreeMem free_mem);

//open tuz_TStream
    //  read some code & alloc mem for dict;
//  kDecodeCacheSize >=1; 64,250,1k,8k,32k, ...
//  if success return tuz_OK;
tuz_TResult             tuz_TStream_open(tuz_TStream* self,tuz_byte* decodeCache,tuz_dict_size_t kDecodeCacheSize);

//decompress part data
//  data_size: input out_data buf size,output decompressed data size;
//  if success return tuz_OK or tuz_STREAM_END;
//    return tuz_STREAM_END means decompress finish;
tuz_TResult             tuz_TStream_decompress(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t* data_size);

//close tuz_TStream
//  free mem;
tuz_inline static void  tuz_TStream_close(tuz_TStream* self) {
                                if (self&&(self->_dict.dict_buf)){
                                    tuz_byte* mem=self->_dict.dict_buf;
                                    self->_dict.dict_buf=0;
                                    self->free_mem(self->listener,mem); } }

#ifdef __cplusplus
}
#endif
#endif //_tuz_dec_h
