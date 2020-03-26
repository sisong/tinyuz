//  tuz_types.h
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
#ifndef _tuz_types_h
#define _tuz_types_h

#ifdef __cplusplus
extern "C" {
#endif
    
#define TINYUZ_VERSION_MAJOR    0
#define TINYUZ_VERSION_MINOR    3
#define TINYUZ_VERSION_RELEASE  0
    
#define _TINYUZ_VERSION                 TINYUZ_VERSION_MAJOR.TINYUZ_VERSION_MINOR.TINYUZ_VERSION_RELEASE
#define _TINYUZ_QUOTE(str)              #str
#define _TINYUZ_EXPAND_AND_QUOTE(str)   _TINYUZ_QUOTE(str)
#define TINYUZ_VERSION_STRING           _TINYUZ_EXPAND_AND_QUOTE(_TINYUZ_VERSION)

    typedef  unsigned char      tuz_byte;
#ifndef tuz_length_t
    //if tuz_length_t==tuz_byte, must set CompressProps.maxSaveLength & dictSize <= 255
    typedef  unsigned int       tuz_length_t;
#endif
    typedef  tuz_length_t       tuz_dict_size_t;
    typedef  tuz_byte           tuz_BOOL;
#define      tuz_FALSE      0
#define      tuz_TRUE       ((tuz_BOOL)(!tuz_FALSE))

#ifdef _MSC_VER
#   define tuz_inline _inline
#else
#   define tuz_inline inline
#endif

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_h
