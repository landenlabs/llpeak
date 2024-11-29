//-------------------------------------------------------------------------------------------------
//  File: FDraw.cpp
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

#include "FDraw.hpp"

const char FFontStd::vec[];

enum {
    A = 1<<0,
    B = 1<<1,
    C = 1<<2,
    D = 1<<3,
    E = 1<<4,
    F = 1<<5,
    G = 1<<6,
    H = 1<<7,
};

//-------------------------------------------------------------------------------------------------
unsigned char FFontStd::FONT_MAP[sizeof(FFontStd::vec)][FONT_HEIGHT] = {
    /* */ {0, 0, 0, 0, 0, 0, 0, 0}, /* it wasn't funny! */
    /*0*/ {B|C|D|E|F, A|F|G, A|E|G, A|D|G, A|C|G, A|B|G, B|C|D|E|F, 0},
    /*1*/ {G, F|G, G, G, G, G, G, 0},
    /*2*/ {B|C|D|E|F, A|G, G, C|D|E|F, B, A, A|B|C|D|E|F|G, 0},
    /*3*/ {B|C|D|E|F, A|G, G, C|D|E|F, G, A|G, B|C|D|E|F, 0},
    /*4*/ {A|F, A|F, A|F, B|C|D|E|F|G, F, F, F, 0},
    /*5*/ {A|B|C|D|E|F|G, A, A, B|C|D|E|F, G, A|G, B|C|D|E|F, 0},
    /*6*/ {B|C|D|E|F, A, A, A|B|C|D|E|F, A|G, A|G, B|C|D|E|F, 0},
    /*7*/ {B|C|D|E|F|G, G, F, E, D, C, B, 0},
    /*8*/ {B|C|D|E|F, A|G, A|G, B|C|D|E|F, A|G, A|G, B|C|D|E|F, 0},
    /*9*/ {B|C|D|E|F, A|G, A|G, B|C|D|E|F|G, G, G, B|C|D|E|F, 0},
    /*a*/ {0, 0, B|C|D|E, F, B|C|D|E|F, A|F, B|C|D|E|G, 0},
    /*b*/ {B, B, B, B|C|D|E|F, B|G, B|G, A|C|D|E|F, 0},
    /*c*/ {0, 0, C|D|E, B|F, B, B|F,C|D|E,0},
    /*d*/ {F, F, F, B|C|D|E|F, A|F, A|F, B|C|D|E|G, 0},
    /*e*/ {0, 0, B|C|D|E, A|F, A|B|C|D|E|F, A, B|C|D|E, 0},
    /*f*/ {C|D|E, B, B, A|B|C|D, B, B, B, 0},
    /*g*/ {0, 0, B|C|D|E|F, A|F, A|F, B|C|D|E|F, F, B|C|D|E},
    /*h*/ {B, B, B, B|C|D|E, B|F, B|F, A|B|F, 0},
    /*i*/ {0, C, 0, B|C, C, C, A|B|C|D|E, 0},
    /*j*/ {0, D, 0, D, D, D, A|D, B|C},
    /*k*/ {0, B|E, B|D, B|C, B|D, B|E,B|F, 0},
    /*l*/ {B|C, C, C, C, C, C, A|B|C|D, 0},
    /*m*/ {0, 0, A|C|E|F, A|B|D|G, A|D|G, A|D|G, A|D|G, 0},
    /*n*/ {0, 0, B|D|E, B|C|F, B|F, B|F, B|F},
    /*o*/ {0, 0, B|C|D|E, A|F, A|F, A|F, B|C|D|E, 0},
    /*p*/ {0, 0, A|B|C|D|E, B|F, B|F, B|C|D|E, B, A|B},
    /*q*/ {0, 0, B|C|D|E|G, A|F, A|F, B|C|D|E, F, F|G},
    /*r*/ {0, 0, A|B|D|E, B|C|F, B, B, A|B, 0},
    /*s*/ {0, 0, B|C|D|E, A, B|C|D|E, F, A|B|C|D|E, 0},
    /*t*/ {0, C, C, A|B|C|D|E, C, C, D|E, 0},
    /*u*/ {0, 0, A|F, A|F, A|F, A|F, B|C|D|E|G, 0},
    /*v*/ {0, 0, A|G, B|F, B|F, C|E, D, 0},
    /*w*/ {0, 0, A|G, A|G, A|D|G, A|D|G, B|C|E|F, 0},
    /*x*/ {0, 0, A|F, B|E, C|D, B|E, A|F, 0},
    /*y*/ {0, 0, B|F, B|F, B|F, C|F, A|D|E, B|C|D},
    /*z*/ {0, 0, A|B|C|D|E|F, E, D, C, B|C|D|E|F|G, 0},
    /*A*/ {D, C|E, B|F, A|G, A|B|C|D|E|F|G, A|G, A|G, 0},
    /*B*/ {A|B|C|D|E, A|F, A|F, A|B|C|D|E, A|F, A|F, A|B|C|D|E, 0},
    /*C*/ {C|D|E, B|F, A, A, A, B|F, C|D|E, 0},
    /*D*/ {A|B|C|D, A|E, A|F, A|F, A|F, A|E, A|B|C|D, 0},
    /*E*/ {A|B|C|D|E|F, A, A, A|B|C|D|E, A, A, A|B|C|D|E|F, 0},
    /*F*/ {A|B|C|D|E|F, A, A, A|B|C|D|E, A, A, A, 0},
    /*G*/ {C|D|E, B|F, A, A, A|E|F|G, B|F|G, C|D|E|G, 0},
    /*H*/ {A|G, A|G, A|G, A|B|C|D|E|F|G, A|G, A|G, A|G, 0},
    /*I*/ {A|B|C|D|E, C, C, C, C, C, A|B|C|D|E, 0},
    /*J*/ {A|B|C|D|E, C, C, C, C, C, A|C, B},
    /*K*/ {A|F, A|E, A|D, A|B|C, A|D, A|E, A|F, 0},
    /*L*/ {A, A, A, A, A, A, A|B|C|D|E|F, 0},
    /*M*/ {A|B|F|G, A|C|E|G, A|D|G, A|G, A|G, A|G, A|G, 0},
    /*N*/ {A|G, A|B|G, A|C|G, A|D|G, A|E|G, A|F|G, A|G, 0},
    /*O*/ {C|D|E, B|F, A|G, A|G, A|G, B|F, C|D|E, 0},
    /*P*/ {A|B|C|D, A|E, A|E, A|B|C|D, A, A, A, 0},
    /*Q*/ {C|D|E, B|F, A|G, A|G, A|C|G, B|D|F, C|D|E, F|G},
    /*R*/ {A|B|C|D, A|E, A|E, A|B|C|D, A|E, A|F, A|F, 0},
    /*S*/ {C|D|E, B|F, C, D, E, B|F, C|D|E, 0},
    /*T*/ {A|B|C|D|E|F|G, D, D, D, D, D, D, 0},
    /*U*/ {A|G, A|G, A|G, A|G, A|G, B|F, C|D|E, 0},
    /*V*/ {A|G, A|G, B|F, B|F, C|E, C|E, D, 0},
    /*W*/ {A|G, A|G, A|G, A|G, A|D|G, A|C|E|G, B|F, 0},
    /*X*/ {A|G, A|G, B|F, C|D|E, B|F, A|G, A|G, 0},
    /*Y*/ {A|G, A|G, B|F, C|E, D, D, D, 0},
    /*Z*/ {A|B|C|D|E|F|G, F, E, D, C, B, A|B|C|D|E|F|G, 0},
    /*.*/ {0, 0, 0, 0, 0, 0, D, 0},
    /*,*/ {0, 0, 0, 0, 0, E, E, D},
    /*:*/ {0, 0, 0, D, 0, 0, D, 0},
    /*!*/ {D, D, D, D, D, 0, D, 0},
    /*/ */ {G, F, E, D, C, B, A, 0},
    /*\\*/ {A, B, C, D, E, F, G, 0},
    /*|*/ {D, D, D, D, D, D, D, D},
    /*+*/ {0, D, D, B|C|D|E|F, D, D, 0, 0},
    /*-*/ {0, 0, 0, B|C|D|E|F, 0, 0, 0, 0},
    /***/ {0, B|D|F, C|D|E, D, C|D|E, B|D|F, 0, 0},
    /*=*/ {0, 0, B|C|D|E|F, 0, B|C|D|E|F, 0, 0, 0},
    
    // ()[]{}
    /*(*/ { C|D, B, A, A, A, B, C|D, 0},
    /*)*/ { D|E, F, G, G, G, F, D|E, 0},
    /*[*/ { A|B, A, A, A, A, A, A|B, 0},
    /*]*/ { F|G, G, G, G, G, G, F|G, 0},
    /*{*/ {   B, A, A, A|B, A, A, B, 0},
    /*}*/ {   F, G, G, F|G, G, G, F, 0}
};


static FFontStd STD_INSTANCE;
const FFontStd& FFontStd::instance() {
    return STD_INSTANCE;
}
 
//-------------------------------------------------------------------------------------------------
bool FDraw::DrawTextI8(
            FDrawFunc& funcs, FDrawHnd drawHnd,
            const lstring& text,
            const FBrush& brush,
            unsigned x,  // upper left column
            unsigned y,  // upper left row
            float scale,
            const FFont& font) {

    const unsigned TEXTLEN = (unsigned)text.length();
    const unsigned HEIGHT = font.getHeight();
    const unsigned BitsPerChar = 8;
    const unsigned X_SPACING = 8;
    const char* CHAR_SET = font.getChars();
    const unsigned char* BITMAP = font.getMap();
    
    unsigned row,col;
    unsigned idx;
    bool okay = true;
    
    if (scale <= 1) {
        for (idx = 0; idx < TEXTLEN; idx++) {
            char sp = text[idx];
            for (row=0; row < HEIGHT; row++) {
                const char* setPtr = strchr(CHAR_SET, sp);
                if (setPtr != NULL) {
                    unsigned vecOff = (unsigned)(setPtr - CHAR_SET);
                    for (col=0; col < BitsPerChar; col++) {
                        bool draw = (BITMAP[vecOff*HEIGHT + row] & 1<<col) != 0;
                        if (draw) {
                            bool ok = funcs.drawPixelI8(drawHnd, x + col + X_SPACING*idx, y + HEIGHT-row, brush.textIndex);
                            okay &= ok;
                        }
                    }
                }
            }
        }
    } else {
        for (idx = 0; idx < TEXTLEN; idx++) {
            char sp = text[idx];
            for (row=0; row < HEIGHT; row++) {
                const char* setPtr = strchr(CHAR_SET, sp);
                if (setPtr != NULL) {
                    unsigned vecOff = (unsigned)(setPtr - CHAR_SET);
                    for (col=0; col < BitsPerChar; col++) {
                        bool draw = (BITMAP[vecOff*HEIGHT + row] & 1<<col) != 0;
                        if (draw) {
                            unsigned x1 = x + (unsigned)((col + X_SPACING*idx) * scale);
                            unsigned y1 = y + (unsigned)((HEIGHT-row) * scale);
                            unsigned x2 =  (unsigned)(x1 + scale);
                            unsigned y2 =  (unsigned)(y1 + scale);
                            bool ok = funcs.drawBoxI8(drawHnd, x1,y1, x2,y2 , brush.textIndex);
                            okay &= ok;
                        }
                    }
                }
            }
        }
    }
 
    
    return okay;
}
