//unit_test.cpp
// for tinyuz
 /*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
 */

#include <iostream>
#include <string.h>
#include <math.h>
#include <vector>
#include "../HDiffPatch/_clock_for_demo.h" //in HDiffPatch
#include "decompress/tuz_dec.h"
#include "compress/tuz_enc.h"

const int   kRandTestCount=2000;
const bool  is_attack_decompress=false;
const bool  is_log_tag=true;
const bool  is_all_rand=false;
const bool  is_decode_step=true;
int     error_count=0;
double  sum_src_size=0;
double  sum_cmz_size=0;


const tuz_size_t kCodeCacheSize=1024*4;
const tuz_size_t kDictSize=1024*64-1;
const tuz_size_t kMaxSaveLength=1024*64-1;

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

tuz_TResult tuz_decompress_stream(const tuz_byte* code,const tuz_byte* code_end,
                                  tuz_byte* out_uncompress,size_t* uncompress_size){
    TTuzListener  listener={code,code_end};
    tuz_byte* _dict_buf=0;
    tuz_TStream tuz;
    tuz_TResult result=tuz_OK;
    tuz_size_t dictSize=tuz_TStream_read_dict_size(&listener,listener.read_code);
    _dict_buf=(tuz_byte*)malloc(dictSize+kCodeCacheSize);
    assert(_dict_buf!=0);
    result=tuz_TStream_open(&tuz,&listener,listener.read_code,_dict_buf,dictSize,kCodeCacheSize);
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

int _attack_seed=11111111;
long attack_decompress(const tuz_byte* _code,const tuz_byte* _code_end,tuz_size_t uncompress_size,
                       const char* error_tag){
    char tag[250]="\0";
    srand(_attack_seed);
    _attack_seed+=1;
    const long kLoopCount=1000;
    long exceptionCount=0;
    const size_t codeSize=_code_end-_code;
    std::vector<tuz_byte> _test_code(codeSize);
    std::vector<tuz_byte> _test_data(uncompress_size*2+1);
    tuz_byte* code=_test_code.data();
    tuz_byte* out_uncompress=_test_data.data();
    for (long i=0; i<kLoopCount; ++i) {
        sprintf(tag, "attackPacth exceptionCount=%ld testAttackSeed=%d i=%ld",exceptionCount,_attack_seed,i);
        memcpy(code,_code,codeSize);
        const long randCount=(long)(1+rand()*(1.0/RAND_MAX)*rand()*(1.0/RAND_MAX)*codeSize/3);
        for (long r=0; r<randCount; ++r){
            code[rand()%codeSize]=rand();
        }
        tuz_byte* code_end=code+codeSize;
        if ((rand()%8)==0){
            size_t lcodeSize=(size_t)(codeSize*rand()*(1.0/(RAND_MAX+1)));
            assert(lcodeSize<codeSize);
            code_end=code+lcodeSize;
        }
        size_t uncompress_size=_test_data.size();
        if ((rand()%8)==0){
            uncompress_size=(size_t)(uncompress_size*rand()*(1.0/(RAND_MAX+1)));
            assert(uncompress_size<_test_data.size());
        }

        try {
            tuz_decompress_stream(code,code_end,out_uncompress,&uncompress_size);
        } catch (...) {
            printf("exception!!! stream tag:%s\n", tag);
            return exceptionCount+1;
        }
        
        tuz_size_t _uncompress_size=(tuz_size_t)_test_data.size();
        try {
            tuz_decompress_mem(code,(tuz_size_t)(code_end-code),out_uncompress,&_uncompress_size);
        } catch (...) {
            printf("exception!!! mem tag:%s\n", tag);
            return exceptionCount+1;
        }
    }
    return exceptionCount;
}

static int test(const unsigned char* src,const unsigned char* src_end,const char* tag){
    std::vector<unsigned char> compressedCode((size_t)tuz_maxCompressedSize(src_end-src));
    hpatch_TStreamOutput out_stream;
    mem_as_hStreamOutput(&out_stream,compressedCode.data(),compressedCode.data()+compressedCode.size());
    hpatch_TStreamInput in_stream;
    mem_as_hStreamInput(&in_stream,src,src_end);
    tuz_TCompressProps props=tuz_kDefaultCompressProps;
    props.dictSize=kDictSize;
    props.maxSaveLength=kMaxSaveLength;
    hpatch_StreamPos_t codeSize=tuz_compress(&out_stream,&in_stream,&props);
    compressedCode.resize((size_t)codeSize);
    
    if (is_attack_decompress){
        error_count+=attack_decompress(compressedCode.data(),compressedCode.data()+compressedCode.size(),
                                       (tuz_size_t)(src_end-src),tag);
    }else{
        bool ret=false;
        std::vector<unsigned char> decompressedData(src_end-src,0);
        {
            size_t uncompress_size=decompressedData.size();
            tuz_TResult tret=tuz_decompress_stream(compressedCode.data(),compressedCode.data()+compressedCode.size(),
                                                   decompressedData.data(),&uncompress_size);
            ret=(tret==tuz_STREAM_END)&&(uncompress_size==decompressedData.size());
        }
        if (!ret){
            ++error_count;
            std::cout << "\nerror_count=="<<error_count<<" result error, tag==\""<<tag<<"\"\n";
        }else if (decompressedData!=std::vector<unsigned char>(src,src_end)){
            ++error_count;
            std::cout << "\nerror_count=="<<error_count<<" data error, tag==\""<<tag<<"\"\n";
        }else if(is_log_tag){
            std::cout << "error_count=="<<error_count<<", test ok  cmpSize/srcSize:"<<compressedCode.size()<<"/"<<src_end-src<<", tag==\""<<tag<<"\"\n";
        }

        memset(decompressedData.data(),0,src_end-src);
        {
            tuz_size_t uncompress_size=(tuz_size_t)decompressedData.size();
            tuz_TResult tret=tuz_decompress_mem(compressedCode.data(),(tuz_size_t)compressedCode.size(),
                                                decompressedData.data(),&uncompress_size);
            ret=(tret==tuz_STREAM_END)&&(uncompress_size==decompressedData.size());
        }
        if (!ret){
            ++error_count;
            std::cout << "\nerror_count=="<<error_count<<" result error, tag==\""<<tag<<"\"\n";
        }else if (decompressedData!=std::vector<unsigned char>(src,src_end)){
            ++error_count;
            std::cout << "\nerror_count=="<<error_count<<" data error, tag==\""<<tag<<"\"\n";
        }else if(is_log_tag){
            std::cout << "error_count=="<<error_count<<", test ok  cmpSize/srcSize:"<<compressedCode.size()<<"/"<<src_end-src<<", tag==\""<<tag<<"\"\n";
        }
    }
    return (int)compressedCode.size();
}
static void test_tuz(const unsigned char* src,const unsigned char* src_end,const char* tag){
    sum_src_size+=src_end-src;
    sum_cmz_size+=test(src,src_end,tag);
}

static void test_tuz(const char* src,const char* tag){
    size_t strLen=src?strlen(src):0;
    test_tuz((const unsigned char*)src,(const unsigned char*)src+strLen,tag);
}

int main(int argc, const char * argv[]){
    double  time0=clock_s();
    std::cout <<"tinyuz " TINYUZ_VERSION_STRING "\n";
    //*
    test_tuz(0,"null");
    test_tuz("","tag0");
    test_tuz("1","tag1");
    test_tuz("11","tag3");
    test_tuz("111","tag4");
    test_tuz("1111","tag5");
    test_tuz("11111","tag6");
    test_tuz("1111111111","tag7");
    test_tuz("11111111111111111111111111111111111111111111111111111111111111111111111111111111","tag8");
    test_tuz("1111111111111111111111111234111111111111111111111111111111111111111111111111","tag9");
    test_tuz("121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212","tag10");
    test_tuz("12121212121212121212g12121212121212121212121g21212121212121212121212125121212121231212122121212121212121212","tag11");
    test_tuz("34tg5h45y6hdfknw23u8ey23eewbd8djny45n54n89dfhuvvbe78fh43ufjhbvdsuy673673fb4ggbh"
             "hjerfuy34gfbehjfberuiyg734gfbhj34fjh34bf","tag12");
    test_tuz("234546457568792341354645756867782334253464576576857235346457658768768769789872342354"
             "35465476587698797436547658763254364575647568","tag13");
    //*/

    const int kMaxDataSize=1024*65;
    const int kMaxCopyCount=5000;
    std::vector<int> seeds(kRandTestCount);
    //srand( (unsigned int)time(0) );
    for (int i=0; i<kRandTestCount; ++i)
        seeds[i]=rand();

    //seeds[0]=?; //for debug error testSeed
    for (int i=0; i<kRandTestCount; ++i) {
        char tag[50];
        sprintf(tag, "testSeed=%d",seeds[i]);
        srand(seeds[i]);
        
        const int srcSize=(int)(pow(rand()*(1.0/RAND_MAX),3)*kMaxDataSize);
        std::vector<unsigned char> _srcData(srcSize);
        unsigned char* srcData=0; if (!_srcData.empty()) srcData=&_srcData[0];
        for (int i=0; i<srcSize; ++i)
            srcData[i]=rand();
        if (!is_all_rand){
            const int copyCount=(int)(rand()*(1.0/RAND_MAX)*kMaxCopyCount);
            const int kMaxCopyLength=1+(int)(pow(rand()*(1.0/RAND_MAX),3)*srcSize*0.3);
            for (int i=0; i<copyCount; ++i) {
                const int length=2+(int)(pow(rand()*(1.0/RAND_MAX),6)*kMaxCopyLength);
                if (length>=srcSize) {
                    continue;
                }
                const int oldPos=rand()%(srcSize-length);
                const int newPos=rand()%(srcSize-length);
                memmove(&srcData[0]+newPos, &srcData[0]+oldPos, length);
            }
        }
        test_tuz(&srcData[0],&srcData[0]+srcSize,tag);
    }
    
    std::cout << "\n  error_count=="<<error_count<<"\n";
    std::cout <<"  tinyuz: "<<" sum compressedSize/srcSize:"<<sum_cmz_size/sum_src_size<<"\n";
    std::cout << "\ndone!\n";
    printf("\n time: %.3f s\n",(clock_s()-time0));
    return error_count;
}

