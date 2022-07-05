# [tinyuz](https://github.com/sisong/tinyuz)
[![release](https://img.shields.io/badge/release-v0.9.2-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   

[![Build Status](https://github.com/sisong/tinyuz/workflows/ci/badge.svg?branch=master)](https://github.com/sisong/tinyuz/actions?query=workflow%3Aci+branch%3Amaster)   

 中文版 | [english](README.md)   

**tinyuz** 是一个无损压缩算法，特色是编译后的解压缩代码(磁盘或Flash占用)非常的小，流模式解压用 Mbed Studio 编译后为 856 字节；
并且解压时内存(RAM 占用)也可以非常的小，大小为 压缩时指定的字典大小(1Byte--1GB) + 解压缩时输入的缓存区大小(>=2Byte)；提示：字典越小压缩率越低，而输入缓存区较小时只影响解压缩速度。   
支持处理巨大的数据，压缩和解压缩时都是流式处理。   
压缩和解压缩速度与数据特性和参数设置有关；在现代 CPU 上，压缩时比较慢约 0.4MB/S--2MB/S，约占用 字典大小*18 的内存；解压缩较快约 180MB/S--300MB/S。   
(开发评估中...)

---
## 二进制发布包
[从 release 下载](https://github.com/sisong/tinyuz/releases) : 分别运行在 Windows、Linux、MacOS操作系统的命令行程序.     
( 编译出这些发布文件的项目路径在 `tinyuz/builds` )   

## 自己编译
编译时需要依赖[HDiffPatch](https://github.com/sisong/HDiffPatch)库   
```
$ cd <dir>
$ git clone https://github.com/sisong/tinyuz.git     tinyuz
$ git clone https://github.com/sisong/HDiffPatch.git HDiffPatch
$ cd tinyuz
$ make
```

---
## 命令行使用:  
```
压缩  : tinyuz -c[-字典大小[k|m]]  inputFile outputFile
解压缩: tinyuz -d[-缓冲区大小[k|m]] inputFile outputFile
```

---
## 库 API 使用:
压缩:
```
hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props);
```
解压缩:
```
tuz_size_t tuz_TStream_read_dict_size(tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code);
tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* dict_and_cache,tuz_size_t dict_size,tuz_size_t cache_size);
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);
```
也可以在内存中一次全部解压缩(用 Mbed Studio 编译后为 424 字节):
```
tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size);
```

---
## 联系
housisong@hotmail.com  

