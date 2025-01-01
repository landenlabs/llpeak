//-------------------------------------------------------------------------------------------------
//  File: ImageUtilF.cpp
//  Desc: Image manipulation utility functions using FreeImage.
//
//  ImageUtilF created by Dennis Lang on 12/21/21.
//  Copyright © 2022 Dennis Lang. All rights reserved.
//
//  FreeImage Design and implementation by
//  - Floris van den Berg (flvdberg@wxs.nl)
//  - Herv<E9> Drolon (drolon@infonie.fr)
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

// Project files
#include "ImageUtilF.hpp"
#include "FPrint.hpp"
#include "FBrush.hpp"
#include "FDraw.hpp"
#include "FBlur.hpp"
#include "FileUtil.hpp"



// C++
#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <vector>
#include <memory>   // unique_ptr, memset, memcpy

// C
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


bool ImageUtilF::initDone = false;


//-------------------------------------------------------------------------------------------------
unsigned DLL_CALLCONV
myReadProc(void* buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned)fread(buffer, size, count, (FILE*)handle);
}

unsigned DLL_CALLCONV
myWriteProc(void* buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned)fwrite(buffer, size, count, (FILE*)handle);
}

int DLL_CALLCONV
mySeekProc(fi_handle handle, long offset, int origin) {
    return fseek((FILE*)handle, offset, origin);
}

long DLL_CALLCONV
myTellProc(fi_handle handle) {
    return ftell((FILE*)handle);
}

//-------------------------------------------------------------------------------------------------
FImage& ImageUtilF::LoadImage(FImage& img, const char* fullname) {
    FILE* inFile = fopen(fullname, "rb");

    if (inFile != NULL) {
        ImageUtilF::init();

        FreeImageIO io;
        io.read_proc = myReadProc;
        io.write_proc = myWriteProc;
        io.seek_proc = mySeekProc;
        io.tell_proc = myTellProc;

        // find the buffer format
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&io, (fi_handle)inFile, 0);

        // const char* fmtStr = FreeImage_GetFIFDescription(fif);
        // std::cout << "File format=" << fmtStr << std::endl;

        if (fif != FIF_UNKNOWN) {
            // load from the file handle
            img.LoadFromHandle(fif, &io, (fi_handle)inFile, 0);
        } else {
            std::cerr << strerror(errno);
            std::cerr << ", Failed to load " << fullname << std::endl;
        }
        
        fclose(inFile);
    }
    return img;
}

//-------------------------------------------------------------------------------------------------
FImage& ImageUtilF::MakeTestI8(FImage& outI8, unsigned width, unsigned height, ImageCfg& cfg) {
    FPalette palette = cfg.getOutPalette();
    
    if (!outI8.Valid()) {
        outI8 = FImage::Create(width, height, 8);
        outI8.setPalette(palette);
    }
    
    outI8.FillImage(palette[0]);
    unsigned nColors = (unsigned)palette.size();
    unsigned marginX = 10;
    unsigned marginY = 10;
    unsigned drawDim = nColors*2;
 
    for (unsigned y = 0; y < drawDim; y++) {
        BYTE* rowI8 = outI8.ScanLine(marginY + y);
        unsigned clrY = (y < nColors) ? y : nColors*2 - y -1;
        for (unsigned x = 0; x < drawDim; x++) {
            unsigned clrX = (x < nColors) ? x : nColors*2 - x -1;
            rowI8[marginX + x] = (BYTE)std::min(clrX, clrY);
        }
    }
   
    marginY += drawDim + 10;
    unsigned lineLen = 20;
    for (unsigned y = 0; y < nColors; y++) {
        BYTE* rowI8 = outI8.ScanLine(marginY + y*4);
        for (unsigned x = 0; x < lineLen; x++) {
            rowI8[marginX + x] = y;
        }
    }
    
    unsigned padX = 10;
    for (unsigned y = 0; y < nColors; y++) {
        for (unsigned h = 0; h < 4; h++) {
            BYTE* rowI8 = outI8.ScanLine(marginY + y*4 + h);
            for (unsigned x = 0; x < 4; x++) {
                rowI8[marginX + x + lineLen + padX + y*3] = y;
            }
        }
    }
    
    saveTo(outI8, "testI8.png");
    return outI8;
}

//-------------------------------------------------------------------------------------------------
bool ImageUtilF::saveTo(const FImage& out, const char* toName, bool verbose) {
    bool okay = false;
    
    // Get output format from the file name or file extension
    FREE_IMAGE_FORMAT out_fif = FreeImage_GetFIFFromFilename(toName);
    if (out_fif != FIF_UNKNOWN) {
        okay = FreeImage_Save(out_fif, out.imgPtr, toName, 0);
        if (verbose) {
            if (okay) {
                std::cout << "Saved " << toName << std::endl;
            } else {
                std::cerr << strerror(errno);
                std::cerr << ", FAILED saving " << toName << std::endl;
            }
        }
    } else {
        std::cerr << "Save ignored, unknown format " << toName << std::endl;
    }
    
    return okay;
}

//-------------------------------------------------------------------------------------------------
bool ImageUtilF::threadSaveAndCloseTo(const FImage& cimg, const char* toName, ImageAux& aux) {
    FImage& img = (FImage&)cimg;
#ifdef USE_THREAD
    if (aux.useThread) {
        return aux.threadSaveImage.StartThread(img, toName, &aux);
    }
#endif
    bool okay = saveTo(img, toName, aux.verbose);
    img.Close();
    return okay;
}

//-------------------------------------------------------------------------------------------------
// class used when montaging (merging) collection of image tiles.
class TileInfo {
public:
    unsigned width;
    unsigned height;
    unsigned colors;
    unsigned bitsPerPixel;
    FREE_IMAGE_COLOR_TYPE type;
    // FImageRef imgRef;
    lstring name;
    unsigned xTile;
    unsigned yTile;
};

//-------------------------------------------------------------------------------------------------
// Merge multiple imput image tiles into single larger output image.
bool ImageUtilF::Montage(
       const FPalette& inPalette,
       StringList inPaths, int xTiles, int yTiles,
       const lstring& outputPath) {
    bool okay = true;
    
    FImage outI8;
    std::vector<TileInfo> imageSet(inPaths.size());
    FPalette outPalette(inPalette);
    FPalette tilePalette;
    
    unsigned idx = 0;
    for (const lstring& fullname : inPaths) {
        FImage img;
        if (LoadImage(img, fullname).Valid()) {
            FREE_IMAGE_COLOR_TYPE imgType = img.GetColorType();
            TileInfo& imgInfo = imageSet[idx];
            imgInfo.width = img.GetWidth();
            imgInfo.height = img.GetHeight();
            imgInfo.colors = img.GetColorsUsed();
            imgInfo.bitsPerPixel = img.GetBitsPerPixel();
            imgInfo.type = imgType;
            imgInfo.name = fullname;
            imgInfo.xTile = idx % xTiles;
            imgInfo.yTile = idx / xTiles;
            
            if (outPalette.size() == 0) {
                img.getPalette(outPalette);
            }
            if (idx != 0) {
                if (imgInfo.bitsPerPixel != imageSet[0].bitsPerPixel) {
                    std::cerr << fullname << " has different bpp " <<  imgInfo.bitsPerPixel
                        << " than first image " << imageSet[0].bitsPerPixel << std::endl;
                    okay = false;
                }
                if (imgInfo.type != imageSet[0].type) {
                    std::cerr << fullname << " has different type " << FPrint::toString(imgInfo.type)
                        << " than first image " << FPrint::toString(imageSet[0].type) << std::endl;
                    okay = false;
                }
                if (imgInfo.width != imageSet[0].width) {
                    std::cerr << fullname << " has different width " << imgInfo.width
                        << " than first image " << imageSet[0].width << std::endl;
                    okay = false;
                }
                if (imgInfo.height != imageSet[0].height) {
                    std::cerr << fullname << " has different height " << imgInfo.height
                        << " than first image " << imageSet[0].height << std::endl;
                    okay = false;
                }
                
                img.getPalette(tilePalette);
                outPalette.merge(tilePalette);
            }
            idx++;
      
            
            img.Close();
        }
    }
    
    if (!okay || imageSet.empty()) {
        std::cerr << "Montage ignored\n";
        return false;
    }
    
    
    yTiles = (yTiles > 0) ? yTiles : ((unsigned)imageSet.size() + xTiles-1) / xTiles;
    unsigned outWidth = imageSet[0].width * xTiles;
    unsigned outHeight = imageSet[0].height * yTiles;
  
    unsigned tileWidth = outWidth / xTiles;
    unsigned tileHeight = outHeight / yTiles;
  
    unsigned bitsPerPixel = imageSet[0].bitsPerPixel;
    unsigned tileByteWidth=0;
    
    FImage* imgPtr = FImage::Allocate(outWidth, outHeight, bitsPerPixel);
    FImageRef outRef(imgPtr);
    
    bool initOut = true;
    PalMapping tileMapping;
 
    for (TileInfo& imageInfo : imageSet) {
        FImage imgTile;
        if (LoadImage(imgTile, imageInfo.name).Valid()) {
            tileByteWidth = imgTile.GetBytesPerLine();
            bool doMapping = false;
            if (initOut) {
                outRef->FillImage(FColor::TRANSPARENT);
                outRef->setPalette(outPalette);
                initOut = false;
            } else {
                if (imageInfo.type == FIC_PALETTE && imageInfo.bitsPerPixel == 8) {
                    imgTile.getPalette(tilePalette);
                    tileMapping = FPalette::getMapping(outPalette, tilePalette);
                    doMapping = (tileMapping.shiftCnt != 0);
                }
            }
          
            if (doMapping) {
                for (unsigned y = 0; y < tileHeight; y++) {
                    const BYTE* inRow = imgTile.ReadScanLine(y);
                    BYTE* outRow = outRef->ScanLine(y + imageInfo.yTile * tileHeight);
                    outRow += imageInfo.xTile * tileWidth;
                    for (unsigned x = 0; x < tileWidth; x++) {
                        outRow[x] = tileMapping.to[inRow[x]];
                    }
                }
            } else {
                for (unsigned y = 0; y < tileHeight; y++) {
                    const BYTE* inRow = imgTile.ReadScanLine(y);
                    BYTE* outRow = outRef->ScanLine(y + imageInfo.yTile * tileHeight);
                    memcpy(outRow + imageInfo.xTile * tileByteWidth, inRow, tileByteWidth);
                }
            }
            
            imgTile.Close();
        }
    }
    
    okay = okay && saveTo(outRef, outputPath, true);
    // outRef->Close();
    return okay;
}

//-------------------------------------------------------------------------------------------------
// Truecolor 32bit blend,  top is blended over bottom.  Bottom can also be output.
FImage& ImageUtilF::BlendP32(const FImage& topImgP32, const FImage& botImgP32, FImage& outImgP32) {
    unsigned botImgBPP = botImgP32.GetBitsPerPixel();
    if (botImgBPP != 32) {
        std::cerr << "Blend - Bottom image not 32bit" << std::endl;
        return outImgP32;
    }

    unsigned topImgBPP = topImgP32.GetBitsPerPixel();
    if (topImgBPP != 32) {
        std::cerr << "Blend - Top image not 32bit" << std::endl;
        return outImgP32;
    }

    if (sizeof(FColor) != 4) {
        std::cerr << "Bug with color size\n";
    }

    unsigned widthTop = topImgP32.GetWidth();
    unsigned heightTop = topImgP32.GetHeight();
    unsigned widthBot = botImgP32.GetWidth();
    unsigned heightBot = botImgP32.GetHeight();
    unsigned height = min(heightTop, heightBot);
    unsigned width = min(widthTop, widthBot);

    if (&botImgP32 == &outImgP32) {
        for (unsigned y = 0; y < height; y++) {
            const FColor* top_argb = (const FColor*)topImgP32.ReadScanLine(y);
            // const FColor* bot_argb = (FColor*)botImgP32.ReadScanLine(y);
            FColor* out_argb =  (FColor*)outImgP32.ScanLine(y);
             
            for (unsigned x = 0; x < width; x++) {
                FColor& botColor = out_argb[x];
                top_argb[x].blendOver(botColor);
            }
        }
    } else {
        for (unsigned y = 0; y < height; y++) {
            const FColor* top_argb = (const FColor*)topImgP32.ReadScanLine(y);
            const FColor* bot_argb = (const FColor*)botImgP32.ReadScanLine(y);
            FColor* out_argb = (FColor*)outImgP32.ScanLine(y);

            for (unsigned x = 0; x < width; x++) {
                FColor botColor = *bot_argb++;
                top_argb[x].blendOver(botColor);
                out_argb[x] = botColor;
            }
        }
    }

    return outImgP32;
}

//-------------------------------------------------------------------------------------------------
// Index 8bit palette blended over 32bit bottom.
FImage& ImageUtilF::BlendI8_P32(const FPalette& topPalette, const FImage& topImgI8, FImage& botImgP32) {
    unsigned widthTop = topImgI8.GetWidth();
    unsigned heightTop = topImgI8.GetHeight();
    unsigned widthBot = botImgP32.GetWidth();
    unsigned heightBot = botImgP32.GetHeight();

    unsigned height = min(heightTop, heightBot);
    unsigned width = min(widthTop, widthBot);

    for (unsigned y = 0; y < height; y++) {
        const BYTE* top = topImgI8.ReadScanLine(y);
        RGBQUAD* bot = (RGBQUAD*)botImgP32.ScanLine( y);
        for (unsigned x = 0; x < width; x++) {
            const FColor& topColor = topPalette[top[x]];
            topColor.blendOver(bot[x]);
        }
    }

    return botImgP32;
}

//-------------------------------------------------------------------------------------------------
// Index 8bit palette blended over 8bit, output to 32bit .
FImage& ImageUtilF::BlendI8_P32(const FImage& topImgI8, const FImage& botImgI8, FImage& outImgP32) {
    FPalette topPalette, botPalette;
    topImgI8.getPalette(topPalette);
    botImgI8.getPalette(botPalette);
    
    unsigned widthTop = topImgI8.GetWidth();
    unsigned heightTop = topImgI8.GetHeight();
    unsigned widthBot = botImgI8.GetWidth();
    unsigned heightBot = botImgI8.GetHeight();

    unsigned height = min(heightTop, heightBot);
    unsigned width = min(widthTop, widthBot);

    for (unsigned y = 0; y < height; y++) {
        const BYTE* top = topImgI8.ReadScanLine(y);
        const BYTE* bot = botImgI8.ReadScanLine(y);
        RGBQUAD* out = (RGBQUAD*)outImgP32.ScanLine( y);
        for (unsigned x = 0; x < width; x++) {
            const FColor& topColor = topPalette[top[x]];
            FColor botColor = botPalette[bot[x]];
            topColor.blendOver(botColor);
            out[x] = botColor;
        }
    }

    return outImgP32;
}


//-------------------------------------------------------------------------------------------------
// Truecolor blended over 8bit palette and saved in 32bit output
FImage& ImageUtilF::BlendP32_I8(const FImage& topImgP32, const FImage& botImgI8, FImage& outImgP32) {
    unsigned widthTop = topImgP32.GetWidth();
    unsigned heightTop = topImgP32.GetHeight();
    unsigned widthBot = botImgI8.GetWidth();
    unsigned heightBot = botImgI8.GetHeight();

    unsigned height = min(heightTop, heightBot);
    unsigned width = min(widthTop, widthBot);
    
    FPalette botPalette;
    botImgI8.getPalette(botPalette);

    for (unsigned y = 0; y < height; y++) {
        const FColor* top_argb = (const FColor*)topImgP32.ReadScanLine(y);
        const BYTE*   bot      = botImgI8.ReadScanLine(y);
        FColor*       out_argb = (FColor*)outImgP32.ScanLine(y);

        for (unsigned x = 0; x < width; x++) {
            FColor botColor = botPalette[bot[x]];
            top_argb[x].blendOver(botColor);
            out_argb[x] = botColor;
        }
    }

    return outImgP32;
}


//-------------------------------------------------------------------------------------------------
// Output is maximizing pixel index, output = max(input, output)
FImage& ImageUtilF::MaximumI8(const FImage& inImgI8, FImage& outImgI8) {
    unsigned widthIn   = inImgI8.GetWidth();
    unsigned heightIn  = inImgI8.GetHeight();
    unsigned widthOut  = outImgI8.GetWidth();
    unsigned heightOut = outImgI8.GetHeight();

    unsigned height    = min(heightIn, heightOut);
    unsigned width     = min(widthIn, widthOut);

    for (unsigned y = 0; y < height; y++) {
        const BYTE* in = inImgI8.ReadScanLine(y);
        BYTE* out = outImgI8.ScanLine(y);
        for (unsigned x = 0; x < width; x++) {
            out[x] = max(in[x], out[x]);   // Output is maximum pixel index.
        }
    }

    return outImgI8;
}

//-------------------------------------------------------------------------------------------------
// Dump image information, palette colors, generate PNG of palette, pixel histogram usage.
void ImageUtilF::Dump(const lstring&  fullname) {
    FImage img;
    if (LoadImage(img, fullname).Valid()) {
        FPrint::printInfo(img, fullname);
        FPrint::printPalette(img);
        FPrint::printHisto(img);
    }
}

//-------------------------------------------------------------------------------------------------
// Shade (darken/lighten) pixels based on slope derived from pixle index value.
bool ImageUtilF::ShadeI8(const lstring& fullname, ImageCfg& cfg, ImageAux& aux, FImage& imgI8) {

    lstring fullPath(fullname);
    lstring outNameExtn;
    FileUtil::getName(outNameExtn, fullPath);
    
    unsigned width = imgI8.GetWidth();
    unsigned height = imgI8.GetHeight();
    
    // Order palette
    FPalette inPalette;
    imgI8.getPalette(inPalette);
  
    if (!aux.shadeMap.isReady) {
        const FPalette& outPalette = cfg.getOutPalette();
        FPalette tmpPalette(outPalette);
        tmpPalette.remove(FColor::BLACK).remove(FColor::WHITE).remove(FColor::TRANSPARENT);
        FPalette spreadPalette;
        tmpPalette.spread(spreadPalette, (unsigned)tmpPalette.size(), 256);

        spreadPalette.insert(spreadPalette.begin(), FColor::TRANSPARENT);
        spreadPalette.push_back(FColor::BLACK);
        spreadPalette.push_back(FColor::WHITE);
        aux.shadeMap = FPalette::getMapping(inPalette, spreadPalette);
    }
    
    // imgI8.ApplyPaletteIndexMapping(aux.shadeMap.from, aux.shadeMap.to, colors, false);
    // imgI8.setPalette(outPalette);

    // Shade pixels
    FImage* imgPtrP32 = FImage::Allocate(width, height, 32, 0xff0000, 0xf00, 0xff);
    FImageRef imgP32(imgPtrP32);
    
    aux.shadeRef->shadeI8_P32(inPalette, imgI8, imgP32, cfg, aux);
    
    ImageUtilF::threadSaveAndCloseTo(imgP32, aux.outPath + outNameExtn, aux);
    // imgP32->Close();

    imgI8.Close();
    return true;
}

//-------------------------------------------------------------------------------------------------
// Shade (darken/lighten) pixels based on slope derived from pixle value (assumed gray colors)
bool ImageUtilF::ShadeP32(const lstring& fullname, ImageCfg& cfg, ImageAux& aux, FImage& imgP32) {
 
    lstring fullPath(fullname);
    lstring outNameExtn;
    FileUtil::getName(outNameExtn, fullPath);
    
    aux.shadeRef->shadeP32(imgP32, imgP32, cfg, aux);
    ImageUtilF::threadSaveAndCloseTo(imgP32, aux.outPath + outNameExtn, aux);

    return true;
}

//-------------------------------------------------------------------------------------------------
// Shade (darken/lighten) pixels based on slope derived from pixle index value.
bool ImageUtilF::Shade(const lstring& fullname, ImageCfg& cfg, ImageAux& aux) {
    FImage img;
    if (cfg.isValid &&  LoadImage(img, fullname).Valid()) {
        
        unsigned bitsPerPixel = img.GetBitsPerPixel();
        switch (bitsPerPixel) {
            case 8:
                return ShadeI8(fullname, cfg, aux, img);
            case 32:
                return ShadeP32(fullname, cfg, aux, img);
                break;
            default:
                std::cerr << "Unsupported bit depth, must be 8 or 32 not " << bitsPerPixel << std::endl;
                return false;
        }
    } else {
        std::cerr << "Shade failed bad cfg or failed to open " << fullname << std::endl;
        return false;
    }
}

//-------------------------------------------------------------------------------------------------
bool ImageUtilF::BlurI8(const lstring& fullname, ImageCfg& cfg, ImageAux& aux, const FImage& inI8) {
    lstring fullPath(fullname);
    lstring outNameExtn;
    FileUtil::getName(outNameExtn, fullPath);
    
    unsigned width = inI8.GetWidth();
    unsigned height = inI8.GetHeight();
    FPalette inPalette;
    inI8.getPalette(inPalette);
    const FPalette& outPalette = cfg.getOutPalette();
    unsigned radius = 2;
    
    PalMapping mapping = FPalette::getMapping(inPalette, outPalette);
    FImage outP32 = FImage::Create(width, height);
    FBlur::blurI8(mapping, outPalette, inI8, outP32, cfg, aux, radius);

    ImageUtilF::threadSaveAndCloseTo(outP32, aux.outPath + outNameExtn, aux);
    return true;
}

//-------------------------------------------------------------------------------------------------
// Shade (darken/lighten) pixels based on slope derived from pixle index value.
bool ImageUtilF::Blur(const lstring& fullname, ImageCfg& cfg, ImageAux& aux) {
    FImage img;
    if (cfg.isValid
        // && MakeTestI8(img, 300, 300, cfg).Valid()
        && LoadImage(img, fullname).Valid()
        ) {
        
        unsigned bitsPerPixel = img.GetBitsPerPixel();
        switch (bitsPerPixel) {
            case 8:
                return BlurI8(fullname, cfg, aux, img);
            case 32:
            default:
                std::cerr << "Unsupported bit depth, must be 8 not " << bitsPerPixel << std::endl;
                return false;
        }
    } else {
        std::cerr << "Blur failed bad cfg or failed to open " << fullname << std::endl;
        return false;
    }
}


//-------------------------------------------------------------------------------------------------
// Find image color in reference palette and copy matched slot from mapping color to output
static unsigned  MapColors(const FPalette& imgPal, const FPalette& refPal, const FPalette& mapPal, FPalette& outPal) {
    unsigned matchCnt = 0;
    // outPal = imgPal;
    
    for (int idx = 0; idx < imgPal.size(); idx++) {
        const FColor& imgColor = imgPal[idx];
        unsigned matchIdx = refPal.findColor(imgColor);
        if (matchIdx != FPalette::NO_MATCH && outPal[idx] != mapPal[matchIdx]) {
            outPal[idx] = mapPal[matchIdx];
            matchCnt++;
        }
    }
    
    return matchCnt;
}

//-------------------------------------------------------------------------------------------------
// Repeat input image with overlay multiple times, fading overlay.
bool ImageUtilF::BlendFade(const lstring& fullname, unsigned extraFrames, ImageCfg& cfg, ImageAux& aux) {
    
    if (aux.overlayImgRef == nullptr) {
        std::cerr << "Unable to Fade without an overlay for " << fullname << std::endl;
    }
    
    FImage imgI8;
    if (LoadImage(imgI8, fullname).Valid()) {
        lstring fullPath(fullname);
        lstring outFname;
        FileUtil::getName(outFname, fullPath);
        lstring extn;
        DirUtil::getExt(extn, outFname);
        lstring fname;
        DirUtil::removeExtn(fname, outFname);
        
        unsigned bitsPerPixel = imgI8.GetBitsPerPixel();
        if (bitsPerPixel != 8) {
            FPrint::printInfo(imgI8, fullname);
            std::cerr << fullname << " must by 8 bit per pixel images\n";
            return false;
        }

        lstring outFullpath = aux.outPath + outFname;
        int status = unlink(outFullpath);
        if (status != 0) {
            std::cerr << "Failed to delete " << outFullpath << " Reason:" << strerror(errno) << std::endl;
        } else {
            std::cout << "Removed " << outFullpath << std::endl;
        }
        
        float alphaMultiple = cfg.overlayCfg.alphaMultiple;
        char outName[256];
        
        FPalette srcPalette;
        imgI8.getPalette(srcPalette);
        if (MapColors(srcPalette, cfg.getInPalette(), cfg.getOutPalette(), srcPalette) != 0) {
            imgI8.setPalette(srcPalette);
        }
        imgI8.SetBackgroundColor(FColor::TRANSPARENT);
        
        for (unsigned frameIdx = 0; frameIdx < extraFrames; frameIdx++) {
            FImage imgP32 = imgI8.ConvertTo32Bits();
            
            aux.overlayImgRef->AdjustAlphaP32(alphaMultiple);
            BYTE alpha = FColor::clamp(255 * alphaMultiple * (extraFrames - frameIdx)/extraFrames);
            aux.overlayImgRef->MinAlphaP32(alpha);
            ImageUtilF::BlendP32(aux.overlayImgRef, imgP32, imgP32);
        
            if (aux.doBottom) {
            //    BYTE alpha = FColor::clamp(cfg.bottomCfg.color.rgbReserved * (extraFrames - frameIdx)/extraFrames);
            //    aux.bottomImgRef->MinAlphaI8(alpha);
                ImageUtilF::BlendP32_I8(imgP32, aux.bottomImgRef, imgP32);
            }
            
            snprintf(outName, sizeof(outName), "%s-%03d.%s", fname.c_str(), frameIdx, extn.c_str());
            
            ImageUtilF::threadSaveAndCloseTo(imgP32, outName, aux);
            // imgP32.Close();
        }
        
        // Save last Fade frame with no overlay and background 50% reduced.
        FImage imgP32 = imgI8.ConvertTo32Bits();
        if (aux.doBottom) {
            BYTE alpha = FColor::clamp(cfg.bottomCfg.color.rgbReserved/2);
            aux.bottomImgRef->MinAlphaI8(alpha);
            ImageUtilF::BlendP32_I8(imgP32, aux.bottomImgRef, imgP32);
        }
        snprintf(outName, sizeof(outName), "%s-%03d.%s", fname.c_str(), extraFrames, extn.c_str());
        ImageUtilF::threadSaveAndCloseTo(imgP32, outName, aux);
        // imgP32.Close();
        
        imgI8.Close();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
void ImageUtilF::BlendI8(const lstring& fullPath, ImageCfg& cfg, ImageAux& aux, FImage& imgI8) {
    lstring outFname;
    FileUtil::getName(outFname, fullPath);
    
    unsigned width = imgI8.GetWidth();
    unsigned height = imgI8.GetHeight();
    unsigned colors = imgI8.GetColorsUsed();
    
    FPalette srcPalette;
    imgI8.getPalette(srcPalette);

    const FPalette& dstPalette = cfg.getOutPalette();
    const FPalette& overlayPalette = cfg.getOverlayPalette();

    if (MapColors(srcPalette, cfg.getInPalette(), cfg.getOutPalette(), srcPalette) != 0) {
        imgI8.setPalette(srcPalette);
    }
    imgI8.SetBackgroundColor(FColor::TRANSPARENT);
    
    // --- Step 1 - blend Overlay layer, Image and Bottom layer and save output image frame.
    FImage imgP32 = imgI8.ConvertTo32Bits();
    if (aux.overlayImgRef != nullptr) {
        aux.overlayImgRef->AdjustAlphaP32(cfg.overlayCfg.alphaMultiple, cfg.overlayCfg.alphaMinimum);
        switch (cfg.overlayerOrder) {
            case ImageCfg::OVER_IMAGE:
                ImageUtilF::BlendP32(aux.overlayImgRef, imgP32, imgP32);
                break;
            case ImageCfg::UNDER_IMAGE:
                ImageUtilF::BlendP32(imgP32, aux.overlayImgRef, imgP32);
                break;
        }
 
        if (aux.doBottom) {
            ImageUtilF::BlendP32_I8(imgP32, aux.bottomImgRef, imgP32);
        }
    }
 
    ImageUtilF::threadSaveAndCloseTo(imgP32, aux.outPath + outFname, aux);
    // imgP32.Close();

    // --- Step 2 - create/update overlay with selected colorized pixels.
    // Source palette can change, always generate new mapping.
    aux.overlayMap = FPalette::getMapping(srcPalette, dstPalette);

    imgI8.ApplyPaletteIndexMapping(aux.overlayMap.from, aux.overlayMap.to, colors, false);
    imgI8.setPalette(overlayPalette);
    
    if (aux.overlayImgRef == nullptr) {
        FImage* imgPtrP32 = FImage::Allocate(width, height, 32, 0xff0000, 0xf00, 0xff);
        FImageRef imgRef(imgPtrP32);
        aux.overlayImgRef.swap(imgRef);
        aux.overlayImgRef->FillImage(FColor::TRANSPARENT);
    }
    BlendI8_P32(overlayPalette, imgI8, aux.overlayImgRef);
    
    // --- Step 3 - create/update bottom coverage layer.
    if (aux.doBottom) {
        // Create bottom (coverage) layer
        if (aux.bottomImgRef == nullptr) {
            FImage* imgPtrI8 = FImage::Allocate(width, height, 8, 0xff0000, 0xf00, 0xff);
            FImageRef imgRef(imgPtrI8);
            aux.bottomImgRef.swap(imgRef);
            aux.bottomImgRef->FillImage(FColor::TRANSPARENT);
            aux.bottomImgRef->setPalette(aux.bottomPalette);
        }
        imgI8.setPalette(aux.bottomPalette);
        MaximumI8(imgI8, aux.bottomImgRef);
    }
}

//-------------------------------------------------------------------------------------------------
bool  FilterImage(const FImage& inImg, FImage& outP32) {
    unsigned width = inImg.GetWidth();
    unsigned height = inImg.GetHeight();
    unsigned bytesPerRow = inImg.GetBytesPerLine();
    unsigned bitsPerPixel = inImg.GetBitsPerPixel();
    unsigned byteWidth = width * bitsPerPixel/8;
    
    if (byteWidth > bytesPerRow) {
        std::cerr << "Filter ignored\n";
        return false;
    }
    
    if (bitsPerPixel == 32) {
        BYTE* colFlag = new BYTE[width];
        for (unsigned y = 0; y < height; y++) {
            const BYTE* inPtr = inImg.ReadScanLine(y);
            BYTE* outPtr = outP32.ScanLine(y);
            for (unsigned x = 0; x < width; x++) {
                BYTE a = *inPtr++;
                BYTE b = *inPtr++;
                BYTE g = *inPtr++;
                BYTE r = *inPtr++;
                bool match = false;
                // if (g > b*2 && g < b*4 && r > b*5) {
                if (g > b*2 && r > b*3) {
                    match = true;
                    colFlag[x]++;
                }
                if (match) {
                    *outPtr++ = a;
                    *outPtr++ = b;
                    *outPtr++ = g;
                    *outPtr++ = r;
                } else {
                    *outPtr++ = 0;
                    *outPtr++ = 0;
                    *outPtr++ = 0;
                    *outPtr++ = 0;
                    /*
                    *outPtr++ = 10;
                    *outPtr++ = b/2;
                    *outPtr++ = g/2;
                    *outPtr++ = r/2;
                     */
                }
            }
        }
        delete []colFlag;
    }
    
    return true;
}

//-------------------------------------------------------------------------------------------------
// Perform Sequence Image Blend on 24 or 32 bit images. 
void ImageUtilF::BlendP32(const lstring& fullPath, ImageCfg& cfg, ImageAux& aux, FImage& inImg) {
    lstring outFname;
    FileUtil::getName(outFname, fullPath);
    
    FImage imgP32 = inImg.ConvertTo32Bits();
    unsigned width = imgP32.GetWidth();
    unsigned height = imgP32.GetHeight();
    
    // --- Step 1 - blend Overlay layer, Image and Bottom layer and save output image frame.
    FImage imgOut = imgP32.Clone();
    FilterImage(imgOut, imgP32);
    if (aux.overlayImgRef != nullptr) {
        imgOut.AdjustAlphaP32(0.3f);
        aux.overlayImgRef->AdjustAlphaP32(cfg.overlayCfg.alphaMultiple, cfg.overlayCfg.alphaMinimum);
        ImageUtilF::BlendP32(aux.overlayImgRef, imgOut, imgOut);

        if (aux.doBottom) {
            ImageUtilF::BlendP32_I8(imgOut, aux.bottomImgRef, imgOut);
        }
    }
    
    ImageUtilF::threadSaveAndCloseTo(imgOut, aux.outPath + outFname, aux);
    // imgOut.Close();
    
    // --- Step 2 - create/update overlay with selected colorized pixels
    // See FilterImage above.
    if (aux.overlayImgRef == nullptr) {
        FImage* imgPtrP32 = FImage::Allocate(width, height, 32, 0xff0000, 0xf00, 0xff);
        FImageRef imgRef(imgPtrP32);
        aux.overlayImgRef.swap(imgRef);
        aux.overlayImgRef->FillImage(FColor::TRANSPARENT);
    }
    BlendP32(imgP32, aux.overlayImgRef, aux.overlayImgRef);
    
    // --- Step 3 - create/update bottom coverage layer.
    if (aux.doBottom) {
        // Create bottom (coverage) layer
        if (aux.bottomImgRef == nullptr) {
            FImage* imgPtrI8 = FImage::Allocate(width, height, 8, 0xff0000, 0xf00, 0xff);
            FImageRef imgRef(imgPtrI8);
            aux.bottomImgRef.swap(imgRef);
            aux.bottomImgRef->FillImage(FColor::TRANSPARENT);
            aux.bottomImgRef->setPalette(aux.bottomPalette);
        }
        imgP32.setPalette(aux.bottomPalette);
        MaximumI8(imgP32, aux.bottomImgRef);
    }

}

//-------------------------------------------------------------------------------------------------
// Perform "Sequenced Image Blend" action
// Every input image is output with two additional layers merged:
//    overlay =  historical path of selected pixels
//    bottom  =  historical coverage of selected pixels
bool ImageUtilF::Blend(const lstring& fullname, ImageCfg& cfg, ImageAux& aux) {
    FImage img;
    if (LoadImage(img, fullname).Valid()) {
        lstring fullPath(fullname);
     
        unsigned bitsPerPixel = img.GetBitsPerPixel();
        switch (bitsPerPixel) {
            case 8:
                BlendI8(fullname, cfg, aux, img);
                break;
            case 24:
            case 32:
                BlendP32(fullname, cfg, aux, img);
                break;
            default:
                // FPrint::printInfo(img, fullname);
                std::cerr << fullname << " must by 8 bit per pixel for Blend\n";
                return false;
        }
        
        img.Close();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
bool ImageUtilF::ToGray(const lstring& fullPath, ImageCfg& cfg, ImageAux& aux) {
    lstring nameExtn;
    FileUtil::getName(nameExtn, fullPath);

    bool okay = false;
    FImage imgP32;
    if (LoadImage(imgP32, fullPath).Valid()) {
        unsigned width = imgP32.GetWidth();
        unsigned height = imgP32.GetHeight();
        
        unsigned bpp = imgP32.GetBitsPerPixel();
        if (bpp == 32) {
            FImage outI8 = FImage::Create( width,  height, 8);
            outI8.SetTransparent(true);
            outI8.setPalette(cfg.getOutPalette());
           
            FREE_IMAGE_COLOR_TYPE clrType = outI8.GetColorType();
            if (clrType != FIC_PALETTE && clrType != FIC_MINISBLACK) {
                std::cerr << FPrint::toString(clrType) << ", Unable to make 8bit palette output image \n";
                return false;
            }
            
            FPalette outPalette = cfg.getOutPalette();
            outI8.setPalette(outPalette);
             
            outPalette.clear();
            for (unsigned clr = 0; clr < 256; clr++) {
                unsigned alpha = std::min(255u, clr*2);
                outPalette.push_back(FColor(clr, clr, clr, alpha));
            }
            outPalette[0] = FColor::TRANSPARENT;
            
            for (unsigned y = 0; y < height; y++) {
                const FColor* inRow = (const FColor*)imgP32.ReadScanLine( y);
                BYTE* outRow = outI8.ScanLine(y);
                for (unsigned x = 0; x < width; x++) {
                    const FColor& inColor = inRow[x];
                    // TODO - confirm color is gray
                    // TODO - map to output palette
                    outRow[x] = inColor.rgbRed;
                    // outPalette[x] = inColor;
                }
            }
            
            // outPalette[0] = FColor::TRANSPARENT;
            outI8.setPalette(outPalette);
            okay = threadSaveAndCloseTo(outI8, aux.outPath + nameExtn, aux);
        }
        imgP32.Close();
    }
    return okay;
}


//-------------------------------------------------------------------------------------------------
void ImageUtilF::ColorlapseI8(const lstring& fullPath, ImageCfg& cfg, ImageAux& aux, FImage& imgI8) {
    lstring nameExtn;
    FileUtil::getName(nameExtn, fullPath);
    lstring extn;
    DirUtil::getExt(extn, nameExtn);
    lstring justname;
    DirUtil::removeExtn(justname, nameExtn);
    char outName[256];
    
    FPalette srcPalette;
    imgI8.getPalette(srcPalette);
    const FPalette& outPalette = cfg.getOutPalette();
    unsigned width = imgI8.GetWidth();
    unsigned height = imgI8.GetHeight();
    unsigned colorCnt = imgI8.GetColorsUsed();
    
    typedef std::vector<unsigned> UArray;
    UArray outPalMap[256];
    unsigned mapCnt = 0;
    BYTE MIN_CLR = 0x10;
    
    // TODO - compute histogram, ignore rare colors.
    for (int idx = 0; idx < srcPalette.size(); idx++) {
        const FColor& srcColor = srcPalette[idx];
        if (srcColor.maxClr() > MIN_CLR) {
            unsigned matchIdx = outPalette.findClosest(srcColor);
            if (matchIdx != FPalette::NO_MATCH) {
                outPalMap[matchIdx].push_back(idx);
                mapCnt++;
            }
        }
    }
    
    // imgI8.SetTransparent(true);
    // imgI8.SetBackgroundColor(FColor::TRANSPARENT);
    if (!aux.colorizeImg.Valid()) {
        aux.colorizeImg = imgI8.Clone();
    }
    if (aux.colorizeImg.Valid()) {
        FPalette clrPalette;
        aux.colorizeImg.getPalette(clrPalette);
        for (unsigned idx = 0; idx < colorCnt; idx++) {
            FColor& color = clrPalette[idx];
            clrPalette[idx] = FColor::darken(0.8f, color, color.rgbReserved);
        }
        clrPalette[0] = FColor::TRANSPARENT;
        aux.colorizeImg.setPalette(clrPalette);
    }
    

    RGBQUAD* colors = imgI8.Palette();
    colors[0] = srcPalette[0] = FColor::TRANSPARENT;
    BYTE* transparentPtr = imgI8.TransparencyTable();
    
    for (unsigned idx = 0; idx < colorCnt; idx++) {
        colors[idx].rgbReserved = 0;
        transparentPtr[idx] = 0;
    }
    
    // ****
    // TODO - thread this for-loop
    // ****
    
    unsigned outIdx = 0;
    for (unsigned idx = 0; idx < outPalette.size(); idx++) {
        // Show palette colors in "output palette" order
        if (outPalMap[idx].size() > 0 && outPalette[idx].rgbReserved != 0 && outPalette[idx].maxClr() > MIN_CLR) {
            UArray& indexes = outPalMap[idx];
            
            // Alpha blend  63, 127, 191, 255
            for (unsigned alpha = 63; alpha < 256; alpha += 64) {
                for (unsigned mapIdx = 0; mapIdx < indexes.size(); mapIdx++) {
                    unsigned clrIdx = indexes[mapIdx];
                    if (srcPalette[clrIdx].maxClr() > MIN_CLR) {
                       // colors[clrIdx].rgbReserved = alpha;
                        transparentPtr[clrIdx] = alpha;
                    }
                }
                imgI8.SetTransparencyTable(transparentPtr, colorCnt);
                
                FImage outP32 = FImage::Create( width,  height, 32);
                BlendI8_P32(imgI8, aux.colorizeImg, outP32);
                snprintf(outName, sizeof(outName), "%s-%04d.%s", justname.c_str(), outIdx++, extn.c_str());
                threadSaveAndCloseTo(outP32, aux.outPath + outName, aux);
                // ImageUtilF::saveTo(outP32, aux.outPath + outName, true);
                // outP32.Close();
            }
            
            // Restore original alpha
            for (unsigned mapIdx = 0; mapIdx < indexes.size(); mapIdx++) {
                unsigned clrIdx = indexes[mapIdx];
                // colors[clrIdx].rgbReserved = srcPalette[clrIdx].rgbReserved;
                transparentPtr[clrIdx] = srcPalette[clrIdx].rgbReserved;
            }
            imgI8.SetTransparencyTable(transparentPtr, colorCnt);
        }
    }
    aux.colorizeImg.Close();
    aux.colorizeImg = imgI8;
    // imgI8.Close();
}

//-------------------------------------------------------------------------------------------------
// Perform "Colorlapse" action - timelapse color blending
bool ImageUtilF::Colorlapse(const lstring& fullname, ImageCfg& cfg, ImageAux& aux) {
    FImage img;
    if (LoadImage(img, fullname).Valid()) {
        lstring fullPath(fullname);
     
        unsigned bitsPerPixel = img.GetBitsPerPixel();
        switch (bitsPerPixel) {
            case 8:
                ColorlapseI8(fullname, cfg, aux, img);
                break;
            case 24:
            case 32:
                
            default:
                // FPrint::printInfo(img, fullname);
                std::cerr << fullname << " must by 8 bit per pixel for Colorlapse\n";
                return false;
        }
        
        img.Close();
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//  DLL_CALLCONV
bool DLL_CALLCONV DrawPixelI8(void* imgPtr, unsigned x, unsigned y, BYTE value)
{ return FreeImage_SetPixelIndex(((FImage*)imgPtr)->imgPtr, x, y, &value); }
bool DLL_CALLCONV DrawBoxI8(void* imgPtr, unsigned x1, unsigned y1, unsigned x2, unsigned y2, BYTE value) {
    FBrush fbrush(BrushValue::FILL_IDX, value);
    ((FImage*)imgPtr)->DrawRectangleI8(fbrush, x1, y1, x2, y2);
    return true;
}

//-------------------------------------------------------------------------------------------------
// Save index image palette as a set of images.
void ImageUtilF::Palette(const lstring& fullname) {
    ImageUtilF::init();

    FImage imgI8;
    if (!LoadImage(imgI8, fullname).Valid()) {
        std::cerr << "Paltette - Failed to load " << fullname << std::endl;
        return;
    }
   
    FPalette palette;
    imgI8.getPalette(palette);
   
    if (palette.size() < 2) {
        return; // no usable palette
    }
    
    const unsigned BOX_WIDTH = 32;
    const unsigned BOX_HEIGHT = 32;

    if (/* DISABLES CODE */ (false)) {
        const unsigned BOX_CNT = 32;
        FImageRef paletteImgRef(FImage::Allocate(BOX_WIDTH * BOX_CNT, BOX_HEIGHT, 8));
        paletteImgRef->SetBackgroundColor(palette[0]);

        FBrush brush;
        for (unsigned boxIdx = 0; boxIdx < BOX_CNT; boxIdx++) {
            brush.fillIndex = boxIdx;
            brush.lineIndex = palette.findColor(FColor::GRAY);
            brush.lineWidth = 2;

            unsigned x1 = boxIdx * BOX_WIDTH;
            unsigned y1 = 0;
            paletteImgRef->DrawRectangleI8(brush, x1, y1, x1 + BOX_WIDTH, y1 + BOX_HEIGHT);
        }

        paletteImgRef->SetTransparent(true);
        paletteImgRef->SetTransparentIndex(0);
        paletteImgRef->setPalette(palette);

        ImageUtilF::saveTo(*paletteImgRef, "paletteH32.png");
    }

    if (true) {
        const unsigned BOX_XCNT = 8;
        const unsigned BOX_YCNT = 8;
        FImageRef paletteImgRef(FImage::Allocate(BOX_WIDTH * BOX_XCNT, BOX_HEIGHT * BOX_YCNT, 8));
        paletteImgRef->SetBackgroundColor(palette[0]);

        FBrush brush;
        brush.lineIndex = palette.findColor(FColor::GRAY);
        brush.lineWidth = 2;
        unsigned clrIdx = 0;
        for (unsigned xIdx = 0; xIdx < BOX_XCNT; xIdx++) {
            for (unsigned yIdx = 0; yIdx < BOX_YCNT; yIdx++) {
                brush.fillColor = palette[clrIdx];
                brush.fillIndex = clrIdx++;
                unsigned x1 = xIdx * BOX_HEIGHT;
                unsigned y1 = yIdx * BOX_HEIGHT;
                paletteImgRef->DrawRectangleI8(brush, x1, y1, x1 + BOX_WIDTH, y1 + BOX_HEIGHT);
            }
        }

        paletteImgRef->SetTransparent(true);
        paletteImgRef->SetTransparentIndex(0);
        paletteImgRef->setPalette(palette);
        ImageUtilF::saveTo(*paletteImgRef, "palette8x8.png");
    }
    
    if (true) {
        saveLegend(palette, "palette256.png");
    }
    
    sort(palette.begin(), palette.end(), FPalette::byHSV);
    saveLegend(palette, "paletteSorted.png");
    FPrint::printPalette(palette.quads(), (unsigned)palette.size());
}

// -------------------------------------------------------------------------------------------------
// Save palette as image of color boxes with text describing each  box.
bool ImageUtilF::saveLegend(const FPalette& palette, const lstring& outFileName) {
    const unsigned BOX_WIDTH = 32;
    const unsigned BOX_HEIGHT = 32;
    const unsigned BOX_XCNT = 1;
    const unsigned BOX_YCNT = 256;
    const unsigned LBL_WIDTH = 9*26;
    FImageRef paletteImgRef(FImage::Allocate(BOX_WIDTH * BOX_XCNT + LBL_WIDTH, BOX_HEIGHT * BOX_YCNT, 8));
    // unsigned bgClrIdx = palette.findColor(FPalette::TRANSPARENT);
    paletteImgRef->SetBackgroundColor(FColor::TRANSPARENT);

    
    FBrush brush;
    unsigned grayClrIdx = palette.findClosest(FColor::GRAY);
    // unsigned whiteClrIdx = palette.findClosest(FColor::WHITE);
    unsigned blackClrIdx = palette.findClosest(FColor::BLACK);
    brush.lineIndex = grayClrIdx;
    brush.textIndex = blackClrIdx;
    brush.lineWidth = 2;
    unsigned clrIdx = 0;
    
    FDrawFunc drawFuncs;
    drawFuncs.drawPixelI8 = DrawPixelI8;
    drawFuncs.drawBoxI8 = DrawBoxI8;
    const float scale = 1.0f;
    
    for (unsigned xIdx = 0; xIdx < BOX_XCNT; xIdx++) {
        for (unsigned yIdx = 0; yIdx < BOX_YCNT; yIdx++) {
         
            brush.fillColor = palette[clrIdx];
            brush.fillIndex = clrIdx;
            unsigned x1 = xIdx * BOX_WIDTH;
            // unsigned y1 = yIdx * BOX_HEIGHT;
            unsigned y1 = (BOX_YCNT -1 - yIdx) * BOX_HEIGHT;
            paletteImgRef->DrawRectangleI8(brush, x1, y1, x1 + BOX_WIDTH, y1 + BOX_HEIGHT);
            
            {
            std::ostringstream ostr;
            ostr << clrIdx << ":" <<  FPrint::toString(brush.fillColor,"RGB(%3d,%3d,%3d,%3d)") << std::ends;
            
            FDraw::DrawTextI8(
                drawFuncs,
                paletteImgRef.get(),
                ostr.str(),
                brush,
                x1 + BOX_WIDTH + 10,
                y1,
                scale);
            }
            {
            std::ostringstream ostr;
            ostr << clrIdx << ":" <<  FPrint::toString(brush.fillColor.toHSV(),"HSV(%1.3f,%1.3f,%1.3f)") << std::ends;
            
            FDraw::DrawTextI8(
                drawFuncs,
                paletteImgRef.get(),
                ostr.str(),
                brush,
                x1 + BOX_WIDTH + 10,
                y1 + 10,
                scale);
            }
            
            clrIdx++;
        }
    }

    paletteImgRef->SetTransparent(true);
    paletteImgRef->SetTransparentIndex(0);
    paletteImgRef->setPalette(palette);

    // FPrint::printInfo(*paletteImgRef, "Palette (vertical)");
    // FPrint::printPalette(*paletteImgRef, BOX_WIDTH);
    // FPrint::printHisto(*paletteImgRef, BOX_WIDTH);

    return ImageUtilF::saveTo(*paletteImgRef, outFileName);
}
