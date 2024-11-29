//-------------------------------------------------------------------------------------------------
//  File: llpeak
//  Desc: "Image manipulation"
//
//  llpeak created by Dennis Lang on 12/21/21.
//  Copyright © 2022 Dennis Lang. All rights reserved.
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2022
// https://landenlabs.com/
//
// This file is part of llpeak project.
//
// ----- License ----
//
// Copyright (c) 2022 Dennis Lang
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

#ifdef HAVE_WIN
// 4291 - No matching operator delete found
#pragma warning( disable : 4291 )
#endif
#define _CRT_SECURE_NO_WARNINGS

// Project files
#include "CmdBlendF.hpp"
#include "CmdShadeF.hpp"
#include "CmdColorlapseF.hpp"
#include "CmdDumpF.hpp"
#include "CmdBlurF.hpp"
#include "CmdToGrayF.hpp"
#include "Directory.hpp"
#include "Split.hpp"
#include "ImageCfg.hpp"

// C++
#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

// C
#include <ctype.h>
#include <stdio.h>

// #include <Magick++.h>
// using namespace Magick;

using namespace std;

// Helper types
typedef std::vector<std::regex> PatternList;
typedef unsigned int uint;

uint optionErrCnt = 0;
uint patternErrCnt = 0;

#ifdef HAVE_WIN
#include <assert.h>
#define strncasecmp _strnicmp
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif
#else
#include <signal.h>
#include <unistd.h>
#endif

//-------------------------------------------------------------------------------------------------
// Recurse over directories, locate files.
static size_t InspectFiles(Command& command, const lstring& dirname, unsigned depth) {
    static std::vector<size_t> counts(10, 0);

    if (Command::abortFlag) {
        return 0;
    }

    Directory_files directory(dirname);
    lstring fullname;

    size_t fileCount = 0;

    struct stat filestat;
    try {
        if (stat(dirname, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
            fileCount += command.add(dirname, IS_FILE);
            return fileCount;
        }
    } catch (exception ex) {
        // Probably a pattern, let directory scan do its magic.
    }

    while (!Command::abortFlag && directory.more()) {
        directory.fullName(fullname);
        if (directory.is_directory()) {
            counts[depth + 1] = 0;
            fileCount += command.add(fullname, IS_DIR_BEG);  // add directory, fullname may change
            fileCount += InspectFiles(command, fullname, depth + 1);
            fileCount += command.add(fullname, IS_DIR_END);  // add directory.
        } else if (fullname.length() > 0) {
            fileCount += command.add(fullname, IS_FILE);
        }

        if (fileCount >= counts[depth] + 10) {
            counts[depth] = fileCount;
            std::cerr << "\r ";
            for (unsigned idx = 0; idx <= depth; idx++)
                std::cerr << counts[idx] << " ";
            std::cerr << " " << dirname << " ";
        }
    }

    return fileCount;
}

//-------------------------------------------------------------------------------------------------
// Return compiled regular expression from text.
std::regex getRegEx(const char* value) {
    try {
        std::string valueStr(value);
        return std::regex(valueStr);
        // return std::regex(valueStr, regex_constants::icase);
    } catch (const std::regex_error& regEx) {
        std::cerr << regEx.what() << ", Pattern=" << value << std::endl;
    }

    patternErrCnt++;
    return std::regex("");
}

//-------------------------------------------------------------------------------------------------
// Validate option matchs and optionally report problem to user.
bool ValidOption(const char* validCmd, const char* possibleCmd, bool reportErr = true) {
    // Starts with validCmd else mark error
    size_t validLen = strlen(validCmd);
    size_t possibleLen = strlen(possibleCmd);

    if (strncasecmp(validCmd, possibleCmd, std::min(validLen, possibleLen)) == 0)
        return true;

    if (reportErr) {
        std::cerr << "Unknown option:'" << possibleCmd << "', expect:'" << validCmd << "'\n";
        optionErrCnt++;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
// Convert special characters from text to binary.
static std::string& ConvertSpecialChar(std::string& inOut) {
    uint len = 0;
    int x, n;
    const char* inPtr = inOut.c_str();
    char* outPtr = (char*)inPtr;
    while (*inPtr) {
        if (*inPtr == '\\') {
            inPtr++;
            switch (*inPtr) {
                case 'n':
                    *outPtr++ = '\n';
                    break;
                case 't':
                    *outPtr++ = '\t';
                    break;
                case 'v':
                    *outPtr++ = '\v';
                    break;
                case 'b':
                    *outPtr++ = '\b';
                    break;
                case 'r':
                    *outPtr++ = '\r';
                    break;
                case 'f':
                    *outPtr++ = '\f';
                    break;
                case 'a':
                    *outPtr++ = '\a';
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    sscanf(inPtr, "%3o%n", &x, &n);
                    inPtr += n - 1;
                    *outPtr++ = (char)x;
                    break;
                case 'x':  // hexadecimal
                    sscanf(inPtr + 1, "%2x%n", &x, &n);
                    if (n > 0) {
                        inPtr += n;
                        *outPtr++ = (char)x;
                        break;
                    }
                    // seep through
                default:
                    throw("Warning: unrecognized escape sequence");
                case '\\':
                case '\?':
                case '\'':
                case '\"':
                    *outPtr++ = *inPtr;
                    break;
            }
            inPtr++;
        } else
            *outPtr++ = *inPtr++;
        len++;
    }

    inOut.resize(len);
    return inOut;
    ;
}

//-------------------------------------------------------------------------------------------------
void showTitle(char* name) {
    cerr << "\n" << name << "  Dennis Lang v2.2 (landenlabs.com) " __DATE__ << "\n";
}

//-------------------------------------------------------------------------------------------------
void showHelp(char* name) {
    showTitle(name);
    cerr << "\nDes: 'Image Manipulation\n"
            "Use: llpeak [options] directories...   or  files\n"
            "\n"
            " Options (only first unique characters required, options can be repeated): \n"
            "\n"
            "   -blend   ; Sequenced image blend\n"
            "   -dump    ; Dump image info, palette, create palette legends \n"
            "   -shade1  ; Apply shade (2+D) look to image (equation #1)\n"
            "   -shade2  ; Apply shade (2+D) look to image (equation #2) \n"
            "   -blur    ; Blur (smooth) image \n"
            "   -colorlapse ; Timelapse color blend \n"
            "   -montage ; Merge image tiles together \n"
            "   -toGray  ; Convert 32bit gray to 8bit gray \n"
            "\n"
            "   -config[=]<config.json>   ; Image palette and manipulation configuration \n"
            "\n"
            " Generic commands: (all directories recursively scanned) \n"
            "   -includefile=<filePattern>\n"
            "   -excludefile=<filePattern>\n"
            "   -config <filecfg.json>\n"
            "   -verbose \n"
            "\n"
            " Example: \n"
            "   llpeak -include=\\*.png -exclude=Wind\\*png -config radar.json ~/data/ \n"
            "   llpeak -config shade.json ~/datapath1/ ~/datapath2/ foo.png car.jpg \n"
            "   llpeak -dump foo.png \n"
            "   llpeak -montage=4x3 -include=\\*.png -output=bigImage.png ~/tiles"
            "\n"
            "\n";
}

#ifdef HAVE_WIN
//-------------------------------------------------------------------------------------------------
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:  // Handle the CTRL-C signal.
            Command::abortFlag = true;
            std::cerr << "\nCaught signal " << std::endl;
            Beep(750, 300);
            exit(-1);
            return TRUE;
    }

    return FALSE;
}

#else
//-------------------------------------------------------------------------------------------------
void sigHandler(int /* sig_t */ s) {
    Command::abortFlag = true;
    std::cerr << "\nCaught signal - exiting" << std::endl;
    exit(-1);
}
#endif

//-------------------------------------------------------------------------------------------------
// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime(time_t& now) {
    now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}


//-------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    StringList      fileDirList;
    ImageCfg        imageCfg;
    CmdBlendF       doBlendF(&imageCfg);
    CmdDumpF        doDumpF(&imageCfg);
    CmdShadeF       doShadeF(&imageCfg);
    CmdColorlapseF  doColorlapse(&imageCfg);
    CmdToGrayF      toGray(&imageCfg);
    CmdBlurF        doBlurF(&imageCfg);
    CmdNone         doNone(&imageCfg);
    Command*        commandPtr = &doNone;

    
#ifdef HAVE_WIN
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        std::cerr << "Failed to install sig handler" << endl;
    }
#else
    // signal(SIGINT, sigHandler);

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    if (sigaction(SIGINT, &sigIntHandler, NULL) != 0) {
        std::cerr << "Failed to install sig handler" << endl;
    }
#endif

    if (argc == 1) {
        showHelp(argv[0]);
    } else {
        // InitializeMagick(*argv);

        bool doParseCmds = true;
        string endCmds = "--";
        for (int argn = 1; argn < argc; argn++) {
            if (*argv[argn] == '-' && doParseCmds) {
                lstring argStr(argv[argn]);
                Split cmdValue(argStr, "=", 2);
                if (cmdValue.size() == 2) {
                    lstring cmd = cmdValue[0];
                    lstring value = cmdValue[1];

                    switch (cmd[(unsigned)1]) {
                        case 'c':  // -config=<cfgFile.json>
                            if (ValidOption("config", cmd + 1)) {
                                imageCfg.parseConfig(value);
                                // imageCfg().print();
                            }
                            break;
                        case 'e':  // excludeFile=<pat>
                            if (ValidOption("excludefile", cmd + 1)) {
                                ReplaceAll(value, "*", ".*");
                                commandPtr->excludeFilePatList.push_back(getRegEx(value));
                            }
                            break;
                        case 'i':
                            if (ValidOption("includefile", cmd + 1)) {
                                // includeFile=<pat>
                                ReplaceAll(value, "*", ".*");
                                commandPtr->includeFilePatList.push_back(getRegEx(value));
                            }
                            break;
                        case 'm':  // montage=<width x height>
                            if (ValidOption("montage", cmd + 1)) {
                                commandPtr->cmdValue = value;
                            }
                            break;
                        case 'o':  // output=<path>
                            if (ValidOption("output", cmd + 1)) {
                                commandPtr->output = value;
                            }
                            break;

                        default:
                            std::cerr << "Unknown parameters " << cmd << std::endl;
                            optionErrCnt++;
                            break;
                    }
                } else {
                    switch (argStr[(unsigned)1]) {
                        case '?':
                            showHelp(argv[0]);
                            break;
                            
                        case 'h':
                            if (ValidOption("help", argStr + 1)) {
                                showHelp(argv[0]);
                            }
                            break;

                        case 'v':
                            commandPtr->verbose = true;
                            commandPtr->showFile = true;
                            continue;
                            
                        case 'b':
                            if (ValidOption("blend", argStr + 1, false)) {
                                commandPtr = &doBlendF.share(*commandPtr);
                                continue;
                            } else if (ValidOption("blur", argStr + 1)) {
                                commandPtr = &doBlurF.share(*commandPtr);
                                continue;
                            }
                            break;
                            
                        case 'c':  // -config <cfgFile.json>
                            if (ValidOption("colorlapse", argStr + 1, false)) {
                                commandPtr = &doColorlapse.share(*commandPtr);
                                continue;
                            } else if (ValidOption("config", argStr + 1) && argn < argc) {
                                lstring value(argv[++argn]);
                                imageCfg.parseConfig(value);
                                continue;
                            }
                            break;
                            
                        case 'd':
                            if (ValidOption("dump", argStr + 1)) {
                                commandPtr = &doDumpF.share(*commandPtr);
                                continue;
                            }
                            break;
                            
                        case 'o':  // -output <outPath|outFile>
                            if (ValidOption("output", argStr + 1) && argn < argc) {
                                lstring value(argv[++argn]);
                                commandPtr->output = value;
                                continue;
                            }
                    
                        case 's':
                            if (ValidOption("shade1", argStr + 1, false)) {
                                commandPtr = &doShadeF.share(*commandPtr);
                                doShadeF.getAux().shadeRef = new FShadeXY1();
                                continue;
                            } else if (ValidOption("shade2", argStr + 1, false)) {
                                commandPtr = &doShadeF.share(*commandPtr);
                                doShadeF.getAux().shadeRef = new FShadeXY2();
                                continue;
                            } else if (ValidOption("shade3", argStr + 1)) {
                                commandPtr = &doShadeF.share(*commandPtr);
                                doShadeF.getAux().shadeRef = new FShadeXY3();
                                continue;
                            }
                            break;
                            
                        case 't':
                            if (ValidOption("togray", argStr + 1)) {
                                commandPtr = &toGray.share(*commandPtr);
                                continue;
                            }
                            break;

                    }

                    if (endCmds == argv[argn]) {
                        doParseCmds = false;
                    } else {
                        std::cerr << "Unknown command " << argStr << std::endl;
                        optionErrCnt++;
                    }
                }
            } else {
                // Store file directories
                fileDirList.push_back(argv[argn]);
            }
        }

        if (commandPtr == nullptr) {
            std::cerr << "Must select command, use -blend, -shade1, or -shade2, see Help\n";
            showHelp(argv[0]);
            exit(-1);
        }
        
        if (commandPtr->begin(fileDirList)) {
            time_t startT;
            showTitle(argv[0]);
            std::cerr << "Start " << currentDateTime(startT) << std::endl;

            if (patternErrCnt == 0 && optionErrCnt == 0 && fileDirList.size() != 0) {
                if (fileDirList.size() == 1 && fileDirList[0] == "-") {
                    lstring filePath;
                    while (std::getline(std::cin, filePath)) {
                        // size_t filesChecked = 
                        InspectFiles(*commandPtr, filePath, 0);
                        // std::cerr << "\n  Files Checked=" << filesChecked << std::endl;
                    }
                } else {
                    for (const lstring& filePath : fileDirList) {
                        // size_t filesChecked = 
                        InspectFiles(*commandPtr, filePath, 0);
                        // std::cerr << "\n  Files Checked=" << filesChecked << std::endl;
                    }
                }
            }

            commandPtr->end();
            time_t endT;
            std::cerr << "\nEnd " << currentDateTime(endT) << std::endl;
            std::cout << "Elapsed " << std::difftime(endT, startT) << " (sec)\n";
        }

        std::cerr << std::endl;
    }

    return 0;
}
