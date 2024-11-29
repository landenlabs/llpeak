//-------------------------------------------------------------------------------------------------
// File: CmdColorlapse.cpp
// Desc: Execute "colorlapse" color timelapse function on image files.
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
#include "CmdColorlapseF.hpp"
#include "FileUtil.hpp"
#include "Split.hpp"
#include "FPrint.hpp"



//-------------------------------------------------------------------------------------------------
bool CmdColorlapseF::begin(StringList& fileDirList) {
    aux.verbose = true;
    aux.outCnt = 0;
    aux.outPath = output;
    aux.overlayImgRef = nullptr;
    aux.bottomImgRef = nullptr;

    bool okay = fileDirList.size() > 0 && imageCfg().valid();
    if (okay) {
        aux.overlayMap.clear();
        aux.init();
    }
    
    return okay;
}

//-------------------------------------------------------------------------------------------------
// Locate matching files which are not in exclude list.
size_t CmdColorlapseF::add(const lstring& fullname, DIR_TYPES dtype) {
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
bool CmdColorlapseF::end() {
    
    bool okay = false;
    std::cout << "\nColorlapse" << " (" << paths.size() << ") images\n";
    if (paths.size() == 0) {
        std::cerr << "No images to process\n";
        return false;
    }
    
    std::sort(paths.begin(), paths.end());
   
    if (imageCfg().valid()) {
        for (const std::string& fullname : paths) {
            ImageUtilF::Colorlapse(fullname, imageCfg(), aux);
        }
    } else {
        std::cerr << "\nMissing or invalid config file" << std::endl;
        imageCfg().print(std::cerr);
    }
    
    aux.complete();
    return okay;
}
