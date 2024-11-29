//-------------------------------------------------------------------------------------------------
// File: Json.hpp
// Desc: Parse json
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

#ifndef json_h
#define json_h

#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <exception>
#include <iostream>
#include <sstream>

#include "lstring.hpp"
#include "MapVector.hpp"
using namespace std;
 

typedef std::vector<lstring> StringList;
// typedef std::map<string, StringList> MapList;



typedef MapVector<string, StringList> MapList;

inline const lstring Join(const StringList& list, const char* delim) {
    size_t len = list.size() * strlen(delim);
    for (const auto& item : list) {
        len += item.length();
    }
    lstring buf;
    buf.reserve(len);
    for (size_t i=0; i<list.size(); i++) {
        if (i != 0) {
            buf += delim;
        }
        buf += list[i];
    }
    return buf;
}

//-------------------------------------------------------------------------------------------------
// Base class for all Json objects
class JsonBase {
public:
    enum Jtype { None, Value, Array, Map };
    Jtype mJtype = None;
    JsonBase(Jtype jtype) {
        mJtype = jtype;
    }
    JsonBase(const JsonBase& other) {
          mJtype = other.mJtype;
    }
    virtual
    string toString(bool allowQuotes = true) const = 0;
    
    virtual
    ostream& dump(ostream& out) const = 0;
    
    virtual
    void toMapList(MapList& mapList, StringList& keys) const = 0;
    
    virtual
    const JsonBase* find(const char* name) const = 0;
};

//-------------------------------------------------------------------------------------------------
// Simple Value
class JsonValue : public JsonBase, public string {
public:
    const char* quote = "\"";
    
    bool isQuoted = false;
    JsonValue() : JsonBase(Value), string() {
    }
    JsonValue(const char* str) : JsonBase(Value), string(str) {
    }
    JsonValue(string& str) : JsonBase(Value), string(str) {
    }
    JsonValue(const JsonValue& other) : JsonBase(other), string(other), isQuoted(other.isQuoted) {
    }
    void clear() {
        isQuoted = false;
        string::clear();
    }
    ostream& dump(ostream& out) const {
        out << toString();
        return out;
    }
    
    void toMapList(MapList& mapList, StringList& keys) const {
        // can't convert a value to a key,value pair.
        mapList[keys.back()].push_back(toString(false));
    }
    
    string toString(bool allowQuotes = true) const {
        if (isQuoted && allowQuotes) {
            return string(quote) + *this + string(quote);
        }
        return *this; // ->c_str();
    }
    
    const JsonBase* find(const char* name) const {
        return nullptr;
    }
};

typedef std::vector<JsonBase*> VecJson;
typedef MapVector<JsonValue, JsonBase*> MapJson;

//-------------------------------------------------------------------------------------------------
// Array of Json objects
class JsonArray : public JsonBase, public VecJson {
public:
    JsonArray() : JsonBase(Array) {
    }
    
    string toString(bool allowQuotes = true) const {
        std::ostringstream ostr;
        ostr << "[\n";
        JsonArray::const_iterator it = begin();
        bool addComma = false;
        while (it != end())
        {
            if (addComma)
                ostr << ",\n";
            addComma = true;
            ostr << (*it++)->toString(allowQuotes);
        }
        ostr << "\n]";
        return ostr.str();
    }
    
    ostream& dump(ostream& out) const {
        cout << toString();
        return cout;
    }
    
    void toMapList(MapList& mapList, StringList& keys) const {
        StringList& list = mapList[Join(keys, ".")];
        JsonArray::const_iterator it = begin();
        while (it != end())
        {
            list.push_back((*it++)->toString(false));
        }
    }
    
    const JsonBase* find(const char* name) const {
        JsonArray::const_iterator it = begin();
        while (it != end()) {
            // JsonValue name = it->first;
            // JsonBase* pValue = it->second;
            it++;
        }
        return nullptr;
    }
};

//-------------------------------------------------------------------------------------------------
// Map (group) of Json objects
class JsonMap : public JsonBase, public MapJson {
public:
    JsonMap() : JsonBase(Map), MapJson() {
    }
    
    string toString(bool allowQuotes = true) const {
        ostringstream out;
        out << "{\n";
        JsonMap::const_iterator it = begin();
        bool addComma = false;
        while (it != end())
        {
            if (addComma)
                out << ",\n";
            addComma = true;
            
            JsonValue name = it->first;
            JsonBase* pValue = it->second;
            out << name.toString() << ": " << pValue->toString(allowQuotes);
            it++;
        }
        out << "\n}\n";
        return out.str();
    }
    
    ostream& dump(ostream& out) const {
        out << toString();
        return out;
    }
    
    void toMapList(MapList& mapList, StringList& keys) const {
        JsonMap::const_iterator it = begin();
        while (it != end())
        {
            std::string name = it->first;
            JsonBase* pValue = it->second;
            keys.push_back(name);
            pValue->toMapList(mapList, keys);
            keys.pop_back();
            it++;
        }
    }
    
    const JsonBase* find(const char* findName) const {
        JsonMap::const_iterator it = cbegin();
        while (it != cend()) {
            const JsonValue& name = it->first;
            const JsonBase* pValue = it->second;
            if (name == findName) {
                return pValue;
            }
            it++;
        }
        return nullptr;
    }
};

// Alternate name JsonFields for JsonMap
typedef JsonMap JsonFields;

//-------------------------------------------------------------------------------------------------
// String buffer being parsed
class JsonBuffer : public std::vector<char> {
public:
    char keyBuf[10];
    const char EOC = (char)0;
    size_t pos = 0;
    int seq = 100;

    char nextChr() {
        if (pos < size()) {
            return at(pos++);
        }
        return EOC;
    }

    char peekChr(int off = 0) {
        off += pos;
        if (off >= 0 && off < size()) {
            return at(off);
        }
        return EOC;
    }

    void moveTo(char find) {
        char c; 
        do {
            c = nextChr();
        } while (c != EOC && c != find);
    }

    string nextKey() {
        snprintf(keyBuf, sizeof(keyBuf), "%03d", seq++);
        return *(new string(keyBuf));
    }
    
    const char* ptr(int len = 0) {
        const char* nowPtr = &at(pos);
        pos = std::min(pos + len, size());
        return nowPtr;
    }
};

//-------------------------------------------------------------------------------------------------
// Json value or parse state change.
class JsonToken : public JsonValue {
public:
    enum Token { Value, EndArray, EndGroup, EndParse } ;
    Token mToken = Value;

    JsonToken() : JsonValue() {
    }
    JsonToken(const char* str) : JsonValue(str) {
    }
    JsonToken(Token token) : JsonValue(), mToken(token) {
    }
    JsonToken(const JsonToken& other) : JsonValue(other), mToken(other.mToken) {
    }
};

static JsonToken END_ARRAY(JsonToken::EndArray);
static JsonToken END_GROUP(JsonToken::EndGroup);
static JsonToken END_PARSE(JsonToken::EndParse);


//-------------------------------------------------------------------------------------------------
class JsonUtil {
public:
    static void getJsonWord(JsonBuffer& buffer, char delim, JsonToken& word);

    static void getJsonArray(JsonBuffer& buffer, JsonArray& array);

    static void getJsonGroup(JsonBuffer& buffer, JsonFields& fields);

    static void addJsonValue(JsonFields& jsonFields, JsonToken& fieldName, JsonToken& value);

    static JsonToken parseJson(JsonBuffer& buffer, JsonFields& jsonFields);
    
    static lstring get(const MapList& mapList, const char* id, const char* defStr);
};
    
#endif /* json_h */

