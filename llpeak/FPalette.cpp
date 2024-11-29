//-------------------------------------------------------------------------------------------------
//  File: FPalette.cpp
//  Desc: FreeImage C++ wrapper on 8bit image palettes. 
//
//  FPalette created by Dennis Lang on 12/21/21.
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
#include "FPalette.hpp"
#include "FPrint.hpp"
#include "MapVector.hpp"

#include <iostream>
#include <limits>
#include <map>
#include <vector>

//-------------------------------------------------------------------------------------------------
unsigned FPalette::findClosest(const FColor& color4, float* distPtr, float maxDst,  unsigned failIdx) const {
    const FClr::HSV hsv = color4.toHSV();
    // size_t minDist =  std::numeric_limits<size_t>::max();
    float minDist = std::numeric_limits<float>::max();
    unsigned minIdx = failIdx;
    for (unsigned idx = 0; idx < size(); idx++) {
        const FColor& color = at(idx);
        // size_t dist = color4.distanceRGB(color);
        // float dist = color4.distanceDeltaE(color);
        float dist = hsv.distance(color.toHSV());
        
        if (dist < minDist) {
            if (color4.rgbReserved == color.rgbReserved  /* && dist < maxDst */  ) {
                minDist = dist;
                minIdx = idx;
            } else if (minIdx != failIdx ) {
                // std::cerr << "ignoring color distance " << dist << " " << FPrint::toString(color) << std::endl;
            }
        }
    }

    if (distPtr != nullptr) {
        *distPtr = minDist;
    }
    return minIdx;
}

//-------------------------------------------------------------------------------------------------
unsigned FPalette::findAlpha(const FColor& color4, unsigned failIdx) const {
    if (color4.rgbReserved != 0xff && hasTransparency) {
        for (unsigned idx = 0; idx < size(); idx++) {
            const FColor& color = at(idx);
            if (color4.rgbReserved == color.rgbReserved) {
                return idx;
            }
        }
    }
    return failIdx;
}

//-------------------------------------------------------------------------------------------------
PalMapping  FPalette::getMapping(const FPalette& srcPalette, const FPalette& dstPalette)  {
    PalMapping mapping;
    // const FPalette& dstPalette = getOutPalette();
    for (unsigned srcIdx = 0; srcIdx < srcPalette.size(); srcIdx++) {
        const FColor& srcColor = srcPalette[srcIdx];
        float matchDist = std::numeric_limits<float>::max();
        unsigned bestIdx = dstPalette.findClosest(srcColor, &matchDist);
        if (bestIdx == NO_CLOSEST && srcColor.rgbReserved != 0xff) {
            bestIdx = dstPalette.findAlpha(srcColor);
            if (bestIdx != NO_CLOSEST) {
                matchDist = srcColor.toHSV().distance(dstPalette[bestIdx].toHSV());
            }
        }
        mapping.from[srcIdx] = srcIdx;
        mapping.to[srcIdx] = bestIdx;
        mapping.dist[srcIdx] = matchDist;
        mapping.shiftCnt += (srcIdx != bestIdx) ? 1 : 0;
    }
    mapping.isReady = true;
    return mapping;
}

//-------------------------------------------------------------------------------------------------
unsigned  FPalette::merge(const FPalette& otherPal, unsigned maxDst, unsigned maxColors) {
    PalMapping mapping = getMapping(otherPal, *this);
    if (mapping.shiftCnt == 0 || size() >= maxColors) {
        return 0;
    }
    
    std::vector< std::pair <float,unsigned> > distPair;
    for (unsigned idx = 0; idx < otherPal.size(); idx++) {
        distPair.push_back(std::make_pair(mapping.dist[idx], idx));
    }
    std::sort(distPair.begin(), distPair.end());
    unsigned mid = (unsigned)otherPal.size()/2;
    for (unsigned idx = 0; size() < maxColors && idx < otherPal.size()/2; idx++) {
        if (mid + idx < otherPal.size()) {
            push_back(otherPal[mid + idx]);
        }
        if (mid - idx < otherPal.size() && size() < maxColors) {
            push_back(otherPal[mid - idx]);
        }
    }
    // TODO  - merge palettes
    return 1;
}

//-------------------------------------------------------------------------------------------------
static void gradient(FColor color1, FColor color2, unsigned spreadCnt, FPalette& outPalette) {
    unsigned redD = color2.rgbRed - color1.rgbRed;
    unsigned greenD = color2.rgbGreen - color1.rgbGreen;
    unsigned blueD = color2.rgbBlue - color1.rgbBlue;
    unsigned alphaD = color2.rgbReserved - color1.rgbReserved;
    
    for (unsigned idx = 0; idx < spreadCnt; idx++) {
        outPalette.push_back(FColor(
            color1.rgbRed + redD * idx / (spreadCnt-1),
            color1.rgbGreen + greenD * idx / (spreadCnt-1),
            color1.rgbBlue + blueD * idx / (spreadCnt-1),
            color1.rgbReserved + alphaD * idx / (spreadCnt-1)
            ));
    }
}

//-------------------------------------------------------------------------------------------------
FPalette& FPalette::spread(FPalette& outPalette, unsigned inSize, unsigned outSize) const {
    inSize = (inSize == 0) ? (unsigned)size() : inSize;
    outSize = (outSize == 0) ? std::min(255u, inSize * 4) : outSize;
    
    if (outSize <= inSize) {
        if (outSize < inSize) {
            std::cerr << "Warning - color spread ignored reduction\n";
        }
        outPalette = *this;
    } else {
        unsigned spreadCnt = outSize/ inSize;
        FColor inColor1 = at(0);
        for (unsigned inIdx = 1; inIdx < inSize; inIdx++) {
            FColor inColor2 = at(inIdx);
            gradient(inColor1, inColor2, spreadCnt, outPalette);
            inColor1 = inColor2;
        }
    }
    return outPalette;
}
