# [tinyuz]
[![release](https://img.shields.io/badge/release-v1.0.0-blue.svg)](https://github.com/sisong/tinyuz/releases) 
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/sisong/tinyuz/blob/master/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-blue.svg)](https://github.com/sisong/tinyuz/pulls)
[![+issue Welcome](https://img.shields.io/github/issues-raw/sisong/tinyuz?color=green&label=%2Bissue%20welcome)](https://github.com/sisong/tinyuz/issues)   
[![Build Status](https://github.com/sisong/tinyuz/workflows/ci/badge.svg?branch=master)](https://github.com/sisong/tinyuz/actions?query=workflow%3Aci+branch%3Amaster)   
 english | [中文版](README_cn.md)   
   
**tinyuz** is a lossless compression algorithm, designed for embedded systems,MCU, NB-IoT, etc.,  with better compression ratios.   
Which is characterized by a very small decompress code(ROM or flash occupancy); 
The stream decompresser compiled by Mbed Studio is 626 bytes(can define to 468 bytes), 
and the memory decompresser is 424 bytes(can define to 298 bytes).    
At the same time, the stream decompress memory(RAM occupancy) can also be very small, 
RAM size = dictionary size(>=1Byte, specified when compress) + cache size(>=2Byte, specified when decompress). 
Tip: The smaller the dictionary, the lower the compression ratio; while the smaller cache only affects the decompress speed.   
( other decompresser compiled by Mbed Studio: [zlib] v1.3.1 stream decompresser >~10k; [lzma] v22.01 stream decompresser ~6k; [miniLZO] v2.10 memory decompresser 868 bytes(unsafe mode 628 bytes). )   
   
Large data are supported, both compress and decompress support streaming. 
The compress and decompress speed is related to the characteristics of the input data and parameter settings; 
On modern CPUs, compress speed is slower by about 0.4MB/S--2MB/S, and decompress speed is faster by about 180MB/S--300MB/S.   

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
## Porting decompress algorithm to embedded devices:
* Add or copy the entire directory `tinyuz/decompress/` to your project;
* Add a reference to the `tuz_dec.h` header file where the decompress algorithm is needed, and call the decompress functions declared in this file.

---
## test compression ratio:
ratio: compressedSize/uncompressedSize   
[tinyuz] v0.9.2: test with multiple different dictSize 1MB,32KB,4KB,1KB,255B 
('tuz -32k' means: tinyuz -c-32k)   
[zlib] v1.3.1 test with compress level 9, windowBits -15(i.e. dictSize 32KB)   
[QuickLZ] v1.5.0 test compress with default setting QLZ_COMPRESSION_LEVEL=3, QLZ_STREAMING_BUFFER=1048576   
[tamp] v1.7.0 test compress with windowBits 15 & 12(i.e. dictSize 32KB & 4KB)   
[heatshrink] v0.4.1 test compress with windowBits 12(i.e. dictSize 4KB), lookahead_sz2=6   
[FastLZ] v0.5.0 test with compress level 2   
[miniLZO] v2.10 test with `lzo1x_1_compress` function, wrkmem used default `LZO1X_1_MEM_COMPRESS=16k*sizeof(viod*)` size   
   
"aMCU.bin" is a firmware file of MCU device;   
"aMCU.bin.diff" is a uncompressed differential file between two versions of firmware files (created by [HPatchLite]);   
"A10.jpg"--"world95.txt" download from http://www.maximumcompression.com/data/files/index.html   
"enwik8" download from https://data.deepai.org/enwik8.zip   
"silesia.tar" download from https://sun.aei.polsl.pl//~sdeor/index.php?page=silesia
   
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
## Contact
housisong@hotmail.com  

