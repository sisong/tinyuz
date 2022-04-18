//  tuz_types_private.h
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
#ifndef _tuz_types_private_h
#define _tuz_types_private_h

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum tuz_TCodeType{
        tuz_codeType_data=0,
        tuz_codeType_dict=1,
    } tuz_TCodeType;
    
    typedef enum tuz_TCtrlType{
        tuz_ctrlType_streamEnd=1,
        tuz_ctrlType_clipEnd=2,
    } tuz_TCtrlType;
    
    #define tuz_kMinLiteralLen      16
    #define tuz_kMinDictMatchLen    2
    #define tuz_kMaxTypeBitCount    8

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_private_h
