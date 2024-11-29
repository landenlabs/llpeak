//-------------------------------------------------------------------------------------------------
//  File: FColor.cpp
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

// Project files
#include "FColor.hpp"
#include "Split.hpp"

#include <math.h>

const FColor FColor::RED(255,0,0);
const FColor FColor::GREEN(0,255,0);
const FColor FColor::BLUE(0,0,255);
const FColor FColor::WHITE(255,255,255);
const FColor FColor::BLACK(0,0,0);
const FColor FColor::GRAY(128,128,128);
const FColor FColor::TRANSPARENT(0,0,0,0);

//-------------------------------------------------------------------------------------------------
FColor::FColor(const std::string& str) {
    std::string msg;
    if (!parse(str, *this, msg)) {
        throw msg;
    }
}

//-------------------------------------------------------------------------------------------------
bool FColor::parse(const std::string& str, FColor& color, std::string& msg)  {
    Split parts(str, ",");
    unsigned nParts = (unsigned)parts.size();
    if (nParts == 3 || nParts == 4) {
        unsigned fcolor[4];
        fcolor[3] = 0xff;
        if (str.find(".") != std::string::npos) {
            for (unsigned i = 0; i < nParts; i++) {
                float fnum = ::atof(parts[i].c_str());
                fcolor[i] = (unsigned)std::max(0, std::min(255, (int)(fnum * 255)));
            }
        } else {
            for (unsigned i = 0; i < nParts; i++) {
                int inum = (int)::strtoul(parts[i].c_str(), nullptr, 10);
                fcolor[i] = (unsigned)std::max(0, std::min(255, inum));
            }
        }
        
        color.rgbRed = fcolor[0];
        color.rgbGreen = fcolor[1];
        color.rgbBlue = fcolor[2];
        color.rgbReserved = fcolor[3];
        return true;
    } else {
        msg = str + ", Expect 3 or 4 color decimal or float values, ex: 1,128,255 or 0.1,0.5,1.0";
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
void FColor::blendOver(RGBQUAD& botColor) const {
    if (botColor.rgbReserved == 0) {
        botColor = *this;
    } else if (rgbReserved != 0) {
        unsigned alpha = rgbReserved;
        botColor.rgbRed   = (rgbRed   * alpha + botColor.rgbRed   * (255 - alpha)) / 255;
        botColor.rgbGreen = (rgbGreen * alpha + botColor.rgbGreen * (255 - alpha)) / 255;
        botColor.rgbBlue  = (rgbBlue  * alpha + botColor.rgbBlue  * (255 - alpha)) / 255;
        botColor.rgbReserved  = (rgbReserved  * alpha + botColor.rgbReserved  * (255 - alpha)) / 255;
        // botColor.rgbReserved = 0xff;    // TODO - is this right
    } 
}

//-------------------------------------------------------------------------------------------------
FClr::HSV FColor::toHSV() const {
    auto r = rgbRed / 255.0;
    auto g = rgbGreen / 255.0;
    auto b = rgbBlue / 255.0;
    
    auto c = r + g +b;
    
    if (c < 1e-4) {
        return FClr::HSV(0,2/3,0);
    } else {
        auto pV = 2*(b*b+g*g+r*r-g*r-b*g-b*r);
        auto p = (pV > 1e-10) ? pow(pV, 0.5) : 0;   // gray values, p=0
        auto h = atan2(b-g,(2*r-b-g) / pow(3, 0.5));
        auto s = p/(c+p);
        auto v = (c+p)/3;
        return FClr::HSV(h,s,v);
    }
}

//-------------------------------------------------------------------------------------------------
FClr::XYZ FColor::toXYZ() const {
    float r = rgbRed / 255.0;
    float g = rgbGreen / 255.0;
    float b = rgbBlue / 255.0;
    
    if (r > 0.04045)
        r = powf(( (r + 0.055) / 1.055 ), 2.4);
    else r /= 12.92;
    
    if (g > 0.04045)
        g = powf(( (g + 0.055) / 1.055 ), 2.4);
    else g /= 12.92;
    
    if (b > 0.04045)
        b = powf(( (b + 0.055) / 1.055 ), 2.4);
    else b /= 12.92;
    
    r *= 100; g *= 100; b *= 100;
    
    // Calibration for observer @2° with illumination = D65
    float x = r * 0.4124 + g * 0.3576 + b * 0.1805;
    float y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    float z = r * 0.0193 + g * 0.1192 + b * 0.9505;
    
    return FClr::XYZ(x, y, z);
}

//-------------------------------------------------------------------------------------------------
// https://github.com/ThunderStruct/Color-Utilities
FClr::LAB FClr::XYZ::toLAB() const {
    float xn, yn, zn, l, a, b;
    const float refX = 95.047, refY = 100.0, refZ = 108.883;
    
    // References set at calibration for observer @2° with illumination = D65
    xn = x / refX;
    yn = y / refY;
    zn = z / refZ;
    
    if (xn > 0.008856)
        xn = powf(xn, 1 / 3.0);
    else xn = (7.787 * xn) + (16.0 / 116.0);
    
    if (yn > 0.008856)
        yn = powf(y, 1 / 3.0);
    else yn = (7.787 * yn) + (16.0 / 116.0);
    
    if (zn > 0.008856)
        zn = powf(z, 1 / 3.0);
    else zn = (7.787 * zn) + (16.0 / 116.0);
    
    l = 116 * y - 16;
    a = 500 * (x - y);
    b = 200 * (y - z);
    
    return LAB(l, a, b);
}

//-------------------------------------------------------------------------------------------------
FColor FClr::HSV::toRGB() const {
    auto r = value * (1+ sat *(cos(hue)-1));
    auto g = value * (1+ sat *(cos(hue-2.09439)-1));
    auto b = value * (1+ sat *(cos(hue+2.09439)-1));
    auto red = (unsigned)(r * 255);
    auto green = (unsigned)(g * 255);
    auto blue = (unsigned)(b * 255);
    return FColor(red, green, blue);
}
