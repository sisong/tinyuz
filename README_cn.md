# [tinyuz]
[![release](https://img.shields.io/badge/release-v1.0.0-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   
[![Build Status](https://github.com/sisong/tinyuz/workflows/ci/badge.svg?branch=master)](https://github.com/sisong/tinyuz/actions?query=workflow%3Aci+branch%3Amaster)   
 中文版 | [english](README.md)   

**tinyuz** 是一个无损压缩算法，为超小型嵌入式设备(MCU、NB-IoT等)设计，保持还不错的压缩率。   
特色是编译后的解压缩代码(ROM或flash占用)非常的小； 
流模式解压用 Mbed Studio 编译后为 626 字节(可以调整宏定义后可以到 468 字节)； 
而内存模式解压为 424 字节(可以调整宏定义后可以到 298 字节)。   
（其他解压器用 Mbed Studio 编译后大小参考: [zlib] v1.3.1 流模式解压 约大于10k; [lzma] v22.01 流模式解压 约6k; [miniLZO] v2.10 内存模式解压为 868 字节(非安全模式 628 字节)。）   
同时，流模式解压时内存(RAM占用)也可以非常的小，RAM大小= 字典大小(>=1Byte，压缩时指定) + 缓冲区大小(>=2Byte，解压缩时指定)。 
提示：字典越小压缩率越低，而缓冲区较小时只影响解压缩速度。   
支持处理巨大的数据，压缩和解压缩时都支持流式处理。   
压缩和解压缩速度与数据特性和参数设置有关；在现代 CPU 上，压缩时比较慢约 0.4MB/S--2MB/S，解压缩较快约 180MB/S--300MB/S。   

[tinyuz]: https://github.com/sisong/tinyuz
[HPatchLite]: https://github.com/sisong/HPatchLite
[zlib]: https://github.com/madler/zlib
[lzma]: https://www.7-zip.org/sdk.html
[QuickLZ]: http://www.quicklz.com/order.html
[tamp]: https://github.com/BrianPugh/tamp
[heatshrink]: https://github.com/atomicobject/heatshrink
[FastLZ]: https://github.com/ariya/fastlz
[miniLZO]: http://www.oberhumer.com/opensource/lzo

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
## 移植解压算法到嵌入式设备:
* 将 `tinyuz/decompress/` 整个目录添加或拷贝到你的项目工程中；
* 在需要使用解压缩算法的地方添加 `tuz_dec.h` 头文件的引用，并调用该文件中声明的解压缩函数。

---
## 压缩率测试:
压缩率: 压缩后大小/压缩前大小   
[tinyuz] v0.9.2 测试时字典大小分别设置为 1MB,32KB,4KB,1KB,255B 
(表中'tuz -32k' 表示: tinyuz -c-32k)   
[zlib] v1.3.1 测试时设置压缩水平为9, 窗口比特大小设置为-15(即字典大小32KB)   
[QuickLZ] v1.5.0 测试时使用默认设置QLZ_COMPRESSION_LEVEL=3, QLZ_STREAMING_BUFFER=1048576   
[tamp] v1.7.0 测试时窗口比特大小分别设置为15和12(即字典大小32KB和4KB)   
[heatshrink] v0.4.1 测试时窗口比特大小设置为12(即字典大小4KB), lookahead_sz2=6   
[FastLZ] v0.5.0 测试时设置压缩水平为2   
[miniLZO] v2.10 测试时`lzo1x_1_compress`函数, wrkmem使用了默认的`LZO1X_1_MEM_COMPRESS=16k*sizeof(viod*)`大小   
   
"aMCU.bin" 是一个MCU设备的固件文件;   
"aMCU.bin.diff" 是用两个不同版本的固件文件来创建的一个未压缩的补丁文件(用 [HPatchLite] 所创建);   
"A10.jpg"--"world95.txt" 从 http://www.maximumcompression.com/data/files/index.html 下载   
"enwik8" 从 https://data.deepai.org/enwik8.zip 下载   
"silesia.tar" 从 https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia 下载
   
|test file|tuz 1m|tuz 32k|tuz 4k|tuz 1k|tuz 255|zlib 32k|QuickLZ|tamp 32k|tamp 4k|heatshrink 4k|FastLZ|miniLZO
|:----|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|
aMCU.bin|45.80%|45.98%|49.64%|54.29%|60.61%|46.54%|54.49%|57.87%|56.11%|58.52%|62.21%|61.33%
aMCU.bin.diff|5.75%|5.75%|5.99%|6.35%|6.89%|5.29%|9.52%|18.91%|16.78%|9.26%|12.50%|14.21%
A10.jpg|99.99%|99.99%|99.99%|99.99%|99.99%|99.88%|100.00%|107.79%|108.27%|112.16%|102.91%|100.38%
AcroRd32.exe|42.12%|43.80%|46.99%|51.48%|58.29%|44.88%|52.07%|55.86%|54.17%|56.15%|61.22%|61.44%
english.dic|28.65%|29.20%|30.10%|31.25%|33.49%|25.83%|35.50%|39.81%|36.64%|36.82%|40.86%|43.82%
FlashMX.pdf|85.34%|85.81%|87.46%|88.31%|89.90%|84.76%|100.00%|92.92%|93.40%|96.60%|89.57%|91.73%
FP.LOG|5.26%|7.36%|10.34%|12.67%|19.27%|6.46%|8.59%|20.95%|21.51%|14.12%|11.97%|13.01%
MSO97.DLL|54.12%|56.96%|60.23%|64.38%|70.62%|57.94%|65.65%|67.75%|65.78%|70.49%|74.80%|75.57%
ohs.doc|21.03%|24.50%|27.14%|31.08%|37.50%|24.05%|25.72%|38.34%|38.15%|33.51%|28.31%|30.41%
rafale.bmp|30.40%|32.66%|35.80%|40.81%|43.52%|30.23%|42.06%|37.38%|39.72%|40.30%|52.63%|55.41%
vcfiu.hlp|17.79%|20.39%|24.51%|27.46%|32.39%|20.41%|24.88%|32.77%|33.87%|30.42%|32.36%|34.10%
world95.txt|23.44%|30.79%|48.07%|54.96%|65.23%|28.87%|35.17%|38.18%|51.30%|54.32%|52.04%|51.56%
enwik8|33.22%|38.36%|44.81%|51.53%|63.38%|36.45%|44.79%|44.48%|48.04%|50.41%|54.52%|55.79%
silesia.tar|29.66%|33.27%|38.99%|44.45%|52.58%|31.98%|38.60%|42.77%|45.09%|44.27%|47.25%|47.50%

---
## 联系
housisong@hotmail.com  

