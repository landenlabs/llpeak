//-------------------------------------------------------------------------------------------------
// File: PalMapping.hpp
// Desc: Store from and to pixel mapping palette index arrays. 
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2022
// https://landenlabs.com
//
// This file is part of llpeak project.
//
// ----- License ----
//
// Copyright (c) 2022  Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

typedef unsigned char BYTE;

//-------------------------------------------------------------------------------------------------
// Indexed Color palette mapping
class PalMapping {
    static const unsigned LEN = 256;
public:
    // FPalette& refPalette;
    BYTE from[LEN];
    BYTE to[LEN];
    float dist[LEN];
    bool isReady = false;
    unsigned shiftCnt = 0;

    void clear() {
        isReady = false;
        shiftCnt = 0;
        memset(to, 0, sizeof(to));
        memset(dist, 0, sizeof(dist));
    }
    void init() {
        clear();
        isReady = true;
        for (unsigned idx = 0; idx < sizeof(from); idx++) from[idx] = (BYTE)idx;
    }
};
