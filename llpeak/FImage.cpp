//-------------------------------------------------------------------------------------------------
//  File: FImage.cpp
//  Desc: FreeImage C++ wrapper
//
//  FImage created by Dennis Lang on 12/21/21.
//  Copyright © 2022 Dennis Lang. All rights reserved.
//
//  FreeImage 3  Design and implementation by
//  - Floris van den Berg (flvdberg@wxs.nl)
//  - Herv<E9> Drolon (drolon@infonie.fr)
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

#include "FImage.hpp"
#include <iostream>
#include <math.h>

unsigned FImage::DBG_CNT = 0;

void FIMAGE_DELETER(FIBITMAP* imgPtr) {
    FreeImage_Unload(imgPtr);
}
FBitmapRef::FBitmapRef(FIBITMAP* imgPtr) : shared_ptr(imgPtr, FIMAGE_DELETER) {
    
}

//-------------------------------------------------------------------------------------------------
FImage::FImage(FIBITMAP* _imgPtr) : imgPtr(_imgPtr) { 
    DBG_CNT++; 
}

//-------------------------------------------------------------------------------------------------
void FImage::Close() {
    if (Valid()) {
        // FreeImage_Unload(imgPtr);
        imgPtr = nullptr;
        // std::cout << "close cnt=" << --DBG_CNT << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
bool FImage::LoadFromHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags) {
    Close();
    DBG_CNT++;
    imgPtr = FreeImage_LoadFromHandle(fif, io, handle, flags);
    return Valid();
}

//-------------------------------------------------------------------------------------------------
void FImage::FillImage(const FColor& color) {
    unsigned width = GetWidth();
    unsigned height = GetHeight();
    unsigned bitsPerPixel = GetBitsPerPixel();

    width *= bitsPerPixel / 8;

    switch (bitsPerPixel) {
    case 8:
    case 32:
        // TODO - use fill color.
        for (unsigned y = 0; y < height; y++) {
            BYTE* bits = ScanLine(y);
            memset(bits, 0, width);
        }
        break;
    default:
         // TODO - handle all image types
        std::cerr << "Unsupported FILL pixel size " << bitsPerPixel << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
FPalette& FImage::getPalette(FPalette& palette) const {
    unsigned colors = GetColorsUsed();
    const RGBQUAD* palettePtr = GetPalette();

    palette.clear();
    palette.reserve(colors);

    palette.hasTransparency = IsTransparent();
    if (palette.hasTransparency) {
        const BYTE* transparentPtr = GetTransparencyTable();
        for (unsigned clrIdx = 0; clrIdx < colors; clrIdx++) {
            palette.push_back(FColor(palettePtr[clrIdx], transparentPtr[clrIdx]));
        }
    } else {
        for (unsigned clrIdx = 0; clrIdx < colors; clrIdx++) {
            palette.push_back(FColor(palettePtr[clrIdx], 0xff));
        }
    }
    return palette;
}

//-------------------------------------------------------------------------------------------------
unsigned FImage::setPalette(const FPalette& palette) {
    unsigned colors = GetColorsUsed();
    RGBQUAD* palettePtr = Palette();
    if (palettePtr != NULL && colors > 0 && palette.size() > 0) {
        BYTE transparency[256];
        memset(transparency, 0xff, sizeof(transparency));
        memset(palettePtr, 0x00, colors * sizeof(RGBQUAD));
        unsigned i = 0;
        for (; i < colors && i < palette.size(); i++) {
            palettePtr[i] = palette[i];
            transparency[i] = palette[i].rgbReserved;
        }

        // transparency[0] = 0x00;
        SetTransparencyTable(transparency, i);
        return i;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Adjust alpha channel on 32bit image, percent (0..1)
void FImage::AdjustAlphaP32(float percent, unsigned alphaMin) {
    unsigned scale = (unsigned)(256 * percent);
    unsigned width  = GetWidth();
    unsigned height = GetHeight();
    for (unsigned y = 0; y < height; y++) {
        RGBQUAD* argbPtr = (RGBQUAD*)ScanLine(y);
        for (unsigned x = 0; x < width; x++) {
            RGBQUAD& argb = *argbPtr++;
            argb.rgbReserved =  (unsigned)(argb.rgbReserved * scale / 256);
            // if (argb.rgbReserved > alphaMin) {
            //    argb.rgbReserved = std::max((unsigned)(argb.rgbReserved * scale / 256), alphaMin);
            // }
        }
    }
}

//-------------------------------------------------------------------------------------------------
void FImage::MinAlphaP32(BYTE alpha) {
    unsigned width  = GetWidth();
    unsigned height = GetHeight();
    for (unsigned y = 0; y < height; y++) {
        RGBQUAD* argbPtr = (RGBQUAD*)ScanLine(y);
        for (unsigned x = 0; x < width; x++) {
            RGBQUAD& argb = *argbPtr++;
            argb.rgbReserved = std::min(argb.rgbReserved, alpha);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void FImage::MinAlphaI8(BYTE alpha) {
    FPalette palette;
    getPalette(palette);
    for (unsigned idx = 0; idx < palette.size(); idx++) {
        FColor& color = palette[idx];
        color.rgbReserved = std::min(color.rgbReserved, alpha);
    }
}

//-------------------------------------------------------------------------------------------------
void FImage::DrawRectangleI8(
    const FBrush& brush,
    unsigned x1, unsigned y1, unsigned x2, unsigned y2) {
    BYTE pixel = brush.fillIndex;

    if (GetBitsPerPixel() != 8) {
        std::cerr << "DrawRectangleI8 ignored, image must by 8bit per pixel\n";
    }
    
    for (unsigned y = y1; y < y2; y++) {
        BYTE* linePtr = ScanLine(y);
        linePtr += x1;
        unsigned width = x2 - x1+1;
        while (width-- > 0)
            *linePtr++ = brush.fillIndex;
    }

    if (brush.lineWidth > 0) {
        pixel = brush.lineIndex;
        for (unsigned x = x1; x < x2; x++) {
            for (unsigned w = 0; w < brush.lineWidth; w++) {
                SetPixelIndex(x, y1 + w, pixel);
                SetPixelIndex(x, y2 - w, pixel);
            }
        }
        for (unsigned y = y1; y < y2; y++) {
            for (unsigned w = 0; w < brush.lineWidth; w++) {
                SetPixelIndex(x1 + w, y, pixel);
                SetPixelIndex(x2 - w, y, pixel);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
void FImage::DrawRectangleP32(
    const FBrush& brush,
    unsigned x1, unsigned y1, unsigned x2, unsigned y2) {
    
    for (unsigned y = y1; y < y2; y++) {
        FColor* linePtr = (FColor*)ScanLine(y);
        linePtr += x1;

        unsigned width = x2 - x1+1;
        while (width-- > 0)
            *linePtr++ = brush.fillColor;
    }

    if (brush.lineWidth > 0) {
        for (unsigned x = x1; x < x2; x++) {
            for (unsigned w = 0; w < brush.lineWidth; w++) {
                SetPixelColor(x, y1 + w, brush.lineColor);
                SetPixelColor(x, y2 - w, brush.lineColor);
            }
        }
        for (unsigned y = y1; y < y2; y++) {
            for (unsigned w = 0; w < brush.lineWidth; w++) {
                SetPixelColor(x1 + w, y, brush.lineColor);
                SetPixelColor(x2 - w, y, brush.lineColor);
            }
        }
    }
}
