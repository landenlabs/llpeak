//-------------------------------------------------------------------------------------------------
//  File: FDraw.hpp
//  Desc: General purpose FreeImage draw rountes, such as drawing Text on an image.
//
//  Created by Dennis Lang on 12/21/21.
//  Copyright © 2022 Dennis Lang. All rights reserved.
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


#include "FBrush.hpp"
#include "lstring.hpp"

typedef void* FDrawHnd;
typedef bool (DLL_CALLCONV *FDrawPixelI8_func) (FDrawHnd handle, unsigned x, unsigned y, unsigned char colorI8);
typedef bool (DLL_CALLCONV *FDrawBoxI8_func) (FDrawHnd handle, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned char colorI8);


class FDrawFunc {
public:
    FDrawPixelI8_func drawPixelI8;      //! pointer to the function used to draw pixel on index 8bit image.
    FDrawBoxI8_func drawBoxI8;          //! pointer to the function used to draw box on index 8bit image.
};


class FFont {
public:
    virtual const unsigned char* getMap() const = 0;
    virtual unsigned getHeight() const = 0;
    virtual unsigned getLength() const = 0;
    virtual const char* getChars() const = 0;
};

class FFontStd : public FFont {
    constexpr static char vec[] = " 0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    ".,:!/\\|+-*=()[]{}";
    
    static const unsigned FONT_HEIGHT = 8;
    static unsigned char FONT_MAP[sizeof(vec)][FONT_HEIGHT];
public:
    static const FFontStd& instance();
    
    const unsigned char* getMap() const {
        return &FONT_MAP[0][0];
    }
    unsigned getHeight() const {
        return FONT_HEIGHT;
    }
    unsigned getLength() const {
        return sizeof(vec);
    }
    const char* getChars() const {
        return vec;
    }
};


class FDraw {
public:
    static
    bool DrawTextI8(
            FDrawFunc& funcs,
            FDrawHnd drawHnd,
            const lstring& text,
            const FBrush& brush,
            unsigned x1,
            unsigned y1,
            float scale = 1.0f,
            const FFont& font = FFontStd::instance());
};
