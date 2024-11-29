//-------------------------------------------------------------------------------------------------
// File: Command.hpp
// Desc: Base class for commands to process files. 
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
#include "Directory.hpp"
#include "ImageUtilF.hpp"
#include "ImageCfg.hpp"

// C++
#include <vector>
#include <regex>
#include <fstream>

// Helper types
typedef std::vector<lstring> StringList;
typedef std::vector<std::regex> PatternList;
typedef unsigned int uint;
typedef std::vector<unsigned> IntList;
typedef char Byte;
// const unsigned BLOCK_SIZE = 1024;

// typedef  std::shared_ptr<ImageCfg>  ImageCfgRef;
typedef  ImageCfg*  ImageCfgRef;

//-------------------------------------------------------------------------------------------------
class Command {
public:
    // Runtime options
    PatternList includeFilePatList;
    PatternList excludeFilePatList;
    lstring output;
    lstring cmdValue;
    
    bool showFile = false;
    bool verbose = false;
    static volatile bool abortFlag;
    
    lstring name;
    ImageCfgRef imageCfgRef;
 
public:
    Command(const lstring& _name, ImageCfgRef cfg) : name(_name), imageCfgRef(cfg) {
    }

    virtual  bool begin(StringList& fileDirList)  {
        return fileDirList.size() > 0;
    }

    virtual size_t add( const lstring& file, DIR_TYPES dtypes) = 0;

    virtual bool end() {
        return true;
    }

    Command& share(const Command& other) {
        includeFilePatList = other.includeFilePatList;
        excludeFilePatList = other.excludeFilePatList;
        output = other.output;
        cmdValue = other.cmdValue;
        
        showFile = other.showFile;
        verbose = other.verbose;
      
        name = other.name;
        imageCfgRef = other.imageCfgRef;
        
        return *this;
    }
    
    ImageCfg& imageCfg() { return *imageCfgRef; }
    const ImageCfg& imageCfg() const { return *imageCfgRef; }
};

// Dummy command to hold any options set prior to actual command selected.
class CmdNone : public Command {
public:
    CmdNone( ImageCfgRef cfg) : Command("none", cfg) {}
    size_t add( const lstring& file, DIR_TYPES dtypes) {
        return 0;
    }
};
