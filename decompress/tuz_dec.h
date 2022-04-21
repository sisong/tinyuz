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
    
    tuz_CTRLTYPE_UNKNOW_ERROR=10,
    tuz_CTRLTYPE_STREAM_END_ERROR,
    
    tuz_READ_CODE_ERROR=20,
    tuz_DICT_POS_ERROR,
    tuz_OUT_SIZE_OR_CODE_ERROR,
    tuz_CODE_ERROR, //unknow code, or decode len(tuz_length_t) overflow

} tuz_TResult;

//-----------------------------------------------------------------------------------------------------------------
// decompress by tuz_TStream: compiled by Mbed Studio is 750 bytes

typedef struct tuz_TStream{
    _tuz_TInputCache    _code_cache;
    _tuz_TDict          _dict;
    _tuz_TState         _state;
} tuz_TStream;

//open tuz_TStream
//  kCodeCacheSize >=1; 64,250,1k,4k,32k,...  only affect decompress speed;
//  read saved dictSize from inputStream to out_dictSize, out_dictSize can null;
//  codeCache lifetime need holding by caller;
//  not need clear tuz_TStream before open;
void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_size_t kCodeCacheSize,tuz_size_t* out_dictSize);

//set dict buf
//  dict_buf lifetime need holding by caller;
static tuz_force_inline 
void tuz_TStream_decompress_begin(tuz_TStream* self,tuz_byte* dict_buf,tuz_size_t dictSize){
    assert((self->_dict.dict_buf==0)&&(dict_buf!=0)&&(dictSize>0)&&(dictSize>=self->_dict.dict_size));
    self->_dict.dict_buf=dict_buf;
}

//decompress partial to out_data
//  data_size: input out_data buf's size,output decompressed data size;
//  if success return tuz_OK or tuz_STREAM_END, tuz_STREAM_END means decompress finish;
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);


//-----------------------------------------------------------------------------------------------------------------

//decompress all to out_data
//  compiled by Mbed Studio is 464 bytes; faster than decompress by tuz_TStream; 
//  if success return tuz_STREAM_END;
tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size);


#ifdef __cplusplus
}
#endif
#endif //_tuz_dec_h
