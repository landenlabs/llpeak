//-------------------------------------------------------------------------------------------------
//  File: FShade.cpp
//  Desc: Image Shade functions using FreeImage
//
//  FShade created by Dennis Lang on 12/21/21.
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

#include "FShade.hpp"
#include "ImageAux.hpp"
#include "FBlur.hpp"

//-------------------------------------------------------------------------------------------------
// Simple 2 dimensional X and Y one pixel slope shading.
bool FShadeXY1::shadeI8_P32(const FPalette& inPalette, const FImage& inI8, FImage& outP32, ImageCfg& cfg, ImageAux& aux) {
    
    const float M = 4.0f;       // TODO - get from ImageCfg
    unsigned width = inI8.GetWidth();
    unsigned height = inI8.GetHeight();
    // unsigned colors = inI8.GetColorsUsed();
    
    const BYTE* prevRowP = inI8.ReadScanLine(0);
    
    for (unsigned y = 0; y < height; y++) {
        const BYTE* inP = inI8.ReadScanLine(y);
        FColor* out = (FColor*)outP32.ScanLine( y);
        int prevX = aux.shadeMap.to[inP[0]];
        
        for (unsigned x = 0; x < width; x++) {
            int prevY = aux.shadeMap.to[prevRowP[x]];
            const FColor& inColor = inPalette[inP[x]];
            int px = aux.shadeMap.to[inP[x]];
            
            float slopeX = (prevX - px)/255.0f;
            float slopeY = (prevY - px)/255.0f;
            float slope = 1.0f + (slopeX + slopeY) * M;
            
            if (slope > 1.0f) {
                // out[x] = FColor::brighten(slope, FColor::GREEN, inColor.rgbReserved);
                out[x] = FColor::brighten(slope, inColor, inColor.rgbReserved);
            } else if (slope < 1.0f) {
                // out[x] = FColor::darken(slope, FColor::RED, inColor.rgbReserved);
                out[x] = FColor::darken(slope, inColor, inColor.rgbReserved);
            } else {
                out[x] = inColor;
            }
            prevX = px;
        }
        prevRowP = inP;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
bool FShadeXY1::shadeP32(const FImage& inP32, FImage& outP32, ImageCfg& cfg, ImageAux& aux) {
    
    if (inP32.imgPtr != outP32.imgPtr) {
        std::cerr << "ShadeP32 - output image should be the same as input \n";
        return false;
    }
    
    const float M = 4.0f;            // TODO - get from ImageCfg
    unsigned width = inP32.GetWidth();
    unsigned height = inP32.GetHeight();
    
    std::unique_ptr<FColor> prevRow(new FColor[width]);
    FColor* prevRowP = prevRow.get();
    memcpy(prevRowP, inP32.ReadScanLine(0), width * sizeof(FColor));
    
    for (unsigned y = 1; y < height; y++) {
        const FColor* inp = (const FColor*)inP32.ReadScanLine(y);
              FColor* out = (FColor*)outP32.ScanLine( y);

        int prevX = (unsigned)inp[0].rgbRed;
        for (unsigned x = 0; x < width; x++) {
            FColor inColor = inp[x];
            int prevY = (unsigned)prevRowP[x].rgbRed;
            prevRowP[x] = inColor;
            int px = inp[x].rgbRed;
            
            float slopeX = (prevX - px)/255.0f;
            float slopeY = (prevY - px)/255.0f;
            float slope = 1.0f + (slopeX + slopeY) * M;
            
            if (slope > 1.0f) {
                // out[x] = FColor::brighten(slope, FColor::GREEN, inColor.rgbReserved);
                out[x] = FColor::brighten(slope, inColor, inColor.rgbReserved);
            } else if (slope < 1.0f) {
                // out[x] = FColor::darken(slope, FColor::RED, inColor.rgbReserved);
                out[x] = FColor::darken(slope, inColor, inColor.rgbReserved);
            } else {
                // out[x] = inColor;
            }
           
            prevX = px;
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// Two dimension X and Y shading with some latency dragging slope across multiple cells.
bool FShadeXY2::shadeI8_P32(const FPalette& inPalette, const FImage& inI8, FImage& outP32, ImageCfg& cfg, ImageAux& aux) {
    
    const float M = 8.0f;           // TODO - get from ImageCfg
    unsigned width = inI8.GetWidth();
    unsigned height = inI8.GetHeight();
    
    const BYTE* prevRowP = inI8.ReadScanLine(0);
    std::unique_ptr<float> prevYscale(new float[width]);
    memset(prevYscale.get(), 0, width*sizeof(float));
    float* prevYscaleP = prevYscale.get();
    
    for (unsigned y = 0; y < height; y++) {
        const BYTE* inP = inI8.ReadScanLine(y);
        FColor* out = (FColor*)outP32.ScanLine( y);
        float slopeX = 0;
        float slopeY = 0;
        float prevXscale = 0.0f;
        int prevX = aux.shadeMap.to[inP[0]];
        
        for (unsigned x = 0; x < width; x++) {
            int prevY = (int)(unsigned)aux.shadeMap.to[prevRowP[x]];
            const FColor& inColor = inPalette[inP[x]];
            int px = (int)(unsigned)aux.shadeMap.to[inP[x]];
            
            slopeX = (prevX - px)/255.0f;
            slopeY = (prevY - px)/255.0f;
            
            float xPrev = prevXscale;
            float yPrev = prevYscaleP[x];
            
            slopeX = (slopeX + xPrev) /2.0f;
            slopeY = (slopeY + yPrev) /2.0f;
            float slope = (slopeX + slopeY);
          
            float scale = std::min(2.0f, std::max(0.0f, 1.0f + slope * M));
            out[x] = FColor::scale(scale, inColor, inColor.rgbReserved);
            
            // Update historical slope values. 
            prevXscale     = (xPrev/2 + slope)/2;
            prevYscaleP[x] = (yPrev/2 + slope)/2;
            
            prevX = px;
        }
        prevRowP = inP;
    }
    
    return true;
}

//-------------------------------------------------------------------------------------------------
bool FShadeXY2::shadeP32(const FImage& inP32, FImage& outP32, ImageCfg& cfg, ImageAux& aux) {
    
    if (inP32.imgPtr != outP32.imgPtr) {
        std::cerr << "ShadeP32 - output image must be the same as input \n";
        return false;
    }
    
    const float M = 8.0f;       // TODO - get from ImageCfg
    unsigned width = inP32.GetWidth();
    unsigned height = inP32.GetHeight();
    
    std::unique_ptr<FColor> prevRow(new FColor[width]);
    FColor* prevRowP = prevRow.get();
    memcpy(prevRowP, inP32.ReadScanLine(0), width * sizeof(FColor));

    std::unique_ptr<float> prevYscale(new float[width]);
    float* prevYscaleP = prevYscale.get();
    memset(prevYscaleP, 0, width*sizeof(float));
    
    for (unsigned y = 0; y < height; y++) {
        const FColor* inp = (const FColor*)inP32.ReadScanLine(y);
        FColor* out = (FColor*)outP32.ScanLine( y);
        float prevXscale = 0.0f;
        int prevX = inp[0].rgbRed;
        for (unsigned x = 0; x < width; x++) {
            FColor inColor = inp[x];
            int prevY = (unsigned)prevRowP[x].rgbRed;
            prevRowP[x] = inColor;
            int px = inp[x].rgbRed;
            
            float slopeX = (prevX - px)/255.0f;
            float slopeY = (prevY - px)/255.0f;
            
            float xPrev = prevXscale;
            float yPrev = prevYscaleP[x];
            
            slopeX = (slopeX + xPrev) /2.0f;
            slopeY = (slopeY + yPrev) /2.0f;
            float slope = (slopeX + slopeY);

            float scale = std::min(2.0f, std::max(0.0f, 1.0f + slope * M));
            
            if (slope > 0.0f) {
                // out[x] = FColor::brighten(slope, FColor::GREEN, inColor.rgbReserved);
                out[x] = FColor::brighten(scale, inColor, inColor.rgbReserved);
            } else if (slope < 0.0f) {
                // out[x] = FColor::brighten(slope, FColor::RED, inColor.rgbReserved);
                out[x] = FColor::darken(scale, inColor, inColor.rgbReserved);
            } else {
                // out[x] = inColor;
            }
            
            // Update historical slope values.
            prevXscale     = (xPrev/2 + slope)/2;   // (xPrev/6 + slope*3/6);
            prevYscaleP[x] = (yPrev/2 + slope)/2;   // (yPrev/6 + slope*3/6);
            
            prevX = px;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FColor brighten(float rate, FColor rgb, BYTE alpha = 0xff) {
    rate -= 1.0f;
    // rate = sin(rate);
    unsigned amt = 255 * rate;
    return FColor(
          std::min(255u, rgb.rgbRed + amt),
          std::min(255u, rgb.rgbGreen + amt),
          std::min(255u, rgb.rgbBlue + amt),
          alpha);
}

// Simple 2 dimensional X and Y one pixel slope shading using smoothed input.
bool FShadeXY3::shadeI8_P32(const FPalette& inPalette, const FImage& inI8, FImage& outP32, ImageCfg& cfg, ImageAux& aux) {
    
    // ---- Blur
#if 0
    const float M = 10.0f;          // TODO - get from ImageCfg
    unsigned radius = 2;            // TODO - get from ImageCfg
    const FPalette& tmpPalette = cfg.getOutPalette();
    FPalette outPalette;
    tmpPalette.spread(outPalette, (unsigned)tmpPalette.size(), 256);
#else
    const float M = 100.0f;         // TODO - get from ImageCfg
    unsigned radius = 2;            // TODO - get from ImageCfg
    const FPalette& outPalette = cfg.getOutPalette();
#endif
    PalMapping mapping = FPalette::getMapping(inPalette, outPalette);
   
    unsigned width = inI8.GetWidth();
    unsigned height = inI8.GetHeight();
    unique_ptr<float> inGrid(new float[width*height]);
    unique_ptr<float> outGrid(new float[width*height]);
    
    FBlur::toFloat(mapping, inI8, width, height, inGrid.get());
    FBlur::hBlur(radius, width, height, inGrid.get(), outGrid.get());
    outGrid.swap(inGrid);
    FBlur::vBlur(radius, width, height, inGrid.get(), outGrid.get());
    
    float* grid = outGrid.get();
    unsigned nColors = (unsigned)outPalette.size();
    
    // ---- Shade
 
    const float* prevRowP = grid;
    
    for (unsigned y = 0; y < height; y++) {
        const float* inP = grid + y * width;
        FColor* out = (FColor*)outP32.ScanLine( y);
        float prevX = inP[0];
        
        for (unsigned x = 0; x < width; x++) {
            float prevY = prevRowP[x];
            float px = inP[x];
            
            float slopeX = (prevX - px)/255.0f;
            float slopeY = (prevY - px)/255.0f;
            float slope = 1.0f + (slopeX + slopeY) * M;
            
            FColor inColor;
            unsigned lowPx = px;
            float percent = px - lowPx;
            if (lowPx+1 < nColors || percent > 0.1) {
                inColor = FColor::percent(percent, outPalette[lowPx], outPalette[lowPx+1]);
            } else {
                inColor = outPalette[lowPx];
            }
            
            if (slope > 1.0f) {
                slope = std::min(2.0f, slope);
                // out[x] = FColor::brighten(slope, FColor::GREEN, inColor.rgbReserved);
                out[x] = brighten(slope, inColor, inColor.rgbReserved);
            } else if (slope < 1.0f) {
                slope = std::max(0.0f, slope);
                // out[x] = FColor::darken(slope, FColor::RED, inColor.rgbReserved);
                out[x] = FColor::darken(slope, inColor, inColor.rgbReserved);
            } else {
                out[x] = inColor;
            }
            prevX = px;
        }
        prevRowP = inP;
    }
    
    return true;
}



