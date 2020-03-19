//  unit_test.cpp
//  for tinyuz

#include <iostream>
#include <string.h>
#include <math.h>
#include <vector>
#include "decompress/tuz_dec.h"
#include "compress/tuz_enc.h"

int     error_count=0;
double  sum_src_size=0;
double  sum_cmz_size=0;

static int test(const unsigned char* src,const unsigned char* src_end,const char* tag){
    std::vector<unsigned char> compressedCode;
    /*tuz_compress(compressedCode,src,src_end,0);
    
    std::vector<unsigned char> uncompressedCode(src_end-src,0);
    frz_BOOL ret=tuz_decompress(&uncompressedCode[0],&uncompressedCode[0]+uncompressedCode.size(), &compressedCode[0], &compressedCode[0]+compressedCode.size());
    if (!ret){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<", "<<tuz_decompress_Name<<" result error, tag==\""<<tag<<"\"\n";
    }else if (uncompressedCode!=std::vector<unsigned char>(src,src_end)){
        ++error_count;
        std::cout << "\nerror_count=="<<error_count<<", "<<tuz_decompress_Name<<" data error, tag==\""<<tag<<"\"\n";
    }else{
        std::cout << "error_count=="<<error_count<<", test ok "<<tuz_compress_Name<<" frzSize/srcSize:"<<compressedCode.size()<<"/"<<src_end-src<<", tag==\""<<tag<<"\"\n";
    }*/
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
    std::cout <<"tinyuz " TINYUZ_VERSION_STRING "\n";
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
    
    const int kRandTestCount=10000;
    const int kMaxDataSize=1024*65;
    const int kMaxCopyCount=5000;
    std::vector<int> seeds(kRandTestCount);
    //srand( (unsigned int)time(0) );
    for (int i=0; i<kRandTestCount; ++i)
        seeds[i]=rand();
    
    
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
    return error_count;
}

