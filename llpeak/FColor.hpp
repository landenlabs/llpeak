//-------------------------------------------------------------------------------------------------
//  File: FColor.hpp
//  Desc: FreeImage C++ wrapper on image pixel (color).
//
//  Wrapper created by Dennis Lang on 12/21/21.
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
//  brew install freeimage
#include "FreeImage.h"

#include <string>
#include <math.h>


// Forward declaration.
class FColor;


namespace FClr {
class HSV {
public:
    float hue;
    float sat;
    float value;
    
    HSV(float h, float s, float v) : hue(h), sat(s), value(v)
    { }
    FColor toRGB() const;
    
    float distance(const HSV& other) const {
        float dHue = hue - other.hue;
        float dSat = sat - other.sat;
        float dVal = value - other.value;
        return dHue*dHue + dSat*dSat + dVal*dVal;
    }
    
    bool operator>(const HSV& other) {
        /*
        if (fabs(hue - other.hue) > 0.075) {
            return hue > other.hue;
        } else if (fabs(sat - other.sat) > 0.1) {
            return sat > other.sat;
        }
        return value > other.value;
         */
     
        if (hue != other.hue) {
            return hue > other.hue;
        }
        if (sat != other.sat) {
            return sat > other.sat;
        }
        return value > other.value;
     
    }
};

//-------------------------------------------------------------------------------------------------
// https://github.com/ThunderStruct/Color-Utilities
class LAB {
public:
    float l, a, b;
    
    LAB(float _l, float _a, float _b) : l(_l), a(_a), b(_b)
    { }
};

//-------------------------------------------------------------------------------------------------
// https://github.com/ThunderStruct/Color-Utilities
class XYZ {
public:
    float x, y, z;
    XYZ(float _x, float _y, float _z) : x(_z), y(_y), z(_z)
    { }
    
    LAB toLAB() const;
};
} // end Namespace FClr


//-------------------------------------------------------------------------------------------------
class FColor : public RGBQUAD {
public:
    static const FColor RED;
    static const FColor GREEN;
    static const FColor BLUE;
    static const FColor WHITE;
    static const FColor GRAY;
    static const FColor BLACK;
    static const FColor TRANSPARENT;
    
    inline
    FColor() {
        rgbRed = rgbGreen = rgbBlue = rgbReserved = 0;
    }
    
    inline
    FColor(BYTE red, BYTE green, BYTE blue, BYTE alpha = 0xff) {
        rgbRed = red;
        rgbGreen = green;
        rgbBlue = blue;
        rgbReserved = alpha;
    }
    
    inline
    FColor(const RGBQUAD& rgb, BYTE alpha) : RGBQUAD(rgb) {
         rgbReserved = alpha;
    }
    
    FColor(const std::string&);
    static bool parse(const std::string& str, FColor& color, std::string& msg);
    
    inline
    FColor(const FColor& other) {
        rgbRed = other.rgbRed;
        rgbGreen = other.rgbGreen;
        rgbBlue = other.rgbBlue;
        rgbReserved = other.rgbReserved;
    }
    
    inline
    FColor& operator=(const FColor& other) {
        *((DWORD*) this) = *((DWORD*) &other);
        return *this;
    }
    
    inline
    bool operator==(const FColor& other) const {
        return *((DWORD*) this) == *((DWORD*) &other);
    }
    
    inline
    bool operator!=(const FColor& other) const {
        return *((DWORD*) this) != *((DWORD*) &other);
    }
    
    inline
    RGBQUAD* quad() const {
        return (RGBQUAD*)this;  // Cast away const
    }

    void blendOver(RGBQUAD& botColor) const;

    FClr::HSV toHSV() const;
    FClr::XYZ toXYZ() const;
    
    inline
    double luminosity() const {
        return ( .241 * rgbRed + .691 * rgbGreen + .068 * rgbBlue );
    }
    
    inline
    unsigned maxClr(BYTE& maxVal) const {
        if (rgbRed == rgbGreen && rgbRed == rgbBlue) {
            maxVal = rgbRed;
            return 3;
        }
        
        maxVal = std::max(std::max(rgbRed, rgbGreen), rgbBlue);
        if (rgbRed == maxVal) return 0;
        if (rgbGreen == maxVal) return 1;
        return 2;
    }
    
    inline
    unsigned maxClr() const {
        return std::max(std::max(rgbRed, rgbGreen), rgbBlue);
    }
    
    inline
    bool isGray(BYTE delta = 8) const {
        BYTE minClr = std::min(std::min(rgbRed, rgbGreen), rgbBlue);
        BYTE maxClr = std::max(std::max(rgbRed, rgbGreen), rgbBlue);
        return  (maxClr - minClr < maxClr/8);
    }
    
    inline
    static
    BYTE clamp(unsigned cBig) {
        return (cBig > 0xff) ? 0xff : (BYTE)cBig;
    }
    
    inline
    static
    float clampMax(float rate, unsigned red, unsigned green, unsigned blue) {
        unsigned maxRGB = std::max(red, std::max(green, blue));
        float maxRate = (maxRGB > 0) ? 255.0f/maxRGB : 1.0f;
        return std::min(rate, maxRate);
    }

    inline
    static // rate (percent) 1.0 = no change, < 1.0 darken
    BYTE darken(float rate, BYTE cByte) {
        unsigned cBig (cByte * rate);
        return cBig;  // return clamp(cBig);
    }
    
    inline
    static // rate (percent) 1.0 = no change, < 1.0 darken
    FColor darken(float rate, BYTE red, BYTE green, BYTE blue, BYTE alpha = 0xff) {
        return FColor(
            darken(rate, red),
            darken(rate, green),
            darken(rate, blue),
            alpha);
    }
    
    inline
    static // rate (percent) 1.0 = no change, < 1.0 darken
    BYTE darken(float rate, unsigned offset, BYTE cByte) {
        unsigned cBig (cByte * rate);
        return (cBig > offset) ? cBig-offset : 0;
    }
    
    static const unsigned OFFSET_RATE = 32;   
    
    inline
    static // rate (percent) 1.0 = no change, < 1.0 darken
    FColor darken(float rate, const FColor& rgb, BYTE alpha = 0xff) {
        unsigned maxRGB = std::max(rgb.rgbRed, std::max(rgb.rgbGreen, rgb.rgbBlue));
        unsigned minRGB = std::min(rgb.rgbRed, std::min(rgb.rgbGreen, rgb.rgbBlue));
        unsigned offset = (maxRGB - minRGB)/OFFSET_RATE+1;
        
        return FColor(
            darken(rate, offset, rgb.rgbRed),
            darken(rate, offset, rgb.rgbGreen),
            darken(rate, offset, rgb.rgbBlue),
            alpha);
    }
    
    inline
    static // rate (percent) 1.0 = no change, > 1.0 brighten
    BYTE brighten(float rate, unsigned offset, BYTE cByte) {
        unsigned cBig(cByte * rate + offset);
        return clamp(cBig);
    }
    
    inline
    static // rate (percent) 1.0 = no change
    FColor brighten(float rate, const FColor& rgb, BYTE alpha = 0xff) {
        unsigned maxRGB = std::max(rgb.rgbRed, std::max(rgb.rgbGreen, rgb.rgbBlue));
        float maxRate = (maxRGB > 0) ? 255.0f/maxRGB : 1.0f;
        rate = std::min(rate, maxRate);
        unsigned minRGB = std::min(rgb.rgbRed, std::min(rgb.rgbGreen, rgb.rgbBlue));
        unsigned offset = (maxRGB - minRGB)/OFFSET_RATE +1;
        return FColor(
              brighten(rate, offset, rgb.rgbRed),
              brighten(rate, offset, rgb.rgbGreen),
              brighten(rate, offset, rgb.rgbBlue),
              alpha);
    }
    
    inline
    static // rate (percent) 1.0 = no change, < 1.0 darken, > 1.0 brighten
    FColor scale(float rate, const FColor& rgb, BYTE alpha = 0xff) {
        if (rate == 1.0f)
            return rgb;
        else if (rate < 1.0f)
            return darken(rate, rgb, alpha);
        else
            return brighten(rate, rgb, alpha);
    }

    inline
    static
    FColor makeColor(BYTE red, BYTE green, BYTE blue, BYTE alpha = 0xff) {
        return FColor(red, green, blue, alpha);
    }

    inline
    size_t distanceRGB(const FColor& other) const {
        int dRed = (int)rgbRed - (int)other.rgbRed;
        int dGreen = (int)rgbGreen - (int)other.rgbGreen;
        int dBlue = (int)rgbBlue - (int)other.rgbBlue;
        return dRed*dRed + dGreen*dGreen + dBlue*dBlue;
    }
    
    inline
    float distanceHSV(const FColor& other) const {
        return toHSV().distance(other.toHSV());
    }
    
    // http://colormine.org/delta-e-calculator
    // https://github.com/ThunderStruct/Color-Utilities
    // CIE76 - Delta E
    inline
    float distanceDeltaE(const FColor& other) const {
        FClr::XYZ xyzC1 = toXYZ(), xyzC2 = other.toXYZ();
        FClr::LAB labC1 = xyzC1.toLAB(), labC2 = xyzC2.toLAB();
        float deltaE;

        // Euclidian Distance between two points in 3D matrices
        deltaE = sqrtf( powf(labC1.l - labC2.l, 2) + powf(labC1.a - labC2.a, 2) + powf(labC1.b - labC2.b, 2) );
        return deltaE;
    }
    
    inline
    static
    BYTE percent(float per, BYTE b1, BYTE b2) {
        return (unsigned)(0.5f + ((int)b2 - (int)b1) * per + (int)b1);
    }
    
    inline
    static
    FColor percent(float per, const FColor& c1, const FColor& c2) {
        return FColor(
              percent(per, c1.rgbRed,   c2.rgbRed),
              percent(per, c1.rgbGreen, c2.rgbGreen),
              percent(per, c1.rgbBlue,  c2.rgbBlue),
              percent(per, c1.rgbReserved, c2.rgbReserved));
    }
};
