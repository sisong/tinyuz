//  tuz_enc.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_enc.h"
#include "tuz_enc_private/tuz_enc_clip.h"
#include <algorithm> //std::max
#include "../../HDiffPatch/libParallel/parallel_channel.h"
using namespace _tuz_private;

#define tuz_kDefaultDictSize  (1<<24)

const tuz_TCompressProps tuz_kDefaultCompressProps={tuz_kDefaultDictSize,tuz_kMaxOfMaxSaveLength,1,
                                                    tuz_isNeedLiteralLine?true:false};

static const size_t   kMaxPackedPosByteSize =sizeof(hpatch_StreamPos_t)*3/2+1;

hpatch_StreamPos_t tuz_maxCompressedSize(hpatch_StreamPos_t data_size){
    const hpatch_StreamPos_t u_size=data_size/8+1;
    hpatch_StreamPos_t c_count=data_size/kMinBestClipSize+1;
    return data_size + u_size + 1+kMaxPackedPosByteSize + 4*c_count + 4;
}

inline void _flush_code(const hpatch_TStreamOutput* out_code,hpatch_StreamPos_t& cur_out_pos,std::vector<tuz_byte>& code){
    checkv(out_code->write(out_code,cur_out_pos,code.data(),code.data()+code.size()));
    cur_out_pos+=code.size();
    code.clear();
}

#if (_IS_USED_MULTITHREAD)

struct _stream_mtsafe_t {
    hpatch_TStreamOutput        base;
    const hpatch_TStreamOutput* _stream;
    CHLocker                    _locker;
    hpatch_StreamPos_t          _sumWrited;
    inline explicit _stream_mtsafe_t(const hpatch_TStreamOutput* stream) 
      :_stream(stream),_sumWrited(0) {
        base.streamImport = this;
        base.streamSize = stream->streamSize;
        base.read_writed = (stream->read_writed) ? _read_writed : 0;
        base.write = (stream->write) ? _write : 0;
    }
    static hpatch_BOOL _read_writed(const struct hpatch_TStreamOutput* stream, hpatch_StreamPos_t readFromPos,
        unsigned char* out_data, unsigned char* out_data_end) {
        _stream_mtsafe_t& self = *(_stream_mtsafe_t*)stream->streamImport;
        CAutoLocker _auto_locker(self._locker.locker);
        return self._stream->read_writed(self._stream, readFromPos, out_data, out_data_end);
    }
    static hpatch_BOOL _write(const struct hpatch_TStreamOutput* stream, hpatch_StreamPos_t writeToPos,
        const unsigned char* data, const unsigned char* data_end) {
        _stream_mtsafe_t& self = *(_stream_mtsafe_t*)stream->streamImport;
        CAutoLocker _auto_locker(self._locker.locker);
        self._sumWrited+=(size_t)(data_end-data);
        return self._stream->write(self._stream,writeToPos,data,data_end);
    }
};

    struct TWorkBuf{
        hpatch_StreamPos_t      clipBegin;
        hpatch_StreamPos_t      clipEnd;
        struct TWorkBuf*        next;
        std::vector<tuz_byte>   code;
    };

struct TMt:public TMtByChannel{
    tuz_TCompressProps  selfProps;

    _stream_mtsafe_t    out_code;
    _stream_mtsafe_t    data;
    TMt(const hpatch_TStreamOutput* _out_code,const hpatch_TStreamInput* _data)
    :out_code(_out_code),data((hpatch_TStreamOutput*)_data){}

    hpatch_StreamPos_t  clipSize;
    hpatch_StreamPos_t  curWorkClipPos;
    hpatch_StreamPos_t  curOutedClipPos;
    hpatch_StreamPos_t  curWritePos;
    size_t              curDictSizeMax;
    TWorkBuf*           workBufList;

    inline TWorkBuf* getWork(){
        if (is_on_error()||(curWorkClipPos==data.base.streamSize)) return 0;
        TWorkBuf* work=(TWorkBuf*)work_chan.accept(true);
        if (work==0) return 0;
        
        CAutoLocker _auto_locker(_locker.locker);
        work->clipBegin=curWorkClipPos;
        work->clipEnd=curWorkClipPos+clipSize;
        if (work->clipEnd>data.base.streamSize) work->clipEnd=data.base.streamSize;
        curWorkClipPos=work->clipEnd;
        if (work->clipBegin>=work->clipEnd) return 0;
        return work;
    }
    void finishWork(TWorkBuf* work,size_t _curDictSize){
        hpatch_StreamPos_t cur_out_pos;
        while (true){
            {
                CAutoLocker _auto_locker(_locker.locker);
                curDictSizeMax=std::max(curDictSizeMax,_curDictSize);
                if (work){
                    if (work->clipBegin!=curOutedClipPos){ //push
                        TWorkBuf** insertBuf=&workBufList;
                        while ((*insertBuf)&&((*insertBuf)->clipBegin<work->clipBegin))
                            insertBuf=&((*insertBuf)->next);
                        work->next=*insertBuf;
                        *insertBuf=work;
                        work=0;
                    }
                }else if (workBufList&&(workBufList->clipBegin==curOutedClipPos)){ //pop
                    work=workBufList;
                    workBufList=workBufList->next;
                }
                if (work){
                    cur_out_pos=curWritePos;
                    curWritePos+=work->code.size();
                    curOutedClipPos=work->clipEnd;
                }
            }
            if (work==0) break;

            _flush_code(&out_code.base,cur_out_pos,work->code);
            checkv(work_chan.send(work,true));
            work=0;
        }
    }
};

static void _tuz_compress_mt(int threadIndex,void* workData){
    TMt& mt=*(TMt*)workData;
    TMt::TAutoThreadEnd __auto_end(mt);
    const hpatch_TStreamInput* data=(hpatch_TStreamInput*)&mt.data.base;

    TDictBuf dict_buf;
    try{
        while(TWorkBuf* work=mt.getWork()) {
            hpatch_StreamPos_t clipBegin=work->clipBegin;
            hpatch_StreamPos_t clipEnd=work->clipEnd;
            std::vector<tuz_byte>& code=work->code;
            bool isToStreamEnd=(clipEnd>=data->streamSize);

            assert(code.empty());
            TTuzCode coder(code,mt.selfProps.isNeedLiteralLine);
            if (clipBegin<clipEnd){
                compress_clip(coder,data,clipBegin,clipEnd,mt.selfProps,dict_buf);
            }
            if (!isToStreamEnd)
                coder.outCtrl_clipEnd();
            else
                coder.outCtrl_streamEnd();
            mt.finishWork(work,coder.getCurDictSizeMax());
        }
    } catch (...){
        mt.on_error();
    }
}

#endif

hpatch_StreamPos_t tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,
                                const tuz_TCompressProps* props){
    checkv(out_code&&(out_code->write));
    checkv(data&&(data->read));
    if (props){
        checkv((props->dictSize>=1)&(props->dictSize<=tuz_kMaxOfDictSize));
        checkv(props->dictSize==(tuz_size_t)props->dictSize);
        checkv(props->maxSaveLength==(tuz_length_t)props->maxSaveLength);
        checkv((props->maxSaveLength>=tuz_kMinOfMaxSaveLength)&&(props->maxSaveLength<=tuz_kMaxOfMaxSaveLength));
    }
    
    tuz_TCompressProps selfProps=(props)?*props:tuz_kDefaultCompressProps;
    if (selfProps.dictSize>data->streamSize){
        selfProps.dictSize=(size_t)(data->streamSize);
        if (selfProps.dictSize==0)
            selfProps.dictSize=1;
    }
    
    hpatch_StreamPos_t cur_out_pos=0;
    std::vector<tuz_byte> code;
    {//head
        assert(code.empty());
        TTuzCode coder(code,selfProps.isNeedLiteralLine);
		checkv(selfProps.dictSize==(tuz_size_t)selfProps.dictSize);
        checkv(selfProps.maxSaveLength==(tuz_length_t)selfProps.maxSaveLength);
        coder.outDictSize(selfProps.dictSize);
        _flush_code(out_code,cur_out_pos,code);
    }

    size_t curDictSizeMax=tuz_kMinOfDictSize;
    hpatch_StreamPos_t clipSize;
    size_t threadNum=props->threadNum;
    {
        clipSize=((hpatch_StreamPos_t)selfProps.dictSize+1)/3;
        if (clipSize<kMinBestClipSize) clipSize=kMinBestClipSize;
        if (clipSize>kMaxBestClipSize) clipSize=kMaxBestClipSize;
        hpatch_StreamPos_t clipCount=(data->streamSize+clipSize)/clipSize;
        clipSize=(data->streamSize+clipCount-1)/clipCount;
        if (threadNum>clipCount) threadNum=(size_t)clipCount;
    }
        
#if (_IS_USED_MULTITHREAD)
    if (threadNum>1){
        TMt mt(out_code,data);
        mt.selfProps=selfProps;
        mt.clipSize=clipSize;
        mt.curWorkClipPos=0;
        mt.curOutedClipPos=0;
        mt.curWritePos=cur_out_pos;
        mt.curDictSizeMax=curDictSizeMax;
        mt.workBufList=0;
        std::vector<TWorkBuf> _codeList;
        _codeList.resize(threadNum+1+threadNum/2);
        for (size_t i=0;i<_codeList.size();++i)
            checkv(mt.work_chan.send(&_codeList[i],true));
        mt.start_threads((int)threadNum,_tuz_compress_mt,&mt,true);

        mt.wait_all_thread_end();
        checkv(!mt.is_on_error());
        checkv(mt.curOutedClipPos==data->streamSize);
        curDictSizeMax=mt.curDictSizeMax;
        cur_out_pos=mt.curWritePos;
    }else
#endif
    {
        TDictBuf dict_buf;
        for (hpatch_StreamPos_t clipBegin=0;true;clipBegin+=clipSize) {
            hpatch_StreamPos_t clipEnd=clipBegin+clipSize;
            bool isToStreamEnd=(clipEnd>=data->streamSize);
            if (isToStreamEnd) clipEnd=data->streamSize;

            assert(code.empty());
            TTuzCode coder(code,selfProps.isNeedLiteralLine);
            if (clipBegin<clipEnd){
                compress_clip(coder,data,clipBegin,clipEnd,selfProps,dict_buf);
            }
            if (!isToStreamEnd)
                coder.outCtrl_clipEnd();
            else
                coder.outCtrl_streamEnd();
            
            curDictSizeMax=std::max(curDictSizeMax,coder.getCurDictSizeMax());
            _flush_code(out_code,cur_out_pos,code);
            if (isToStreamEnd) break;
        }
    }

    {//update dictSize
        checkv(curDictSizeMax<=selfProps.dictSize);
        if (curDictSizeMax<selfProps.dictSize){
            assert(code.empty());
            TTuzCode coder(code,selfProps.isNeedLiteralLine);
            coder.outDictSize(curDictSizeMax);
            hpatch_StreamPos_t dict_out_pos=0;
            _flush_code(out_code,dict_out_pos,code);
        }
    }

    return cur_out_pos;
}

