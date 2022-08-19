# [tinyuz](https://github.com/sisong/tinyuz)
[![release](https://img.shields.io/badge/release-v0.9.3-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   

[![Build Status](https://github.com/sisong/tinyuz/workflows/ci/badge.svg?branch=master)](https://github.com/sisong/tinyuz/actions?query=workflow%3Aci+branch%3Amaster)   

 english | [中文版](README_cn.md)   
   
**tinyuz** is a lossless compression algorithm, designed for tiny devices (MCU, NB-IoT, etc.) with better compression ratios.   
Which is characterized by a very small decompress code(ROM occupancy); 
The stream decompresser compiled by Mbed Studio is 856 bytes(can define to 758bytes), 
and the memory decompresser compiled by Mbed Studio is 424 bytes(can define to 298bytes).    
At the same time, the decompress memory(RAM occupancy) can also be very small, 
RAM size = dictionary size(1Byte--1GB) specified when compress + input cache size(>=2Byte) when decompress. 
Tip: The smaller the dictionary, the lower the compression ratio; while the smaller input cache only affects the decompress speed.   
   
Large data are supported, both compress and decompress support streaming. 
The compress and decompress speed is related to the characteristics of the input data and parameter settings; 
On modern CPUs, compress speed is slower by about 0.4MB/S--2MB/S, and decompress speed is faster by about 180MB/S--300MB/S.   

---
## Releases/Binaries
[Download from latest release](https://github.com/sisong/tinyuz/releases) : Command line app for Windows, Linux, MacOS.     
( release files build by projects in path `tinyuz/builds` )   

## Build it yourself
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
build `tinyuz/builds/vc/tinyuz.sln` with [`Visual Studio`](https://visualstudio.microsoft.com)   

---
## command line usage:  
```
compress   : tinyuz -c[-dictSize]  inputFile outputFile
deccompress: tinyuz -d[-cacheSize] inputFile outputFile
options:
  -c[-dictSize]
      set compress dictSize;
      dictSize>=1, DEFAULT -c-16m, recommended: 127, 4k, 1m, 512m, etc...
      requires O(dictSize*18) bytes of memory;
  -d[-cacheSize]
      set decompress cacheSize;
      cacheSize>=2, DEFAULT -d-256k, recommended: 64, 1k, 32k, 4m, etc...
      requires (dictSize+cacheSize) bytes of memory;
```

---
## library API usage:
compress:
```
hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props);
```
decompress:
```
tuz_size_t tuz_TStream_read_dict_size(tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code);
tuz_TResult tuz_TStream_open(tuz_TStream* self,tuz_TInputStreamHandle inputStream,tuz_TInputStream_read read_code,
                             tuz_byte* dict_and_cache,tuz_size_t dict_size,tuz_size_t cache_size);
tuz_TResult tuz_TStream_decompress_partial(tuz_TStream* self,tuz_byte* out_data,tuz_size_t* data_size);
```
can also decompress at once in memory:
```
tuz_TResult tuz_decompress_mem(const tuz_byte* in_code,tuz_size_t code_size,tuz_byte* out_data,tuz_size_t* data_size);
```

---
## test compression ratio:
ratio: compressedSize/uncompressedSize   
zlib v1.2.11: test with compress level 9, windowBits -15   
tinyuz v0.9.2: test with multiple different dictSizes 32MB,1MB,32KB,5KB,1KB,255B,79B,24B   
  ('tuz -32m' means: tinyuz -c-32m)   
   
"aMCU.bin" is a firmware file of MCU device;   
"aMCU.bin.diff" is a uncompressed differential file between two versions of firmware files (created by [HPatchLite](https://github.com/sisong/HPatchLite));   
"A10.jpg"--"world95.txt" download from http://www.maximumcompression.com/data/files/index.html   
"enwik8" download from https://data.deepai.org/enwik8.zip   
"silesia.tar" download from https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia
   
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
## Contact
housisong@hotmail.com  

