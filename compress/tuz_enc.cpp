//  tuz_enc.cpp
/*
 Copyright (c) 2012-2020 HouSisong All Rights Reserved.
 (The MIT License)
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/
#include "tuz_enc.h"
#include "tuz_enc_private/tuz_enc_clip.h"
using namespace _tuz_private;

void tuz_defaultCompressProps(tuz_TCompressProps* out_props){
    out_props->dictSize=1024*16;
    out_props->maxStepLength=1024*64-1;
    out_props->minDictMatchLen=2;
    out_props->threadNum=1;
}

void tuz_compress(const hpatch_TStreamOutput* out_code,const hpatch_TStreamInput* data,const tuz_TCompressProps* props){
    assert(out_code&&(out_code->write));
    assert(data&&(data->read));
    assert(props);
    assert(props->dictSize>=1);
    assert(props->maxStepLength>=63);
    assert(props->minDictMatchLen>=2);
    
    std::vector<tuz_byte> code;
    compress_clip(code,data,0,data->streamSize,*props);
    checkv(out_code->write(out_code,0,code.data(),code.data()+code.size()));
}

