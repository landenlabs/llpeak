//-------------------------------------------------------------------------------------------------
//  File: ImageAux.hpp
//  Desc: Auxiliary data used by Image functions
//
//  ImageAux created by Dennis Lang on 12/21/21.
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

// Project files
#include "FShade.hpp"
#include "FImage.hpp"
#include "FPalette.hpp"
#include "ImageCfg.hpp"
#include "PalMapping.hpp"

#define USE_THREAD
#ifdef USE_THREAD
#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <memory>         // unique_ptr
#include "RingBuffer.hpp"

// Forward declaration
class ImageAux;

//-------------------------------------------------------------------------------------------------
// Class to manage saving Images in a thread.
class ThreadJob {
public:
    FImage img;
    lstring name;
    std::thread thread1;
    bool verbose;
    
    ThreadJob()
    { }
    ThreadJob(FImage& _img, const lstring& _name, bool _verbose = false) :
        img(_img),
        name(_name),
        verbose(_verbose),
        thread1(&ThreadJob::saveImageThreadFnc, this) {
    }
    ~ThreadJob()
    { }
    ThreadJob& operator=( ThreadJob& other) {
        if (this != &other) {
            img = other.img;
            name = other.name;
            verbose = other.verbose;
            thread1.swap(other.thread1);
        }
        return *this;
    }

    // Start thread to save image.
    bool StartThread( FImage& img, const char* toName, ImageAux* aux = nullptr);
    // Wait for pending threads to complete.
    void EndThreads();
    
private:
    void saveImageThreadFnc();
};
#endif

//-------------------------------------------------------------------------------------------------
// Auxiliary data used by Image functions.
class ImageAux {
public:
    // General
    bool        verbose = false;
    unsigned    outCnt = 0;
    lstring     outPath;
    
    // Blend
    FImageRef   overlayImgRef;
    FImageRef   bottomImgRef;
    PalMapping  overlayMap;
    PalMapping  bottomMap;
    FPalette    bottomPalette;
    bool        doBottom = false;
    
    // Shade
    FShadeRef   shadeRef;
    PalMapping  shadeMap;
    
    // Colorlapse
    FImage      colorizeImg;
    
    // Thread saving used by Blend
    bool        useThread = false;
#ifdef USE_THREAD
    ThreadJob threadSaveImage;

    void init() {
        useThread = true;
    }
    // Threading
    void complete() {
        threadSaveImage.EndThreads();
    }
#else
    void init()
    {  }
    void complete()
    {   }
#endif
};

