# [tinyuz](https://github.com/sisong/tinyuz)
[![release](https://img.shields.io/badge/release-v0.9.3-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   

[![Build Status](https://github.com/sisong/tinyuz/workflows/ci/badge.svg?branch=master)](https://github.com/sisong/tinyuz/actions?query=workflow%3Aci+branch%3Amaster)   

 中文版 | [english](README.md)   

**tinyuz** 是一个无损压缩算法，为超小型设备(MCU、NB-IoT等)设计，保持还不错的压缩率。   
特色是编译后的解压缩代码(ROM占用)非常的小； 
流模式解压用 Mbed Studio 编译后为 856 字节(可以调整宏定义后到758字节)； 
而内存模式解压为 424 字节(可以调整宏定义后到非安全模式298字节)。   
（其他解压器用 Mbed Studio 编译后大小参考: zlib v1.2.12 流模式解压 约大于10k; lzma v22.01 流模式解压 约6k; minilzo v2.10 内存模式解压为 868 字节(非安全模式 628 字节)。）   
同时，流模式解压时内存(RAM占用)也可以非常的小，大小为 压缩时指定的字典大小(1Byte--1GB) + 解压缩时输入的缓存区大小(>=2Byte)； 
提示：字典越小压缩率越低，而输入缓存区较小时只影响解压缩速度。   
支持处理巨大的数据，压缩和解压缩时都支持流式处理。   
压缩和解压缩速度与数据特性和参数设置有关；在现代 CPU 上，压缩时比较慢约 0.4MB/S--2MB/S，解压缩较快约 180MB/S--300MB/S。   

---
## 二进制发布包
[从 release 下载](https://github.com/sisong/tinyuz/releases) : 分别运行在 Windows、Linux、MacOS操作系统的命令行程序。     
( 编译出这些发布文件的项目路径在 `tinyuz/builds` )   

## 自己编译
### Linux or MacOS X ###
```
$ cd <dir>
$ git clone https://github.com/sisong/tinyuz.git     tinyuz
$ git clone https://github.com/sisong/HDiffPatch.git HDiffPatch
$ cd tinyuz
$ make
```
### Windows ###
```
$ cd <dir>
$ git clone https://github.com/sisong/tinyuz.git     tinyuz
$ git clone https://github.com/sisong/HDiffPatch.git HDiffPatch
```
用 [`Visual Studio`](https://visualstudio.microsoft.com) 打开 `tinyuz/builds/vc/tinyuz.sln` 编译   

---
## 命令行使用:  
```
压缩  : tinyuz -c[-dictSize]  inputFile outputFile
解压缩: tinyuz -d[-cacheSize] inputFile outputFile
选项说明:
  -c[-dictSize]
      设置压缩时使用的字典大小;
      dictSize>=1, 默认 -c-16m, 推荐: 127、4k、1m、512m 等...
      运行时需要内存约 (dictSize*18) 字节;
  -d[-cacheSize]
      设置解压时使用的缓冲区大小;
      cacheSize>=2, 默认 -d-256k, 推荐: 64、1k、32k、4m 等...
      运行时需要内存 (dictSize+cacheSize) 字节;
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
也可以在内存中一次全部解压缩:
```
tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size);
```

---
## 压缩率测试:
压缩率: 压缩后大小/压缩前大小   
zlib v1.2.11: 测试时设置压缩水平为9, 窗口比特大小设置为-15   
tinyuz v0.9.2: 测试时设置多个不同的字典大小 32MB,1MB,32KB,5KB,1KB,255B,79B,24B   
  (表中'tuz -32m' 表示: tinyuz -c-32m)   
   
"aMCU.bin" 是一个MCU设备的固件文件;   
"aMCU.bin.diff" 是用两个不同版本的固件文件来创建的一个未压缩的补丁文件(用 [HPatchLite](https://github.com/sisong/HPatchLite) 所创建);   
"A10.jpg"--"world95.txt" 从 http://www.maximumcompression.com/data/files/index.html 下载   
"enwik8" 从 https://data.deepai.org/enwik8.zip 下载   
"silesia.tar" 从 https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia 下载
   
||zlib -9|tuz -32m|tuz -1m|tuz -32k|tuz -5k|tuz -1k|tuz -255|tuz -79|tuz -24|
|:----|----:|----:|----:|----:|----:|----:|----:|----:|----:|
|aMCU.bin|46.54%|45.80%|45.80%|45.98%|49.16%|54.29%|60.61%|68.03%|77.95%|
|aMCU.bin.diff|5.29%|5.75%|5.75%|5.75%|5.95%|6.35%|6.89%|7.85%|9.54%|
|A10.jpg|99.88%|99.99%|99.99%|99.99%|99.99%|99.99%|99.99%|99.99%|99.99%|
|AcroRd32.exe|44.88%|42.01%|42.12%|43.80%|46.53%|51.48%|58.29%|67.57%|78.81%|
|english.dic|25.83%|28.62%|28.65%|29.20%|29.98%|31.25%|33.49%|36.53%|39.93%|
|FlashMX.pdf|84.76%|86.08%|85.34%|85.81%|87.34%|88.31%|89.90%|92.05%|96.83%|
|FP.LOG|6.46%|4.95%|5.26%|7.36%|9.99%|12.67%|19.27%|99.25%|100.00%|
|MSO97.DLL|57.94%|53.54%|54.12%|56.96%|59.80%|64.38%|70.62%|78.36%|87.73%|
|ohs.doc|24.05%|20.65%|21.03%|24.50%|26.85%|31.08%|37.50%|69.31%|82.85%|
|rafale.bmp|30.23%|30.30%|30.40%|32.66%|35.51%|40.81%|43.52%|47.70%|54.42%|
|vcfiu.hlp|20.41%|17.71%|17.79%|20.39%|24.24%|27.46%|32.39%|49.01%|69.64%|
|world95.txt|28.87%|22.88%|23.44%|30.79%|47.15%|54.96%|65.23%|78.53%|97.20%|
|enwik8|36.45%|30.09%|33.22%|38.36%|43.96%|51.53%|63.38%|79.63%|96.78%|
|silesia.tar|31.98%|28.41%|29.66%|33.27%|38.21%|44.45%|52.58%|63.62%|78.49%|

---
## 联系
housisong@hotmail.com  

