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
#ifdef NDEBUG
# ifndef assert
#   define  assert(expression) ((void)0)
# endif
#else
#   include <assert.h> //assert
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
#define TINYUZ_VERSION_MAJOR    0
#define TINYUZ_VERSION_MINOR    4
#define TINYUZ_VERSION_RELEASE  0
    
#define _TINYUZ_VERSION                 TINYUZ_VERSION_MAJOR.TINYUZ_VERSION_MINOR.TINYUZ_VERSION_RELEASE
#define _TINYUZ_QUOTE(str)              #str
#define _TINYUZ_EXPAND_AND_QUOTE(str)   _TINYUZ_QUOTE(str)
#define TINYUZ_VERSION_STRING           _TINYUZ_EXPAND_AND_QUOTE(_TINYUZ_VERSION)


#ifndef tuz_length_t
    //tuz_length_t must can saved CompressProps.maxSaveLength & dictSize value
    //  if tuz_length_t==uint8_t, must CompressProps.maxSaveLength & dictSize <= 255 when compress;
    //  if tuz_length_t==int16_t, must CompressProps.maxSaveLength & dictSize <= (2<<15)-1 ; ...
    typedef  unsigned int       tuz_length_t;
#endif
    typedef  tuz_length_t       tuz_dict_size_t;
#ifndef tuz_byte
    typedef  unsigned char      tuz_byte;
#endif
    typedef  tuz_byte           tuz_BOOL;
#define      tuz_FALSE      0
#define      tuz_TRUE       1
    
#ifndef tuz_inline
#ifdef _MSC_VER
#   define tuz_inline _inline
#else
#   define tuz_inline inline
#endif
#endif
    
#if 1
#   define tuz_try_inline
#else
#   define tuz_try_inline tuz_inline
#endif

#  define tuz_kMinDictMatchLen  3

#ifdef __cplusplus
}
#endif
#endif //_tuz_types_h
