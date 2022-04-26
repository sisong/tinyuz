//speed_test.cpp
// for tinyuz
 /*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
 */
#include <iostream>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include "../HDiffPatch/_clock_for_demo.h" //in HDiffPatch
#include "decompress/tuz_dec.h"
#include "compress/tuz_enc.h"
#include "zlib.h"

#ifdef min
#   undef min
#endif

std::string TEST_FILE_DIR;

void readFile(std::vector<unsigned char>& data,const char* fileName){
    FILE	* file=fopen(fileName, "rb");
    assert(file);
	fseek(file,0,SEEK_END);
	int file_length = (int)ftell(file);
	fseek(file,0,SEEK_SET);
    
    data.resize(file_length);
    if (file_length>0)
        fread(&data[0],1,file_length,file);
    
    fclose(file);
}

void writeFile(const std::vector<unsigned char>& data,const char* fileName){
    FILE* file=fopen(fileName, "wb");
    
    int dataSize=(int)data.size();
    if (dataSize>0)
        fwrite(&data[0], 1,dataSize, file);
    
    fclose(file);
}


typedef int (*T_compress)(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* src,const unsigned char* src_end);
typedef bool (*T_decompress)(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end);

struct TTestResult {
    std::string     procName;
    std::string     srcFileName;
    double          compressTime_s;
    double          decompressTime_s;
    int             srcSize;
    int             zipSize;
};

double minEncTestTime=0.5;
double minDecTestTime=0.5;
double testDecodeProc(T_decompress proc_decompress,unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    int testDecompressCount=0;
    double time1=clock_s();
    do {
        for (int i=0; i<10; ++i){
            bool ret=proc_decompress(out_data,out_data_end,zip_code,zip_code_end);
            ++testDecompressCount;
            if (!ret) throw "error result!";
        }
    }while ((clock_s()-time1)<minDecTestTime);
    double time2=clock_s();
    double decompressTime_s=(time2-time1)/testDecompressCount;
    return  decompressTime_s;
}


double testEncodeProc(T_compress proc_compress,std::vector<unsigned char>& compressedCode,const unsigned char* src,const unsigned char* src_end){
    int testCompressCount=0;
    compressedCode.resize((size_t)((src_end-src)*1.2)+1024);
    int dstCodeSize=0;
    double time1=clock_s();
    do{
        dstCodeSize=proc_compress(&compressedCode[0],&compressedCode[0]+compressedCode.size(),src,src_end);
        ++testCompressCount;
    }while ((clock_s()-time1)<minEncTestTime);
    double time2=clock_s();
    compressedCode.resize(dstCodeSize);
    double compressTime_s=(time2-time1)/testCompressCount;
    return compressTime_s;
}

TTestResult testProc(const char* srcFileName,T_compress proc_compress,const char* proc_compress_Name,
                 T_decompress proc_decompress,const char* proc_decompress_Name){
    std::string testFilePath=TEST_FILE_DIR; testFilePath.append(srcFileName);
    std::vector<unsigned char> oldData; readFile(oldData,testFilePath.c_str());
    const unsigned char* src=&oldData[0];
    const unsigned char* src_end=src+oldData.size();
    
    std::vector<unsigned char> compressedCode;
    double compressTime_s=testEncodeProc(proc_compress,compressedCode,src,src_end);
    const unsigned char* unsrc=&compressedCode[0];
    
    std::vector<unsigned char> uncompressedCode(oldData.size(),0);
    unsigned char* undst=&uncompressedCode[0];
    
    double decompressTime_s=testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size());
    decompressTime_s=std::min(decompressTime_s,testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size()));
    decompressTime_s=std::min(decompressTime_s,testDecodeProc(proc_decompress,undst,undst+uncompressedCode.size(),unsrc,unsrc+compressedCode.size()));

    if (uncompressedCode!=oldData){
        throw "error data!";
    }
    
    TTestResult result;
    result.procName=proc_decompress_Name;
    result.srcFileName=srcFileName;
    result.compressTime_s=compressTime_s;
    result.decompressTime_s=decompressTime_s;
    result.srcSize=(int)(src_end-src);
    result.zipSize=(int)compressedCode.size();
    return result;
}


static void outResult(const TTestResult& rt){
    std::cout<<"\""<<rt.srcFileName.c_str()<< "\"\t";
    std::cout<<rt.srcSize/1024.0/1024<<"M\t";
    std::cout<<rt.procName.c_str()<< "\t";
    std::cout<<rt.zipSize*100.0/rt.srcSize<<"% ("<<rt.zipSize<<") \t";
    //std::cout<<rt.compressTime_s<<"S\t";
    std::cout<<rt.srcSize/rt.compressTime_s/1024/1024<<"M/S\t";
    //std::cout<<rt.decompressTime_s<<"S\t";
    std::cout<<rt.srcSize/rt.decompressTime_s/1024/1024<<"M/S\n";
}


////
int zlib_windowBits=0;

int zlib_compress(unsigned char* out_data,unsigned char* out_data_end,
                 const unsigned char* src,const unsigned char* src_end){
    const unsigned char* _zipSrc=&src[0];
    unsigned char* _zipDst=&out_data[0];
 
    z_stream c_stream;
    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;
    c_stream.next_in = (Bytef*)_zipSrc;
    c_stream.avail_in = (int)(src_end-src);
    c_stream.next_out = (Bytef*)_zipDst;
    c_stream.avail_out = (unsigned int)(out_data_end-out_data);
    int ret = deflateInit2(&c_stream,9,Z_DEFLATED,zlib_windowBits,MAX_MEM_LEVEL,Z_DEFAULT_STRATEGY);
    if(ret!=Z_OK)
        throw "deflateInit2 error !";
    ret = deflate(&c_stream,Z_FINISH);
    if (ret!=Z_STREAM_END)
        throw "deflate error !";
    int zipLen = (int)c_stream.total_out;
    ret = deflateEnd(&c_stream);
    if (ret!=Z_OK)
        throw "deflateEnd error !";
    return zipLen;
}

const tuz_size_t kCodeCacheSize=1024*1/4;

bool zlib_decompress(unsigned char* out_data,unsigned char* out_data_end,
                    const unsigned char* zip_code,const unsigned char* zip_code_end){
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[kCodeCacheSize];
    int totalsize = 0;
    
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    
    ret = inflateInit2(&strm,zlib_windowBits);
    
    if (ret != Z_OK)
        return false;

    strm.avail_in = (int)(zip_code_end-zip_code);
    strm.next_in = (unsigned char*)zip_code;
    
    do {
        strm.avail_out = kCodeCacheSize;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);
        switch (ret)
        {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                return false;
        }
        
        have = kCodeCacheSize - strm.avail_out;
        memcpy(out_data + totalsize,out,have);
        totalsize += have;
        assert(out_data+totalsize<=out_data_end);
    } while (strm.avail_out == 0);
    
    inflateEnd(&strm);
    return ret == Z_STREAM_END;
}



tuz_size_t _tuz_kDictSize=0;

int _test_tuz_compress(unsigned char* out_data,unsigned char* out_data_end,
                       const unsigned char* src,const unsigned char* src_end){
    hpatch_TStreamOutput out_stream;
    mem_as_hStreamOutput(&out_stream,out_data,out_data_end);
    hpatch_TStreamInput in_stream;
    mem_as_hStreamInput(&in_stream,src,src_end);
    tuz_TCompressProps props=tuz_kDefaultCompressProps;
    props.dictSize=_tuz_kDictSize;
    //props.maxSaveLength=255;
    hpatch_StreamPos_t codeSize=tuz_compress(&out_stream,&in_stream,&props);
    return (int)codeSize;
}

struct TTuzListener{
    const unsigned char* src;
    const unsigned char* src_end;
    static tuz_BOOL read_code(void* listener,tuz_byte* out_code,tuz_size_t* code_size){
        TTuzListener* self=(TTuzListener*)listener;
        tuz_size_t r_size=*code_size;
        size_t s_size=self->src_end-self->src;
        if (r_size>s_size){
            r_size=(tuz_size_t)s_size;
            *code_size=r_size;
        }
        memcpy(out_code,self->src,r_size);
        self->src+=r_size;
        return tuz_TRUE;
    }
};

const bool is_decode_step=true;
tuz_TResult tuz_decompress_stream(const tuz_byte* code,const tuz_byte* code_end,
                                  tuz_byte* out_uncompress,size_t* uncompress_size){
    TTuzListener  listener={code,code_end};
    tuz_byte* _dict_buf=0;
    tuz_TStream tuz;
    tuz_TResult result=tuz_OK;
#if (tuz_isNeedSaveDictSize)
    tuz_size_t dictSize=tuz_TStream_read_dict_size(&listener,listener.read_code);
#else
    tuz_size_t dictSize=_tuz_kDictSize;//unknow dictSize
#endif
    _dict_buf=(tuz_byte*)malloc(dictSize+kCodeCacheSize);
    assert(_dict_buf!=0);
    result=tuz_TStream_open(&tuz,&listener,listener.read_code,_dict_buf,dictSize+kCodeCacheSize,dictSize);
    if (is_decode_step){
        const size_t buf_size=*uncompress_size;
        size_t data_size=0;
        while (result==tuz_OK) {
            tuz_size_t step_size=kCodeCacheSize; //for test
            if (data_size+step_size>buf_size)
                step_size=(tuz_size_t)(buf_size-data_size);
            result=tuz_TStream_decompress_partial(&tuz,out_uncompress,&step_size);
            data_size+=step_size;
            out_uncompress+=step_size;
        }
        *uncompress_size=data_size;
    }else if (result==tuz_OK){
        tuz_size_t usize=(tuz_size_t)(*uncompress_size);
        assert(usize==*uncompress_size);
        result=tuz_TStream_decompress_partial(&tuz,out_uncompress,&usize);
        *uncompress_size=usize;
    }
    free(_dict_buf);
    return result;
}

bool _test_tuz_decompress_stream(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    size_t uncompress_size=out_data_end-out_data;
    tuz_TResult ret=tuz_decompress_stream(zip_code,zip_code_end,out_data,&uncompress_size);
    return (ret==tuz_STREAM_END)&&(uncompress_size==(out_data_end-out_data));
}

bool _test_tuz_decompress_mem(unsigned char* out_data,unsigned char* out_data_end,const unsigned char* zip_code,const unsigned char* zip_code_end){
    tuz_size_t uncompress_size=out_data_end-out_data;
    tuz_TResult ret=tuz_decompress_mem(zip_code,zip_code_end-zip_code,out_data,&uncompress_size);
    return (ret==tuz_STREAM_END)&&(uncompress_size==(out_data_end-out_data));
}

static void testFile(const char* srcFileName){
    outResult(testProc(srcFileName,zlib_compress     ,"",zlib_decompress            ,"       zlib_9"));
    outResult(testProc(srcFileName,_test_tuz_compress,"",_test_tuz_decompress_stream,"tinyuz_stream"));
    outResult(testProc(srcFileName,_test_tuz_compress,"",_test_tuz_decompress_mem   ,"   tinyuz_mem"));
    //std::cout << "\n";
}

int main(int argc, const char * argv[]){
    const int testDictBit=12;
    zlib_windowBits=-testDictBit;
    _tuz_kDictSize=(1<<testDictBit);
    if (argc!=2){
        std::cout << "speed_test testFile\n";
        return -1;
    }
    std::cout << "  ( dictSize: " << _tuz_kDictSize
              << "   codeCacheSize: " << kCodeCacheSize << " )\n";

    testFile(argv[1]);

    /* //for test
    std::cout << "test start> \n";
    minEncTestTime=0.2;
    minDecTestTime=0.3;

    testFile("V0.pat"); testFile("V1.pat"); testFile("V2.pat"); testFile("V3.pat");
    testFile("V0.bin"); testFile("V1.bin"); testFile("V2.bin"); testFile("V3.bin"); testFile("V4.bin");

    //*
    testFile("world95.txt");
    testFile("ohs.doc");
    testFile("FP.LOG");
    testFile("vcfiu.hlp");
    testFile("AcroRd32.exe");
    testFile("MSO97.DLL");
    testFile("english.dic");
    testFile("FlashMX.pdf");
    testFile("rafale.bmp");
    testFile("A10.jpg");
    testFile("enwik8");
    testFile("silesia.tar");
    //*/

    std::cout << "done!\n";
    return 0;
}

