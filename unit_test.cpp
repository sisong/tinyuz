//  unit_test.cpp
//  for tinyuz

#include <iostream>
#include <string.h>
#include <math.h>
#include <vector>
#include "_clock_for_demo.h" //in HDiffPatch
#include "decompress/tuz_dec.h"
#include "compress/tuz_enc.h"

bool    is_log_tag=false;
int     error_count=0;
double  sum_src_size=0;
double  sum_cmz_size=0;


const tuz_size_t kDecodeCacheSize=1024*4;
const tuz_size_t kDictSize=1024*128;
struct TTuzListener{
    const unsigned char* src;
    const unsigned char* src_end;
    static tuz_BOOL read_code(void* listener,tuz_byte* out_code,tuz_size_t* code_size){
        TTuzListener* self=(TTuzListener*)listener;
        tuz_size_t r_size=*code_size;
        tuz_size_t s_size=self->src_end-self->src;
        if (r_size>s_size){
            r_size=s_size;
            *code_size=s_size;
        }
        memcpy(out_code,self->src,r_size);
        self->src+=r_size;
        return tuz_TRUE;
    }
    
    tuz_byte _mem_buf[kDictSize+kDecodeCacheSize];
    static void* alloc_mem(void* listener,tuz_size_t mem_size){
        //return malloc(mem_size);
        TTuzListener* self=(TTuzListener*)listener;
        assert(mem_size<=kDictSize);
        return self->_mem_buf;
    }
    static void free_mem(void* listener,void* pmem){
        //free(pmem);
    }
};

static int test(const unsigned char* src,const unsigned char* src_end,const char* tag){
    std::vector<unsigned char> compressedCode(tuz_maxCompressedSize(src_end-src));
    hpatch_TStreamOutput out_stream;
    mem_as_hStreamOutput(&out_stream,compressedCode.data(),compressedCode.data()+compressedCode.size());
    hpatch_TStreamInput in_stream;
    mem_as_hStreamInput(&in_stream,src,src_end);
    tuz_TCompressProps props;
    tuz_defaultCompressProps(&props);
    props.dictSize=kDictSize;
    hpatch_StreamPos_t codeSize=tuz_compress(&out_stream,&in_stream,&props);
    compressedCode.resize(codeSize);
    
    bool ret=false;
    std::vector<unsigned char> decompressedData(src_end-src,0);
    tuz_TStream tuz;
    tuz_TStream_init(&tuz);
    TTuzListener  listener={compressedCode.data(),compressedCode.data()+compressedCode.size()};
    tuz.listener=&listener;
    tuz.read_code=listener.read_code;
    tuz.alloc_mem=listener.alloc_mem;
    tuz.free_mem=listener.free_mem;
    if (tuz_OK==tuz_TStream_open(&tuz,&listener._mem_buf[kDictSize],kDecodeCacheSize)){
        tuz_size_t data_size=decompressedData.size();
        tuz_TResult tret=tuz_TStream_decompress(&tuz,decompressedData.data(),&data_size);
        tuz_TStream_close(&tuz);
        ret=(tret==tuz_STREAM_END)&&(data_size==decompressedData.size());
    }
    if (!ret){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<" result error, tag==\""<<tag<<"\"\n";
    }else if (decompressedData!=std::vector<unsigned char>(src,src_end)){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<" data error, tag==\""<<tag<<"\"\n";
    }else if(is_log_tag){
        std::cout << "error_count=="<<error_count<<", test ok  frzSize/srcSize:"<<compressedCode.size()<<"/"<<src_end-src<<", tag==\""<<tag<<"\"\n";
    }
    return (int)compressedCode.size();
}
static void test_tuz(const unsigned char* src,const unsigned char* src_end,const char* tag){
    sum_src_size+=src_end-src;
    sum_cmz_size+=test(src,src_end,tag);
}

static void test_tuz(const char* src,const char* tag){
    if (src!=0)
        test_tuz((const unsigned char*)src,(const unsigned char*)src+strlen(src),tag);
    else
        test_tuz(0,0,tag);
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
    
    const int kRandTestCount=2000;
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
        test_tuz(&srcData[0],&srcData[0]+srcSize,tag);
    }
    
    std::cout << "\n  error_count=="<<error_count<<"\n";
    std::cout <<"  tinyuz: "<<" sum compressedSize/srcSize:"<<sum_cmz_size/sum_src_size<<"\n";
    std::cout << "\ndone!\n";
    printf("\n time: %.3f s\n",(clock_s()-time0));
    return error_count;
}

