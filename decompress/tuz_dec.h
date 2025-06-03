//  tuz_dec.h
/*
 The MIT License (MIT)
 Copyright (c) 2012-2025 HouSisong All Rights Reserved.
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
    tuz_READ_DICT_SIZE_ERROR,
    tuz_CACHE_SIZE_ERROR,
    tuz_DICT_POS_ERROR,
    tuz_OUT_SIZE_OR_CODE_ERROR,
    tuz_CODE_ERROR, //unknow code, or decode len(tuz_length_t) overflow

} tuz_TResult;


//-----------------------------------------------------------------------------------------------------------------
// decompress step by step: compiled by Mbed Studio is 626 bytes
//   if set tuz_isNeedLiteralLine=0 & _IS_RUN_MEM_SAFE_CHECK=0, compiled by Mbed Studio is 468 bytes

typedef struct tuz_TStream{
    _tuz_TInputCache    _code_cache;
    _tuz_TDict          _dict;
    _tuz_TState         _state;
} tuz_TStream;

//read dict_size from inputStream
tuz_size_t tuz_TStream_read_dict_size(tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code);

//open tuz_TStream
//  not need clear tuz_TStream before open;
//  dict_and_cache lifetime need holding by caller;
//  must cache_size>0, cache_size only affect decompress speed;
//  if success return tuz_OK
tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* dict_and_cache,tuz_size_t dict_size,tuz_size_t cache_size);

//decompress partial to out_data
//  data_size: input out_data buf's size, output decompressed data size when return tuz_STREAM_END;
//  if success return tuz_OK or tuz_STREAM_END, tuz_STREAM_END means decompress finish;
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);


//-----------------------------------------------------------------------------------------------------------------

#ifndef tuz_pbyte_r
typedef const tuz_byte* tuz_pbyte_r;  // read only 
#endif
#ifndef tuz_pbyte_rw
typedef tuz_byte*       tuz_pbyte_rw; // read & write
#endif

//decompress all to out_data memory
//  compiled by Mbed Studio is 424 bytes; faster than decompress by tuz_TStream; 
//    if set tuz_isNeedLiteralLine=0 & _IS_RUN_MEM_SAFE_CHECK=0, compiled by Mbed Studio is 298 bytes
//  data_size: input out_data buf's size, output decompressed data size;
//  tip: dict_size no limit when compress;
//  if success return tuz_STREAM_END;
tuz_TResult tuz_decompress_mem(tuz_pbyte_r in_code,tuz_size_t code_size,tuz_pbyte_rw out_data,tuz_size_t* data_size);


#ifdef __cplusplus
}
#endif
#endif //_tuz_dec_h
