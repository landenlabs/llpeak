//-------------------------------------------------------------------------------------------------
// File: CmdBlendF.cpp
// Desc: Execute "blend" function on image files.
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
#include "CmdBlendF.hpp"
#include "FileUtil.hpp"
#include "Split.hpp"
#include "FPrint.hpp"



//-------------------------------------------------------------------------------------------------
bool CmdBlendF::begin(StringList& fileDirList) {
    aux.verbose = true;
    aux.outCnt = 0;
    aux.outPath = output;
    aux.overlayImgRef = nullptr;
    aux.bottomImgRef = nullptr;

    bool okay = fileDirList.size() > 0 && imageCfg().valid();
    if (okay) {
        const FPalette& inPalette = imageCfg().getInPalette();
        
        int maxValue = 0;
        const int NO_VALUE = -1;
        std::vector<int> colorValues(inPalette.names.size(), NO_VALUE);
        for (unsigned idx = 0; idx < inPalette.names.size(); idx++) {
            Split nameParts(inPalette.names[idx], "-");
            if (nameParts.size() > 1) {
                char* endPtr;
                int val = (int)strtol(nameParts[1], &endPtr, 10);
                if (endPtr != nameParts[1].c_str()) {
                    colorValues[idx] = val;
                    maxValue = std::max(maxValue, val);
                }
            }
        }
        
        aux.bottomPalette.clear();
        aux.doBottom = imageCfg().bottomCfg.color.rgbReserved != 0;
        if (aux.doBottom) {
            FColor bg = imageCfg().bottomCfg.color;
            aux.bottomPalette.assign(256, bg);
            aux.bottomPalette[0]   = FColor::TRANSPARENT;
            aux.bottomPalette[255] = FColor::BLACK;
            
            for (unsigned idx = 1; idx < 255; idx++) {
                if (idx < colorValues.size()) {
                    int val = colorValues[idx];
                    if (val != NO_VALUE) {
                        BYTE rgb = (BYTE)((maxValue-val)*(256/maxValue));
                        aux.bottomPalette[idx] = FColor(rgb, rgb, rgb, bg.rgbReserved);
                    }
                }
            }
            std::cout << "Blend includes bottom Coverage layer\n";
        }
       
        aux.overlayMap.clear();
        aux.init();
    }
    
    return okay;
}

//-------------------------------------------------------------------------------------------------
// Locate matching files which are not in exclude list.
size_t CmdBlendF::add(const lstring& fullname, DIR_TYPES dtype) {
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
bool CmdBlendF::end() {
    
    bool okay = false;
    std::cout << "\nBlend" << " (" << paths.size() << ") images\n";
    if (paths.size() == 0) {
        std::cerr << "No images to process\n";
        return false;
    }
    
    std::sort(paths.begin(), paths.end());
   
    if (imageCfg().valid()) {
        
        // *****
        // TODO - Thread loading and conversion to 32bit
        // Ideal code execution:
        //     1. Threads preloading and converting to 32bit
        //     2. main cpu sequencial "blending" available images
        //     3. Threads saving images to disk [coding completed]
        // *****
        
        for (const std::string& fullname : paths) {
            ImageUtilF::Blend(fullname, imageCfg(), aux);
        }
        
        if (aux.overlayImgRef != nullptr) {
            // FPrint::printInfo(aux.overlayImgRef, "overlayImg");
            okay = ImageUtilF::saveTo(aux.overlayImgRef, "/tmp/llpeak-overlay.png");
       
        }
        if (aux.bottomImgRef != nullptr) {
            // FPrint::printInfo(aux.bottomImgRef, "bottomImg");
            okay = ImageUtilF::saveTo(aux.bottomImgRef, "/tmp/llpeak-bottom.png");
        }
        
        if (aux.overlayImgRef != nullptr) {
            const unsigned extra = 30;
            ImageUtilF::BlendFade(paths.back(), extra, imageCfg(), aux);
            aux.overlayImgRef->Close();
            *aux.overlayImgRef = nullptr;
        }
        
        if (aux.bottomImgRef != nullptr) {
            aux.bottomImgRef->Close();
            *aux.bottomImgRef = nullptr;
        }
       
    } else {
        std::cerr << "\nMissing or invalid config file" << std::endl;
        imageCfg().print(std::cerr);
    }
    
    aux.complete();
    return okay;
}
