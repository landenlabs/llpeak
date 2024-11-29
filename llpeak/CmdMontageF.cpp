//-------------------------------------------------------------------------------------------------
// File: CmdMontageF.cpp
// Desc: Montage (merge) image tiles into a single larger output image. 
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
#include "CmdMontageF.hpp"
#include "Directory.hpp"
#include "FPrint.hpp"
#include "FileUtil.hpp"


//-------------------------------------------------------------------------------------------------
bool CmdMontageF::begin(StringList& fileDirList) {
    // if (!imageCfg().isValid) {
    //     std::cerr << "Missing or invalid config file, use -config <cfg.json>\n";
    // }
    
    const char errMsg[] = "Missing or poorly formed montage tile command, expects widthXheigh, ex 1024x720 not ";
    
    char* nextPtr;
    xTiles = (int)strtol(cmdValue, &nextPtr, 10);
    if (nextPtr == cmdValue.c_str() || xTiles <= 0) {
        std::cerr << errMsg << cmdValue << std::endl;
        return false;
    }
    
    while (*nextPtr != '\0' && !isdigit(*nextPtr) )
        nextPtr++;
    
    yTiles = 0; // assume yTiles = totalTiles / xTiles
    if (isdigit(*nextPtr)) {
        char* ptr = nextPtr;
        yTiles = (int)strtol(ptr, &nextPtr, 10);
        if (ptr == nextPtr || yTiles <= 0) {
            std::cerr << errMsg << cmdValue << std::endl;
            return false;
        }
    }
    
    return fileDirList.size() > 0; //  && imageCfg().valid();
}

//-------------------------------------------------------------------------------------------------
// Locate matching files which are not in exclude list.
size_t CmdMontageF::add(const lstring& fullname, DIR_TYPES dtype) {
    size_t fileCount = 0;
    lstring name;
    FileUtil::getName(name, fullname);

    if (dtype == IS_FILE && !name.empty()
            && !FileUtil::FileMatches(name, excludeFilePatList, false)
            && FileUtil::FileMatches(name, includeFilePatList, true)) {
        fileCount++;

        struct stat info;
        if (stat(fullname, &info) == 0 && FileUtil::isWriteableFile(info)) {
            if (showFile)
                std::cout << fullname.c_str() << std::endl;
        } else {
            if (showFile)
                std::cout << "ReadOnly " << fullname.c_str() << std::endl;
        }
        paths.push_back(fullname);
    }

    return fileCount;
}

//-------------------------------------------------------------------------------------------------
bool CmdMontageF::end() {
    bool okay = true;

    std::cout << "\n" << name << " (" << paths.size() << ") images\n";
    if (paths.size() == 0) {
        std::cerr << "No images to process\n";
        return false;
    }
    
    // std::sort(paths.begin(), paths.end());
    
    FPalette inPalette;
    if (imageCfg().isValid) {
        inPalette = imageCfg().getInPalette();
    }
    okay |= ImageUtilF::Montage(inPalette, paths, xTiles, yTiles, output);

    return okay;
}
