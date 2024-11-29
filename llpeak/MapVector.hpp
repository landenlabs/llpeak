//-------------------------------------------------------------------------------------------------
// File: MapVector.hpp
// Desc: Vector with Map interface (keeps original order with Key access).
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

#include "lstring.hpp"
#include <vector>

template <typename TKEY, typename TVALUE>
class MapVector : private std::vector<std::pair<TKEY, TVALUE>> {
  
public:
    typedef std::pair<TKEY, TVALUE> MapPair;
    typedef std::vector<MapPair> Vec;
    typedef typename Vec::const_iterator const_iterator;
    typedef typename Vec::iterator iterator;
    
    using Vec::push_back;
    // using Vec::operator[];
    using Vec::begin;
    using Vec::end;
    using Vec::cbegin;
    using Vec::cend;
    using Vec::front;
    using Vec::back;
    using Vec::clear;
    
    // MapVector operator*(const MapVector & ) const;
    // MapVector operator+(const MapVector & ) const;
    // MapVector();
    // virtual ~MapVector();
    
    
    const_iterator find(const TKEY& id) const {
        auto it = cbegin();
        while (it != cend() && it->first != id) {
            it++;
        }
        return it;
    }
    
    iterator find(const TKEY& id)  {
        auto it = begin();
        while (it != end() && it->first != id) {
            it++;
        }
        return it;
    }

    
    TVALUE& operator[](const TKEY& id)  {
        auto it = find(id);
        if (it == end()) {
            MapPair mapPair;
            mapPair.first = id;
            push_back(mapPair);
            return back().second;
        }
        return it->second;
    }
    
    const TVALUE& operator[](const TKEY& id) const  {
        auto it = find(id);
        if (it == end()) {
            MapPair mapPair;
            mapPair.first = id;
            ((MapVector*)this)->push_back(mapPair);
            return back().second;
        }
        return it->second;
    }
    
    const TVALUE& at(const TKEY& id) const {
        return (*this)[id];
    }

};
