//tinyuz_demo.cpp
// for tinyuz
 /*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
 */
#include <iostream>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include "../HDiffPatch/_clock_for_demo.h" //in HDiffPatch
#include "../HDiffPatch/_atosize.h"
#include "../HDiffPatch/file_for_patch.h"
#include "../HDiffPatch/libHDiffPatch/HDiff/private_diff/mem_buf.h"
#include "decompress/tuz_dec.h"
#include "compress/tuz_enc.h"

#ifndef _IS_NEED_MAIN
#   define  _IS_NEED_MAIN 1
#endif

typedef enum TTinyResult {
    TINY_SUCCESS=0,
    TINY_OPTIONS_ERROR,
    TINY_OPENREAD_ERROR,
    TINY_OPENWRITE_ERROR,
    TINY_FILECLOSE_ERROR,
    TINY_MEM_ERROR, // 5
    TINY_COMPRESS_ERROR,
    TINY_DECOMPRESS_ERROR,
} TTinyResult;

int tinyuz_cmd_line(int argc, const char * argv[]);
int tinyuz_by_file(const char* inputFile,const char* out_inputFile,bool isCompress,size_t runWithSize);

static void printHelpInfo(){
    //printf("tinyuz v" TINYUZ_VERSION_STRING "\n");
    std::cout << "compress   : tinyuz -c[-dictSize[k|m]]  inputFile outputFile\n";
    std::cout << "deccompress: tinyuz -d[-cacheSize[k|m]] inputFile outputFile\n";
}

#if (_IS_NEED_MAIN)
#   if (_IS_USED_WIN32_UTF8_WAPI)
int wmain(int argc,wchar_t* argv_w[]){
    hdiff_private::TAutoMem  _mem(hpatch_kPathMaxSize*4);
    char** argv_utf8=(char**)_mem.data();
    if (!_wFileNames_to_utf8((const wchar_t**)argv_w,argc,argv_utf8,_mem.size()))
        return TINY_OPTIONS_ERROR;
    SetDefaultStringLocale();
    return tinyuz_cmd_line(argc,(const char**)argv_utf8);
}
#   else
int main(int argc,char* argv[]){
    return  tinyuz_cmd_line(argc,(const char**)argv);
}
#   endif
#endif


#define _options_check(value,errorInfo){ \
    if (!(value)) { LOG_ERR("options " errorInfo " ERROR!\n\n"); printHelpInfo(); return TINY_OPTIONS_ERROR; } }

#define _kNULL_VALUE    ((tuz_BOOL)(-1))
#define _kNULL_SIZE     (~(size_t)0)

int tinyuz_cmd_line(int argc, const char * argv[]){
    if (argc!=4){
        printHelpInfo();
        return TINY_OPTIONS_ERROR;
    }

    tuz_BOOL isCompress=_kNULL_VALUE;
    size_t   runWithSize=_kNULL_SIZE;
    std::vector<const char *> arg_values;
    for (int i=1; i<argc; ++i) {
        const char* op=argv[i];
        _options_check(op!=0,"?");
        if (op[0]!='-'){
            arg_values.push_back(op); //file
            continue;
        }
        switch (op[1]) {
            case 'c':{
                _options_check((isCompress==_kNULL_VALUE)&&((op[2]=='-')||(op[2]=='\0')),"-c");
                isCompress=tuz_TRUE;
                if (op[2]=='-'){
                    const char* pnum=op+3;
                    _options_check(kmg_to_size(pnum,strlen(pnum),&runWithSize),"-c-?");
                }
            } break;
            case 'd':{
                _options_check((isCompress==_kNULL_VALUE)&&((op[2]=='-')||(op[2]=='\0')),"-d");
                isCompress=tuz_FALSE;
                if (op[2]=='-'){
                    const char* pnum=op+3;
                    _options_check(kmg_to_size(pnum,strlen(pnum),&runWithSize),"-d-?");
                }
            } break;
            default: {
                _options_check(tuz_FALSE,"-?");
            } break;
        }//switch
    }

    _options_check(arg_values.size()==2,"must input two files");
    _options_check(isCompress!=_kNULL_VALUE,"must run with -c or -d");

    if (runWithSize!=_kNULL_SIZE){
        const size_t minSize=isCompress?1:2;
        const size_t maxSize=tuz_kMaxOfDictSize;
        if (runWithSize<minSize) runWithSize=minSize;
        else if (runWithSize>maxSize) runWithSize=maxSize;
    }else{
        const size_t defaultSize=isCompress?tuz_kDefaultCompressProps.dictSize:(1024*256);
        runWithSize=defaultSize;
    }

    return tinyuz_by_file(arg_values[0],arg_values[1],isCompress?true:false,runWithSize);
}

struct TTuzListener{
    const hpatch_TStreamInput* data;
    hpatch_StreamPos_t         readPos;
    static tuz_BOOL read_code(void* listener,tuz_byte* out_code,tuz_size_t* code_size){
        TTuzListener* self=(TTuzListener*)listener;
        tuz_size_t r_len=*code_size;
        hpatch_StreamPos_t curReadPos=self->readPos;
        hpatch_StreamPos_t s_size=self->data->streamSize-curReadPos;
        if (r_len>s_size){
            r_len=(tuz_size_t)s_size;
            *code_size=r_len;
        }
        self->readPos+=r_len;
        return self->data->read(self->data,curReadPos,out_code,out_code+r_len);
    }
};

TTinyResult _tuz_decompress_stream(const hpatch_TStreamOutput* out_code,
                                   const hpatch_TStreamInput* data,size_t cache_size){
    TTuzListener  listener={data,0};
    tuz_TStream tuz;
    tuz_TResult result=tuz_OK;
    tuz_size_t dictSize=tuz_TStream_read_dict_size(&listener,listener.read_code);
    printf("  decompress with  dict size : %" PRIu64 "\n",(hpatch_StreamPos_t)dictSize);
    tuz_byte* _buf=0;
    cache_size>>=1;
    _buf=(tuz_byte*)malloc(dictSize+cache_size*2);
    if (_buf==0) return TINY_MEM_ERROR;
    result=tuz_TStream_open(&tuz,&listener,listener.read_code,_buf+cache_size,
                            (tuz_size_t)dictSize,(tuz_size_t)cache_size);
    hpatch_StreamPos_t data_size=0;
    while (result==tuz_OK) {
        tuz_size_t step_size=(tuz_size_t)cache_size;
        result=tuz_TStream_decompress_partial(&tuz,_buf,&step_size);
        if (result<=tuz_STREAM_END){
            if (out_code->write(out_code,data_size,_buf,_buf+step_size)){
                data_size+=step_size;
            }else{
                free(_buf); 
                return TINY_OPENWRITE_ERROR;
            }
        }
    }
    free(_buf);
    return (result==tuz_STREAM_END)?TINY_SUCCESS:TINY_DECOMPRESS_ERROR;
}

#define  _check_on_error(errorType) { \
    if (result==TINY_SUCCESS) result=errorType; if (!_isInClear){ goto clear; } }
#define check(value,errorType,errorInfo) { \
    std::string erri=std::string()+errorInfo+" ERROR!\n"; \
    if (!(value)){ hpatch_printStdErrPath_utf8(erri.c_str()); _check_on_error(errorType); } }

int tinyuz_by_file(const char* inputFile,const char* outputFile,bool isCompress,size_t runWithSize){
    int _isInClear=tuz_FALSE;
    TTinyResult result=TINY_SUCCESS;
    tuz_byte*   temp_cache=0;
    hpatch_TFileStreamInput   inputData;
    hpatch_TFileStreamOutput  outputData;
    hpatch_TFileStreamInput_init(&inputData);
    hpatch_TFileStreamOutput_init(&outputData);
    hpatch_StreamPos_t inputSize;
    hpatch_StreamPos_t outputSize;
    double time0=clock_s();
    
    check(hpatch_TFileStreamInput_open(&inputData,inputFile),TINY_OPENREAD_ERROR,"open inputFile");
    check(hpatch_TFileStreamOutput_open(&outputData,outputFile,~(hpatch_StreamPos_t)0),
          TINY_OPENREAD_ERROR,"open outputFile");
    inputSize=inputData.base.streamSize;
    printf("inputSize : %" PRIu64 " Bytes\n",inputSize);
    if (isCompress){
        if (runWithSize>=inputSize) runWithSize=(inputSize>1)?(size_t)inputSize-1:1;
        printf("  compress with dict size : %" PRIu64 "\n",(hpatch_StreamPos_t)runWithSize);
        try{
            tuz_TCompressProps props=tuz_kDefaultCompressProps;
            props.dictSize=runWithSize;
            outputSize=tuz_compress(&outputData.base,&inputData.base,&props);
            assert(outputSize==outputData.out_length);
        }catch(const std::exception& e){
            check(!inputData.fileError,TINY_OPENREAD_ERROR,"read inputFile");
            check(!outputData.fileError,TINY_OPENWRITE_ERROR,"write outputFile");
            check(false,TINY_COMPRESS_ERROR,"tuz_compress() run an error: "+e.what());
        }
    }else{
        printf("  decompress with cache size : %" PRIu64 "\n",(hpatch_StreamPos_t)(runWithSize>>1<<1));
        result=_tuz_decompress_stream(&outputData.base,&inputData.base,runWithSize);
        if (result!=TINY_SUCCESS){
            check(!inputData.fileError,TINY_OPENREAD_ERROR,"read inputFile");
            check(!outputData.fileError,TINY_OPENWRITE_ERROR,"write outputFile");
            check(false,result,"_tuz_decompress_stream()");
        }
        outputSize=outputData.out_length;
    }
    check(hpatch_TFileStreamOutput_close(&outputData),TINY_FILECLOSE_ERROR,"outpuFile close");
    {//out result info
        hpatch_StreamPos_t uncompressSize=isCompress?inputData.base.streamSize:outputSize;
        hpatch_StreamPos_t compressedSize=(!isCompress)?inputData.base.streamSize:outputSize;
        double runTime=(clock_s()-time0);
        printf("outputSize: %" PRIu64 " Bytes \t( %.3f %% )\n",outputSize,compressedSize*100.0/uncompressSize);
        printf("run   time: %.3f S     \t( %.3f MB/S )\n",runTime,uncompressSize/runTime/1024/1024);
        printf("  out file ok!\n"); 
    }
clear:
    _isInClear=tuz_TRUE;
    if (temp_cache) free(temp_cache);
    check(hpatch_TFileStreamOutput_close(&outputData),TINY_FILECLOSE_ERROR,"outpuFile close");
    check(hpatch_TFileStreamInput_close(&inputData),TINY_FILECLOSE_ERROR,"inputFile close");
    return result;
}