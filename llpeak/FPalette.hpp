//-------------------------------------------------------------------------------------------------
//  File: FPalette.hpp
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

#pragma once

// Project files
#include "FColor.hpp"
#include "PalMapping.hpp"

#include <vector>
#include <string>



//-------------------------------------------------------------------------------------------------
class FPalette : public std::vector<FColor> {
    
public:
    static const FPalette EMPTY;
    static const unsigned NO_MATCH = -1;
    static const unsigned NO_CLOSEST = 256;
    
    bool hasTransparency;
    std::vector<std::string> names;

    FPalette() : hasTransparency(false) {}

    FPalette(const FColor* colorPtr, unsigned nColors, bool _hasTransparency = false)  
            : hasTransparency(_hasTransparency) {
        reserve(nColors);
        for (unsigned idx =0; idx < nColors; idx++) {
            push_back(colorPtr[idx]);
        }
    }
    
    typedef std::vector<FColor> Vec;
    using Vec::push_back;
    void push_back(const FColor& color, const std::string& name) {
        push_back(color);
        names.push_back(name);
    }
    
    FPalette& spread(FPalette& outPalette, unsigned inSize=0, unsigned outSize=256) const;
    
    RGBQUAD* quads() const {
        return (RGBQUAD*)data();  // Cast away const
    }
  
    unsigned findClosest(const FColor& color4, float* distPtr=nullptr, float maxDst=256*256, unsigned failIdx=NO_CLOSEST) const;
    unsigned findAlpha(const FColor& color4, unsigned failIdx=256) const;

    unsigned findColor(FColor color4, unsigned defIdx=NO_MATCH) const {
        for (unsigned idx = 0; idx < size(); idx++) {
            const FColor& ours = at(idx);
            if (color4 == ours)
                return idx;
        }
        return defIdx;
    }
    
    FPalette& remove(FColor color) {
        unsigned NO_MATCH = -1;
        unsigned idx = findColor(color, NO_MATCH);
        if (idx != NO_MATCH) {
            erase(begin() + idx);
        }
        return *this;
    }
    
    static PalMapping getMapping(const FPalette& srcPalette, const FPalette& dstPalette);
    unsigned merge(const FPalette& inPal, unsigned maxDst=256, unsigned maxColors=256);
    
    
    // See "The incredibly challenging task of sorting colours"
    // https://www.alanzucconi.com/2015/09/30/colour-sorting/
    // Compares two RGB colors, sort colors first by hue, gray last.
    static  bool byHSV(FColor rgb1, FColor rgb2) {
        if (rgb1.rgbRed == rgb2.rgbRed && rgb1.rgbGreen == rgb2.rgbGreen && rgb1.rgbBlue == rgb2.rgbBlue) {
            return rgb1.rgbReserved < rgb2.rgbReserved;
        }
        
        bool gray1 = rgb1.isGray();
        bool gray2 = rgb2.isGray();
        if (gray1 != gray2) {
            return gray1 < gray2;
        } else if (gray1) {
            return rgb1.luminosity() > rgb2.luminosity();
        }
        /*
        BYTE max1, max2;
        if (rgb1.maxClr(max1) == rgb2.maxClr(max2)) {
            return max1 > max2;
        }
         */
        return rgb1.toHSV() > rgb2.toHSV();
        /*
        HSV hsv1 = rgb1.toHSV();
        HSV hsv2 = rgb2.toHSV();
        if (fabs(hsv1.hue - hsv2.hue) < 0.075) {
            return rgb1.luminosity() > rgb2.luminosity();
        }
        return hsv1 > hsv2;
        */
    }
};
