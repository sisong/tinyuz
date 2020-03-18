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

#ifndef tuz_kCodeCacheSize
#   define tuz_kCodeCacheSize 1024
#endif

typedef enum tuz_TResult{
    tuz_OK=0,
    tuz_STREAM_END,
    
    tuz_OPEN_ERROR=10,
    tuz_ALLOC_MEM_ERROR,
    tuz_READ_CODE_ERROR,
    tuz_DICT_SIZE_ERROR,
    tuz_CODE_ERROR, //unknow code ,or len overflow tuz_length_t
} tuz_TResult;

    
    typedef struct _tuz_TInputCache{
        tuz_byte*       cache_buf;
        tuz_size_t      cache_begin;
        tuz_size_t      cache_end;
        tuz_BOOL        is_input_stream_end;
        tuz_BOOL        is_input_stream_error;
    } _tuz_TInputCache;
    typedef struct _tuz_TDict{
        tuz_byte*       dict_buf_end;
        tuz_size_t      dict_cur;
        tuz_size_t      dict_size;
    } _tuz_TDict;
    typedef struct _tuz_TState{
        tuz_byte        types;
        tuz_byte        type_count;
        tuz_byte        half_code;
    } _tuz_TState;

typedef struct tuz_TStream{
//listener parameters
    void*       listener;
    //code_size: input out_code buf size,output readed code size,
    //     if output size < input size means input stream end;
    //if read error return tuz_FALSE;
    tuz_BOOL    (*read_code) (void* listener,tuz_byte* out_code,tuz_size_t* code_size);
    void*       (*alloc_mem) (void* listener,tuz_size_t mem_size);//mem_size==tuz_kCodeCacheSize+dictSize
    void        (*free_mem)  (void* listener,void* pmem);
//private:
    _tuz_TInputCache    _code_cache;
    _tuz_TDict          _dict;
    _tuz_TState         _state;
} tuz_TStream;

tuz_inline static void  tuz_TStream_init(tuz_TStream* self)  { memset(self,0,sizeof(*self)); }

//open tuz_TStream
//  must set listener parameters befor open;
//  read some code & alloc mem;
//  if success return tuz_OK;
tuz_TResult             tuz_TStream_open(tuz_TStream* self);
    
//decompress part data
//  data_size: input out_data buf size,output decompressed data size;
//  if success return tuz_OK or tuz_STREAM_END;
//    return tuz_STREAM_END means decompress finish;
tuz_TResult             tuz_TStream_decompress(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);

//close tuz_TStream
//  free mem;
tuz_inline static void  tuz_TStream_close(tuz_TStream* self) {
                                if (self&&(self->_code_cache.cache_buf)){
                                    tuz_byte* mem=self->_code_cache.cache_buf;
                                    self->_code_cache.cache_buf=0;
                                    self->free_mem(self->listener,mem); } }

#ifdef __cplusplus
}
#endif
#endif //_tuz_dec_h
