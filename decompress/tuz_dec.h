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
    tuz_READ_CODE_ERROR,
    tuz_DICT_POS_ERROR,
    tuz_OUT_SIZE_OR_CODE_ERROR,
    tuz_CODE_ERROR, //unknow code ,or decode len(tuz_length_t) overflow
} tuz_TResult;
    
    typedef struct _tuz_TInputCache{
        tuz_dict_size_t cache_begin;
        tuz_dict_size_t cache_end;
        tuz_byte*       cache_buf;
        tuz_byte        input_state;
    } _tuz_TInputCache;
    typedef struct _tuz_TDict{
        tuz_dict_size_t dict_cur;
        tuz_dict_size_t dict_size;
        tuz_byte*       dict_buf;
    } _tuz_TDict;
    typedef struct _tuz_TState{
        tuz_dict_size_t dictType_pos;
        tuz_dict_size_t dictType_pos_inc;
        tuz_length_t    dictType_len;
        tuz_length_t    literalType_len;
        tuz_byte        types;
        tuz_byte        type_count;
        tuz_BOOL        is_ctrlType_stream_end;
    } _tuz_TState;

//data_size: input out_data buf's size,output readed data size,
//     if output size < input size means input stream end;
//if read error return tuz_FALSE;
typedef void*    tuz_TInputStreamHandle;
typedef tuz_BOOL (*tuz_TInputStream_read)(tuz_TInputStreamHandle inputStream,tuz_byte* out_data,tuz_dict_size_t* data_size);

typedef struct tuz_TStream{
    tuz_TInputStreamHandle  inputStream;
    tuz_TInputStream_read   read_code;
    
    _tuz_TInputCache    _code_cache;
    _tuz_TDict          _dict;
    _tuz_TState         _state;
} tuz_TStream;


//open tuz_TStream
//  kCodeCacheSize >=1; 64,250,1k,4k,32k,...  only affect decompress speed
//  read saved dictSize from inputStream to out_dictSize;
void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_dict_size_t kCodeCacheSize,tuz_dict_size_t* out_dictSize);

//set dict buf
//  dict_buf lifetime need holding by caller
static tuz_inline void tuz_TStream_decompress_begin(tuz_TStream* self,tuz_byte* dict_buf,tuz_dict_size_t dictSize){
    assert((self->_dict.dict_buf==0)&&(dict_buf!=0)&&(dictSize>0)&&(dictSize>=self->_dict.dict_size));
    self->_dict.dict_buf=dict_buf;
    self->_dict.dict_size=dictSize;
}

//decompress partial to out_data
//  data_size: input out_data buf's size,output decompressed data size;
//  if success return tuz_OK or tuz_STREAM_END, tuz_STREAM_END means decompress finish;
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t* data_size);

//not need clear tuz_TStream;

#ifdef __cplusplus
}
#endif
#endif //_tuz_dec_h
