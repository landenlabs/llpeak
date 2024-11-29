//-------------------------------------------------------------------------------------------------
//  File: FBlur.cpp
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

 

// Project files
#include "FBlur.hpp"

//-------------------------------------------------------------------------------------------------
void FBlur::toFloat( const PalMapping& mapping, const FImage& inI8, unsigned width, unsigned height, float* grid) {
    
    for (unsigned y = 0; y < height; y++) {
        const BYTE* inPx = inI8.ReadScanLine(y);
        for (unsigned x = 0; x < width; x++) {
            unsigned px = mapping.to[inPx[x]];
            if (px < 256) {
                *grid++ = px;
            } else {
                std::cerr << "Bad pixel at " << x << ", " << y << std::endl;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
void FBlur::hBlur(unsigned radius, unsigned xDim, unsigned yDim, const float* inGrid, float* outGrid) {
    unsigned numSamples = radius * 2 + 1;
    float invN = 1.0f / numSamples;
    
    for (unsigned y = 0; y < yDim; y++, inGrid+= xDim, outGrid+= xDim) {
        const float* inPtr = inGrid;
        float* outPtr = outGrid;
 
        const float* head = inPtr;
        const float* tail = head;
        float* pOut = outPtr;

        // Split row into three parts:
        // Radius = 7
        //
        //     Left Margin       Middle        Right Margin
        // |------>R=======------------------======>R-------|
        // |123456787654321..................123456787654321|
        //

        const float* leftMargin = head + radius;
        const float* leftMiddle = head + numSamples;
        const float* rightMiddle = head + xDim;
        const float* rightMargin = head + xDim - radius - 1;

        float sum = 0;

        // Accumulate left edge in margin
        while (head < leftMargin) {
            sum += (*head++);
        }

        // Compute left half margin output
        int cntSum = radius;
        while (head < leftMiddle) {
            sum += *head++;
            *pOut++ = sum / ++cntSum;
        }

        // Compute output in middle where full samples available.
        while (head < rightMiddle) {
            sum += (*head++);
            sum -= (*tail++);
            *pOut++ = (sum * invN);
        }

        // Compute right half margin output
        while (tail < rightMargin) {
            sum -= (*tail++);
            *pOut++ = (sum / --cntSum);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void FBlur::vBlur(unsigned radius, unsigned xDim, unsigned yDim, const float* inGrid, float* outGrid) {
    unsigned numSamples = radius * 2 + 1;
    float invN = 1.0f / numSamples;
    
    for (unsigned x = 0; x < xDim; x++) {
        const float* head = inGrid + x;
        const float* tail = head;
        float* pOut = outGrid + x;

        const float* topMargin = head + (xDim * radius);
        const float* begMiddle = head + (xDim * numSamples);
        const float* endMiddle = head + (xDim * yDim);
        const float* botMargin = head + (xDim * (yDim - radius -1));

        float sum = 0;

         /* Debug copy source to destination
         while (head < endMiddle) {
             sum = (*head);
             sum[1] = 0;
             *pOut = (sum);
             head += xDim;
             pOut += xDim;
         }
         head = inPtr + columnIndex;
         pOut = outPtr + columnIndex;
         */

         if (xDim * yDim > 0) {
             // Accumulate top margin
             while (head < topMargin) {
                 sum += (*head);
                 head += xDim;
             }

             // Output top half margin
             int cntSum = radius;
             while (head < begMiddle) {
                 sum += (*head);
                 *pOut = (sum / ++cntSum);
                 head += xDim;
                 pOut += xDim;
             }

             // Output middle with full samples
             while (head < endMiddle) {
                 sum += (*head);
                 sum -= (*tail);
                 *pOut = (sum * invN);
                 head += xDim;
                 tail += xDim;
                 pOut += xDim;
             }

             // Output bottom half margin.
             while (tail < botMargin) {
                 sum -= (*tail);
                 *pOut = (sum / --cntSum);
                 tail += xDim;
                 pOut += xDim;
             }
         }
    }
}

//-------------------------------------------------------------------------------------------------
void FBlur::toPixel32(const float* inGrid, const FPalette& inPalette, unsigned width, unsigned height, FImage& outP32) {
    const unsigned nColors = (unsigned)inPalette.size();
    for (unsigned y = 0; y < height; y++) {
        FColor* outRow = (FColor*)outP32.ScanLine(y);
        for (unsigned x = 0; x < width; x++) {
            float fp = *inGrid++;
            unsigned lowPx = fp;
            float percent = fp - lowPx;
            if (lowPx+1 < nColors || percent > 0.1) {
                *outRow++ = FColor::percent(percent, inPalette[lowPx], inPalette[lowPx+1]);
            } else {
                *outRow++ = inPalette[lowPx];
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
bool FBlur::blurI8(
        const PalMapping& mapping,
        const FPalette& outPalette,
        const FImage& inI8, FImage& outP32,
        ImageCfg& cfg,
        ImageAux& aux,
        unsigned radius) {
    
    unsigned width = inI8.GetWidth();
    unsigned height = inI8.GetHeight();
    unique_ptr<float> inGrid(new float[width*height]);
    unique_ptr<float> outGrid(new float[width*height]);
    
    toFloat(mapping, inI8, width, height, inGrid.get());
    hBlur(radius, width, height, inGrid.get(), outGrid.get());
    outGrid.swap(inGrid);
    vBlur(radius, width, height, inGrid.get(), outGrid.get());
    toPixel32(outGrid.get(), outPalette, width, height, outP32);
    
    return true;
}

