# [tinyuz](https://github.com/sisong/tinyuz)
[![release](https://img.shields.io/badge/release-v0.4.0-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)

[english](README.md)   

**tinyuz** 是一个无损压缩算法，特色是编译后的解压缩代码(磁盘或Flash占用)非常的小，用 Mbed Studio 编译后为776字节；并且解压时内存(RAM占用)也可以非常的小，大小为 压缩时指定的字典大小(1Byte--16MB) + 解压缩输入缓存区大小(>=1Byte)；提示：字典越小压缩率越低，而输入缓存区较小时只影响解压缩速度。   
支持处理巨大的数据，压缩和解压缩时都是流式处理。   
压缩和解压缩速度和数据特性有关；在现代CPU上，压缩时比较慢约0.2MB/S--3MB/S，解压缩较快约160MB/S--250MB/S。   
(开发评估中...)

## 自己编译
编译时需要[HDiffPatch](https://github.com/sisong/HDiffPatch)库   

---
## 库 API 使用:
压缩:
```
hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props);
```
解压缩:
```
void tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                      tuz_byte* codeCache,tuz_dict_size_t kCodeCacheSize,tuz_dict_size_t* out_dictSize);
void tuz_TStream_decompress_begin(tuz_TStream* self,tuz_byte* dict_buf,tuz_dict_size_t dictSize);
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_dict_size_t* data_size);
```

---
## 联系
housisong@hotmail.com  

