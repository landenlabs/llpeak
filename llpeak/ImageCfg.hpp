//-------------------------------------------------------------------------------------------------
// File: FConfig.hpp
// Desc: Image blend/shade configuration parsed from json file.
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
#include "ll_stdhdr.hpp"
#include "Json.hpp"
#include "FPalette.hpp"
#include "FColor.hpp"

// C++
#include <memory>
#include <iostream>
#include <map>

// C
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


class PixelFilterCfg {
public:
    float rate = 0.9f;          // 0..< 1=darkens, 1=no change, > 1=brightens,
    unsigned startAlpha = 255;  // 0..255
    unsigned endAlpha = 0;      // 0..255
};
class OverlayCfg {
public:
    float    alphaMultiple = 0.99f;   // 0..< 1.0=fade, 1.0=no change, > 1.0 invalid.
    unsigned alphaMinimum = 0;        // 0..255
};
class BottomCfg {
public:
    FColor color;
    BottomCfg() : color(0,0,0,0)    // alpha=0 no bottom
    { }
};

class ImageCfg {
public:
    ImageCfg() : isValid(false)
    { }

    bool parseConfig(const lstring& cfgFilename);
    void print(ostream& out = std::cerr);
    bool valid() const {
        return isValid;
    }
    
    struct stat filestat;
    JsonBuffer buffer;
    JsonFields fields;

    FPalette inPalette;
    FPalette outPalette;
    FPalette overlayPalette;
    std::map<lstring, PixelFilterCfg> overlayFilters;
    OverlayCfg overlayCfg;
    BottomCfg bottomCfg;
    bool isValid;
   
    enum OverlayOrder { OVER_IMAGE, UNDER_IMAGE };
    OverlayOrder overlayerOrder = UNDER_IMAGE;
    
    const FPalette&  getInPalette();         
    const FPalette&  getOutPalette();
    const FPalette&  getOverlayPalette();
    
    const OverlayCfg& getOverlayCfg() const {
        return overlayCfg;
    }
    bool hasOverlayFilter(const char* id) const {
        return overlayFilters.find(id) != overlayFilters.cend();
    }
    const PixelFilterCfg& getOverlayFilter(const char* id)  {
        return overlayFilters[id];
    }

    FColor filter(const PixelFilterCfg& cfg, const FColor& inColor) const;
    
private:
    bool parsePalette(const char* PaletteID, FPalette& palette, const FPalette* refPalettePtr = nullptr);
    bool getMapList(const char* id, MapList& mapList, const lstring& validIds) const;
    bool getMapList(const char* id, JsonMap& mapList, const lstring& validIds) const;
};
