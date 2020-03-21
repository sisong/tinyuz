//  tuz_sstring.cpp
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
#include "tuz_sstring.h"
#include "libHDiffPatch/HDiff/private_diff/libdivsufsort/divsufsort.h"

namespace _tuz_private{
    
    typedef TSuffixString::TInt TInt;
    
    static void _Rank_create(TInt n,const TInt* SA,TInt* R){
        for (TInt i=0;i<n;++i){
            R[SA[i]]=i;
        }
    }
    
    static void _LCP_create_withR(const tuz_byte* T,TInt n,const TInt* SA,const TInt* R,TInt* LCP){
        if (n>0)
            LCP[n-1]=0;
        for (TInt h=0,i=0; i<n; ++i){
            if (R[i]==0) continue;
            TInt j = SA[R[i]-1];
            while (((i+h!=n)&&(j+h!=n))&&(T[i+h]==T[j+h]))
                ++h;
            LCP[R[i]-1]=h;
            if (h>0) --h;
        }
    }

void TSuffixString::_init(TInt maxLCPValue){
    size_t sa_size=size();
    checkv(_uint_is_less_2g(sa_size));
    SA.resize(sa_size);
    R.resize(sa_size);
    LCP.resize(sa_size);
    if (sa_size==0) return;
    
    checkv(0==divsufsort(src,(saidx_t*)SA.data(),(saidx_t)sa_size));
    _Rank_create((TInt)SA.size(),SA.data(),R.data());
    _LCP_create_withR(src,(TInt)SA.size(),SA.data(),R.data(),LCP.data());
    for (size_t i=0; i<sa_size; ++i) {
        if (LCP[i]<=maxLCPValue)
            ;//continue;
        else
            LCP[i]=maxLCPValue;
    }
}

}
