# [tinyuz](https://github.com/sisong/tinyuz)
[![release](https://img.shields.io/badge/release-v0.5.0-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   

[中文版](README_cn.md)   
   
**tinyuz** is a lossless compression algorithm, which is characterized by a very small decompress code(disk or Flash occupancy, compiled from source code); The code compiled by Mbed Studio is 738 bytes. And the decompress memory(RAM occupancy) can also be very small, RAM size = dictionary size(1Byte--16MB) specified when compress + input cache size(>=1Byte) when decompress. Tip: The smaller the dictionary, the lower the compression ratio; while the smaller input cache only affects the decompress speed.   
Large data are supported, and both compress and decompress are streaming.   
The compressi and decompress speed is related to the characteristics of the input data; On modern CPUs, compress speed is slower by about 0.2MB/S--3MB/S, and decompress speed is faster by about 160MB/S--250MB/S.   
(developmenting & evaluating ...)

## Build it yourself
need library [HDiffPatch](https://github.com/sisong/HDiffPatch)

---
## library API usage:
compress:
```
hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props);
```
decompress:
```
void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_size_t kCodeCacheSize,tuz_size_t* out_dictSize);
void tuz_TStream_decompress_begin(tuz_TStream* self,tuz_byte* dict_buf,tuz_size_t dictSize);
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);
```
can also decompress at once in memory:
```
tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size);
```

---
## Contact
housisong@hotmail.com  

