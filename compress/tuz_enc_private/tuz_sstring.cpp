//  tuz_sstring.cpp
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong All Rights Reserved.
*/
#include "tuz_sstring.h"
#include "../../../HDiffPatch/libHDiffPatch/HDiff/private_diff/libdivsufsort/divsufsort.h"

namespace _tuz_private{
    
    typedef TSuffixString::TInt     TInt;
    typedef TSuffixString::TLCPInt  TLCPInt;
    
    static void _Rank_create(TInt n,const TInt* SA,TInt* R){
        for (TInt i=0;i<n;++i){
            R[SA[i]]=i;
        }
    }
    
    static void _LCP_create_withR(const tuz_byte* T,TInt n,const TInt* SA,const TInt* R,
                                  TLCPInt* LCP,TLCPInt maxLCPValue){
        if (n>0)
            LCP[n-1]=0;
        for (TInt h=0,i=0; i<n; ++i){
            if (R[i]==0) continue;
            TInt j = SA[R[i]-1];
            while (((i+h!=n)&(j+h!=n))&&(T[i+h]==T[j+h]))
                ++h;
            LCP[R[i]-1]=(TLCPInt)((h<=maxLCPValue)?h:maxLCPValue);
            if (h>0) --h;
        }
    }

void TSuffixString::_init(TLCPInt maxLCPValue){
    size_t sa_size=size();
    checkv((sa_size==(size_t)(TInt)sa_size)&&(0<=(TInt)sa_size));
    SA.resize(sa_size);
    R.resize(sa_size);
    LCP.resize(sa_size);
    if (sa_size==0) return;
    
    checkv(0==divsufsort(src,(saidx32_t*)SA.data(),(saidx32_t)sa_size,(int)threadNum));
    _Rank_create((TInt)SA.size(),SA.data(),R.data());
    _LCP_create_withR(src,(TInt)SA.size(),SA.data(),R.data(),LCP.data(),maxLCPValue);
}

}
