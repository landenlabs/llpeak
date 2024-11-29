//-------------------------------------------------------------------------------------------------
//  File: FBlur.hpp
//  Desc: Image "blur" functions using FreeImage
//
//  FBlur created by Dennis Lang on 12/21/21.
//  Copyright © 2022 Dennis Lang. All rights reserved.
//
//  FreeImage 3  Design and implementation by
//  - Floris van den Berg (flvdberg@wxs.nl)
//  - Herv<E9> Drolon (drolon@infonie.fr)
//
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

// Project files
class ImageAux;         // #include "ImageAux.hpp"  Circular reference
#include "ImageCfg.hpp"
#include "FImage.hpp"
#include "FPalette.hpp"


//-------------------------------------------------------------------------------------------------
class FBlur {
public:
    static
    bool blurI8(const PalMapping& mapping, const FPalette& outPalette, const FImage& inI8, FImage& outP32, ImageCfg& cfg, ImageAux& aux, unsigned radius);
    
    static void toFloat( const PalMapping& mapping, const FImage& inI8, unsigned width, unsigned height, float* grid);
    static void hBlur(unsigned radius, unsigned xDim, unsigned yDim, const float* inGrid, float* outGrid);
    static void vBlur(unsigned radius, unsigned xDim, unsigned yDim, const float* inGrid, float* outGrid);
    static void toPixel32(const float* inGrid, const FPalette& inPalette, unsigned width, unsigned height, FImage& outP32);

};
