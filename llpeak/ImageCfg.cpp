//-------------------------------------------------------------------------------------------------
// File: ImageCfg.cpp
// Desc: Parse json config file.
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

// 4291 - No matching operator delete found
// #pragma warning(disable : 4291)
#define _CRT_SECURE_NO_WARNINGS

// Project files
#include "ImageCfg.hpp"
#include "Directory.hpp"
#include "Json.hpp"

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

/*
  Sample Radar configuration json file:
 
 {
     // Temperature config
     // https://ssds-catalogui-useast1.qa.ssds.weather.com/v2/catalogui/tilepaletter/palettes/temp
     "in-palette" : {
         "transparent" : "0,0,0,0",
            
         "rain-1" : "99,235,99",       //  1  green
         "rain-2" : "60,198,60",      //  2  green
         "rain-3" : "28,157,52",      //  3  green
         "rain-4" : "14,104,26",      //  4  FColor
         "rain-5" : "0,63,0",         //  5  green
     
         "rain-6" : "251,235,2",      //  6  yellow
         "rain-7" : "238,109,2",      //  7  orange
     
         "rain-8" : "210,11,6",       //  8  red
         "rain-9" : "189,8,4",        //  9  red
         "rain-10" : "169,5,3",       // 10  red
         "rain-11" : "148,2,1",       // 11  red
         "rain-12" : "128,0,0",       // 12  red

         // Freeze
         "freeze-1" : "188,165,240",    // purple
         "freeze-3" : "161,137,214",    // purple
         "freeze-6" : "130,104,186",    // purple
         "freeze-9" : "98,70,155",      // purple
         "freeze-12" : "82,53,140",     // purple
     
         // Mixed
         "mixed-1" : "255,160,207",    // red
         "mixed-3" : "224,120,172",    // red
         "mixed-6" : "192,77,134",     // red
         "mixed-9" : "155,25,90",      // red
         "mixed-12" : "146,13,79",     // red
         
         // Snow
         "snow-1" : "138,248,255",    // blue
         "snow-3" : "96,181,191",     // blue
         "snow-6" : "40,93,106",      // blue
         "snow-9" : "13,49,64",       // blue
         "snow-12" : "13,49,64",      // blue

         "black" : "0,0,0,255",
         "white" : "255,255,255,255"
     },
     "out-palette" : {
         "transparent" : "0,0,0,0",
            
         "rain-1" : "99,235,99",       //  1  green
         "rain-2" : "60,198,60",      //  2  green
         "rain-3" : "28,157,52",      //  3  green
         "rain-4" : "14,104,26",      //  4  FColor
         "rain-5" : "0,63,0",         //  5  green
     
         "rain-6" : "251,235,2",      //  6  yellow
         "rain-7" : "238,109,2",      //  7  orange
     
         "rain-8" : "210,11,6",       //  8  red
         "rain-9" : "189,8,4",        //  9  red
         "rain-10" : "169,5,3",       // 10  red
         "rain-11" : "148,2,1",       // 11  red
         "rain-12" : "128,0,0",       // 12  red

         // Freeze
         "freeze-1" : "188,165,240",    // purple
         "freeze-3" : "161,137,214",    // purple
         "freeze-6" : "130,104,186",    // purple
         "freeze-9" : "98,70,155",      // purple
         "freeze-12" : "82,53,140",     // purple
     
         // Mixed
         "mixed-1" : "255,160,207",    // red
         "mixed-3" : "224,120,172",    // red
         "mixed-6" : "192,77,134",     // red
         "mixed-9" : "155,25,90",      // red
         "mixed-12" : "146,13,79",     // red
         
         // Snow
         "snow-1" : "138,248,255",    // blue
         "snow-3" : "96,181,191",     // blue
         "snow-6" : "40,93,106",      // blue
         "snow-9" : "13,49,64",       // blue
         "snow-12" : "13,49,64",      // blue

         "black" : "0,0,0,0",
         "white" : "1,1,1,0"
     },
     "overlay-palette" : {
         "transparent" : "0,0,0,0",
            
         "rain-1" : "0,0,0,0",       //  1  green
         "rain-2" : "0,0,0,0",       //  2  green
         "rain-3" : "0,0,0,0",       //  3  green
         "rain-4" : "0,0,0,0",       //  4  FColor
         "rain-5" : "0,0,0,0",       //  5  green
     
         "rain-6" : "darken1",       //  6  yellow
         "rain-7" : "darken1",       //  7  orange
     
         "rain-8" : "darken1",       //  8  red
         "rain-9" : "darken1",       //  9  red
         "rain-10" : "darken1",      // 10  red
         "rain-11" : "darken1",      // 11  red
         "rain-12" : "darken1",      // 12  red

         // Freeze
         "freeze-1" : "0,0,0,0",     // purple
         "freeze-2" : "0,0,0,0",     // purple
         "freeze-3" : "0,0,0,0",     // purple
         "freeze-4" : "darken1",     // purple
         "freeze-5" : "darken1",     // purple
     
         // Mixed
         "mixed-1" : "0,0,0,0",      // red
         "mixed-2" : "0,0,0,0",      // red
         "mixed-3" : "0,0,0,0",      // red
         "mixed-4" : "darken1",      // red
         "mixed-5" : "darken1",      // red
         
         // Snow
         "snow-1" : "0,0,0,0",       // blue
         "snow-2" : "0,0,0,0",       // blue
         "snow-3" : "0,0,0,0",       // blue
         "snow-4" : "darken1",       // blue
         "snow-5" : "darken1",       // blue

         "black" : "0,0,0,0",
         "white" : "1,1,1,0"
     },
     "overlay-filters" : {
         "darken1" : {
             "rate" : 90,
             "alpha": 100
         },
         "darken2" : {
             "rate" : 80,
             "alpha": 90
         }
     },
     "overlay" : {
         "alpha-multiple" : 99.0,
         "alpha-minimum": 20
     },
     "bottom" : {
         "rgba" : "128,128,128,32"
     }
 }
 
 */


//-------------------------------------------------------------------------------------------------
inline BYTE cnv100To255(float f100) {
    return std::min((unsigned)(f100/100.0f * 255), 255u);
}

//-------------------------------------------------------------------------------------------------
static
PixelFilterCfg makeOverlayFilterCfg(const JsonBase* jPtr) {
    PixelFilterCfg cfg;
    if (jPtr != nullptr) {
        MapList mapList;
        StringList keys;
        jPtr->toMapList(mapList, keys);
        
        // v1
        cfg.startAlpha = cnv100To255(atof(JsonUtil::get(mapList, "alpha", "100")));
        // v2
        cfg.startAlpha = cnv100To255(atof(JsonUtil::get(mapList, "start-alpha", "100")));
        cfg.endAlpha = cnv100To255(atof(JsonUtil::get(mapList, "end-alpha", "0")));
        
        cfg.rate = atof(JsonUtil::get(mapList, "rate", "100"))/100.0f;
    }
    return cfg;
}

//-------------------------------------------------------------------------------------------------
bool ImageCfg::parseConfig(const lstring& cfgName) {
    ifstream in;
    ofstream out;
 
    lstring cfgFilename = cfgName;
    ReplaceAll(cfgFilename, "~", getHomeDir()); // Convert ~ to home directory.
    
    isValid = false;
    try {
        if (stat(cfgFilename, &filestat) == 0) {
            in.open(cfgFilename);
            if (in.good()) {
               
                buffer.resize(filestat.st_size + 1);
                streamsize inCnt = in.read(buffer.data(), buffer.size()).gcount();
                assert(inCnt < buffer.size());
                in.close();
                buffer.push_back('\0');

                JsonUtil::parseJson(buffer, fields);
                in.close();
                
                MapList mapList;
                JsonMap mapJson;
                if (getMapList("overlay-filters", mapJson, "")) {
                    for (auto it=mapJson.cbegin(); it != mapJson.cend(); it++) {
                        overlayFilters[it->first] = makeOverlayFilterCfg(it->second);
                    }
                }
                if (getMapList("overlay", mapList, "alpha-multiple|alpha-minimum")) {
                    overlayCfg.alphaMultiple = atof(JsonUtil::get(mapList, "alpha-multiple", "99")) / 100.0f;
                    overlayCfg.alphaMinimum = cnv100To255(atof(JsonUtil::get(mapList, "alpha-minimum", "0")));
                }
                if (getMapList("bottom", mapList, "rgba")) {
                    bottomCfg.color = FColor(JsonUtil::get(mapList, "rgba", "128,128,128,16"));
                }
                
                isValid = true;
                getOutPalette();
                getOverlayPalette();
            } else {
                cerr << "Config Failed to open:" << cfgFilename << endl;
            }
        } else {
            cerr << "Config Missing or bad path:" << cfgFilename << endl;
        }
    } catch (exception ex) {
        cerr << "Config " << ex.what() << ", Error in file:" << cfgFilename << endl;
    }
 
    return isValid;
}

//-------------------------------------------------------------------------------------------------
void ImageCfg::print(ostream& out) {
    out << fields.toString() << std::endl;
}

//-------------------------------------------------------------------------------------------------
bool ImageCfg::getMapList(const char* id, MapList& mapList, const lstring& validIds ) const {
    const JsonBase* jBasePtr = fields.at("");
    if (jBasePtr != nullptr) {
        const JsonBase* jPtr = jBasePtr->find(id);
        if (jPtr != nullptr) {
            mapList.clear();
            StringList keys;
            jPtr->toMapList(mapList, keys); // TODO - validate key list against validIds
            if (!validIds.empty()) {
                for (auto it = mapList.cbegin(); it != mapList.cend(); it++) {
                    if (validIds.find(it->first) == lstring::npos) {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
bool ImageCfg::getMapList(const char* id, JsonMap& mapList, const lstring& validIds) const {
    const JsonBase* jBasePtr = fields.at("");
    if (jBasePtr != nullptr) {
        const JsonBase* jPtr = jBasePtr->find(id);
        if (jPtr != nullptr && jPtr->mJtype == JsonBase::Jtype::Map) {
            mapList = (*(JsonMap*)jPtr);
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
const FPalette&   ImageCfg::getInPalette()  {
    if (inPalette.empty() && isValid) {
        isValid &= parsePalette("in-palette", inPalette);
    }
    return inPalette;
}

//-------------------------------------------------------------------------------------------------
const FPalette&   ImageCfg::getOutPalette()  {
    if (outPalette.empty() && isValid) {
        if (!parsePalette("out-palette", outPalette, &getInPalette())) {
            outPalette = getInPalette();
            isValid &= outPalette.size() > 0;
        }
    }
    return outPalette;
}

//-------------------------------------------------------------------------------------------------
const FPalette&  ImageCfg::getOverlayPalette()  {
    if (overlayPalette.empty() && isValid) {
        isValid &= parsePalette("overlay-palette", overlayPalette, &getOutPalette());
    }
    return overlayPalette;
}

//-------------------------------------------------------------------------------------------------
bool ImageCfg::parsePalette(const char* PaletteID, FPalette& palette, const FPalette* refPalettePtr)  {
    bool valid = false;
    if (palette.empty()) {
        MapList mapList;
        if (getMapList(PaletteID, mapList, "")) {
            unsigned idx = 0;
            for (auto it = mapList.cbegin(); it != mapList.cend(); ++it, idx++) {
                const lstring& name = it->first;
                const StringList& list = it->second;
                const lstring& item = list.front();
                FColor color;
                std::string msg;
                if (FColor::parse(item, color, msg)) {
                    palette.push_back(color, name);
                } else if (hasOverlayFilter(item) && refPalettePtr != nullptr && refPalettePtr->size() > idx) {
                    const PixelFilterCfg& filterCfg = getOverlayFilter(item);
                    FColor refColor = (*refPalettePtr)[idx];
                    FColor overColor = filter(filterCfg, refColor);
                    palette.push_back(overColor, name);
                } else {
                    valid = false;
                    
                    if ( refPalettePtr->size() <= idx) {
                        std::cerr << "Config - " << PaletteID << " has fewer colors than out-palette\n";
                    } else {
                        std::cerr << "Config - Unknown or poorly formed " << PaletteID << " item:" << item << std::endl;
                    }
                }
            }
            valid = !palette.empty();
        }
    }

    return valid;
}

//-------------------------------------------------------------------------------------------------
FColor ImageCfg::filter(const PixelFilterCfg& cfg, const FColor& inColor) const {
    return FColor::scale(cfg.rate, inColor,cfg.startAlpha);
}

