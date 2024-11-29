//-------------------------------------------------------------------------------------------------
//  File: ImageUtilF.hpp
//  Desc: Image manipulation utility functions using FreeImage.
//
//  ImageUtilF created by Dennis Lang on 12/21/21.
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
#include "FImage.hpp"
#include "FPalette.hpp"
#include "FBrush.hpp"
#include "ImageAux.hpp"
#include "ImageCfg.hpp"

// C++
#include <iostream>


// Image manipulation actions.
class ImageUtilF {
    
public:
    static bool saveTo(const FImage& img, const char* toName, bool verbose = false);
    static bool threadSaveAndCloseTo(const FImage& img, const char* toName, ImageAux& aux);
    static FImage& LoadImage(FImage& img, const char* fullname);
    static FImage& MakeTestI8(FImage& outI8, unsigned width, unsigned height, ImageCfg& cfg);
    
    static void FreeImageErrorHandler(FREE_IMAGE_FORMAT imgFmt, const char *message) {
        std::cerr << "\nFreeImage error ";
        if (imgFmt != FIF_UNKNOWN) {
            std::cerr << FreeImage_GetFormatFromFIF(imgFmt);
        }
        std::cerr << message << std::endl;
    }
    
    static bool initDone;
    static void init() {
        if (!initDone) {
            // Call this ONLY when linking with FreeImage as a static library
            FreeImage_Initialise();
            FreeImage_SetOutputMessage(FreeImageErrorHandler);
            std::cout << "\n" << FreeImage_GetCopyrightMessage() << "\nFreeImage v" << FreeImage_GetVersion() << std::endl;
            initDone = true;
        }
    }
    

    // Main "Montage" function
    static bool Montage(const FPalette& inPal, StringList inPaths, int xTiles, int yTiles, const lstring& outputPath);
    
    // Main "Dump" function
    static void Dump(const lstring& imagePath);
    static void Palette(const lstring& imagePath);
    static bool saveLegend(const FPalette& palette, const lstring& outFileName);

    // Main "Blur" function
    static bool Blur(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux);
    static bool BlurI8(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, const FImage& imgI8);
    
    // Main "Shade" function (must set shade function in ImageAux)
    static bool Shade(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux);
    static bool ShadeI8(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, FImage& imgI8);
    static bool ShadeP32(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, FImage& imgP32);
    
    // Main "ToGray function
    static bool ToGray(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux);
    
    // Main "Colorlapse" function
    static bool Colorlapse(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux);
    static void ColorlapseI8(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, FImage& imgI8);
    
    // Main "Blend" function.
    static bool Blend(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux);
    static bool BlendFade(const lstring& imagePath, unsigned extraFrames, ImageCfg& cfg, ImageAux& aux);
    // Blend support functions
    static void BlendI8(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, FImage& imgI8);
    static void BlendP32(const lstring& imagePath, ImageCfg& cfg, ImageAux& aux, FImage& imgP32);
        
    static FImage& BlendP32(const FImage& topImgP32, const FImage& botImgP32, FImage& outImgP32);
    static FImage& BlendI8_P32(const FPalette& topPalette, const FImage& topImgI8,  FImage& botImgP32);
    static FImage& BlendI8_P32(const FImage& topImgI8, const FImage& botImgI8, FImage& outImgP32);
    static FImage& BlendP32_I8(const FImage& topImgP32, const FImage& botImgI8, FImage& outImgP32);
    static FImage& MaximumI8(const FImage& inImgI8, FImage& outImgI8);       // out = max(in, out)
    static unsigned BestMapping(const FPalette& srcPalette, const FPalette& dstPalette, const BYTE* dstMapping, PalMapping& mappings);
    static void AdjustAlpha(float percent, const FImage& imgP32); 
};


