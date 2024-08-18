//test_others.h
#ifndef tinyuz_test_others_h
#define tinyuz_test_others_h


#include "../minilzo/minilzo.h"
// GNU GENERAL PUBLIC LICENSE   Version 2

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

int minilzo_compress(unsigned char* out_data,unsigned char* out_data_end,
                     const unsigned char* src,const unsigned char* src_end){
    HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);
    lzo_uint dst_len=out_data_end-out_data;
    int ret=lzo1x_1_compress(src,src_end-src,out_data,&dst_len,wrkmem);
    if (ret!=LZO_E_OK)
        return 0;
    return dst_len;
}

bool minilzo_decompress(unsigned char* out_data,unsigned char* out_data_end,
                        const unsigned char* zip_code,const unsigned char* zip_code_end){
    lzo_uint dst_len=out_data_end-out_data;
    int ret=lzo1x_decompress_safe(zip_code,zip_code_end-zip_code,out_data,&dst_len,0);
    return (ret==LZO_E_OK);
}



#include "../quicklz/src/quicklz.h"
// GNU GENERAL PUBLIC LICENSE   Version 2

int quicklz_compress(unsigned char* out_data,unsigned char* out_data_end,
                     const unsigned char* src,const unsigned char* src_end){
    qlz_state_compress *state_compress = (qlz_state_compress *)malloc(sizeof(qlz_state_compress));
    int c=qlz_compress(src,(char*)out_data,src_end-src, state_compress);
    free(state_compress);
    return c;
}

bool quicklz_decompress(unsigned char* out_data,unsigned char* out_data_end,
                        const unsigned char* zip_code,const unsigned char* zip_code_end){
    qlz_state_decompress *state_decompress = (qlz_state_decompress *)malloc(sizeof(qlz_state_decompress));
    if (zip_code_end-zip_code<9) return false;
    int dc = qlz_size_decompressed((const char*)zip_code);
    int c = qlz_size_compressed((const char*)zip_code);
    if (out_data_end-out_data<dc) return false;
    if (zip_code_end-zip_code<c) return false;

    int d = qlz_decompress((const char*)zip_code, out_data, state_decompress);
    free(state_decompress);
    return (d==out_data_end-out_data);
}



#include "../fastlz/fastlz.h"
// MIT LICENSE

int _fastlz_compress(unsigned char* out_data,unsigned char* out_data_end,
                     const unsigned char* src,const unsigned char* src_end){
    return fastlz_compress_level(2,src,src_end-src,out_data);
}

bool _fastlz_decompress(unsigned char* out_data,unsigned char* out_data_end,
                        const unsigned char* zip_code,const unsigned char* zip_code_end){
    int d=fastlz_decompress(zip_code,zip_code_end-zip_code,out_data,out_data_end-out_data);
    return (d==out_data_end-out_data);
}



#if defined(__cplusplus)
extern "C" {
#endif

#include "../heatshrink/heatshrink_encoder.h"
#include "../heatshrink/heatshrink_decoder.h"
// ISC LICENSE

#if defined(__cplusplus)
}
#endif

// test fail when hs_windowBits=15
uint8_t hs_windowBits   =10; // [4--15]
uint8_t hs_lookaheadBits=4;  // [3--hs_windowBits)
int heatshrink_compress(unsigned char* out_data,unsigned char* out_data_end,
                     const unsigned char* src,const unsigned char* src_end){
    heatshrink_encoder* hse=heatshrink_encoder_alloc(hs_windowBits,hs_lookaheadBits);
    if (!hse) return 0;

    size_t output_size=0;
    HSE_sink_res sres;
    HSE_poll_res pres;
    HSE_finish_res fres;
    do {
        if (src<src_end) {
            size_t sink_sz;
            sres = heatshrink_encoder_sink(hse,(unsigned char*)src,src_end-src, &sink_sz);
            if (sres < 0) { output_size=0; break; }
            src += sink_sz;
        }
        
        size_t poll_sz;
        do {
            pres = heatshrink_encoder_poll(hse, out_data, out_data_end-out_data, &poll_sz);
            if (pres < 0) { output_size=0; break; }
            output_size+=poll_sz;
            out_data+=poll_sz;
        } while (pres == HSER_POLL_MORE);
        if (pres < 0) break;
        
        if (poll_sz == 0 && src==src_end) {
            fres = heatshrink_encoder_finish(hse);
            if (fres < 0) { output_size=0; break; }
            if (fres == HSER_FINISH_DONE) { break; }
        }
    } while (true);

    heatshrink_encoder_free(hse);
    return output_size;
}

bool heatshrink_decompress(unsigned char* out_data,unsigned char* out_data_end,
                        const unsigned char* zip_code,const unsigned char* zip_code_end){
    heatshrink_decoder *hsd = heatshrink_decoder_alloc(out_data_end-out_data,
                                hs_windowBits,hs_lookaheadBits);
    if (!hsd) return false;

    bool result=true;
    HSD_sink_res sres;
    HSD_poll_res pres;
    HSD_finish_res fres;

    do {
        if (zip_code<zip_code_end) {
            size_t sink_sz;
            sres = heatshrink_decoder_sink(hsd,(uint8_t*)zip_code,zip_code_end-zip_code, &sink_sz);
            if (sres < 0) { result=false; break; }
            zip_code += sink_sz;
        }

        size_t poll_sz;
        do {
            pres = heatshrink_decoder_poll(hsd, out_data, out_data_end-out_data, &poll_sz);
            if (pres < 0) { result=false; break; }
            out_data+=poll_sz;
        } while ((pres == HSDR_POLL_MORE)&&(out_data<out_data_end));
        if (pres < 0) break;
        
        if (zip_code==zip_code_end && poll_sz == 0) {
            fres = heatshrink_decoder_finish(hsd);
            if (fres < 0) { result=false; break; }
            if (fres == HSDR_FINISH_DONE) { break; }
        }
    } while (true);

    heatshrink_decoder_free(hsd);
    return result;
}


#endif //tinyuz_test_others_h