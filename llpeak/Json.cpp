//-------------------------------------------------------------------------------------------------
// File: Json.cpp
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


#include "Json.hpp"


#ifdef WIN32
    // const char SLASH_CHAR('\\');
    #include <assert.h>
    #define strncasecmp _strnicmp
    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
    #define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
    #endif
#else
    // const char SLASH_CHAR('/');
#endif

//-------------------------------------------------------------------------------------------------
// Parse json word surrounded by quotes.
void JsonUtil::getJsonWord(JsonBuffer& buffer, char delim, JsonToken& word) {
    const char* lastPtr = strchr(buffer.ptr(), delim);
    word.clear();
    int len = int(lastPtr - buffer.ptr());
    word.append(buffer.ptr(len + 1), len);
    word.isQuoted = true;
}

//-------------------------------------------------------------------------------------------------
// Parse json array
void JsonUtil::getJsonArray(JsonBuffer& buffer, JsonArray& array) {
    JsonFields jsonFields;
    for (;;) {
        JsonToken token = parseJson(buffer, jsonFields);
        if (token.mToken == JsonToken::Value) {
            JsonValue* jsonValue = new JsonValue(token);
            array.push_back(jsonValue);
        } else {
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Parse json group
void JsonUtil::getJsonGroup(JsonBuffer& buffer, JsonFields& fields) {
    for (;;) {
        JsonToken token = parseJson(buffer, fields);
        if (token.mToken == JsonToken::EndGroup) {
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void JsonUtil::addJsonValue(JsonFields& jsonFields, JsonToken& fieldName, JsonToken& value) {
    if (!fieldName.empty() && !value.empty()) {
        jsonFields[fieldName] = new JsonToken(value);
        fieldName.clear();
        value.clear();
    }
}

//-------------------------------------------------------------------------------------------------
JsonToken JsonUtil::parseJson(JsonBuffer& buffer, JsonFields& jsonFields) {
    JsonToken fieldName = "";
    JsonToken fieldValue;
    JsonToken tmpValue;

    while (buffer.pos < buffer.size()) {
        char chr = buffer.nextChr();
        switch (chr) {
            default:
                fieldValue += chr;
                break;

            case '/':
                if (buffer.peekChr() == '/') {
                    buffer.moveTo('\n');
                } else {
                    fieldValue += chr;
                }
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                addJsonValue(jsonFields, fieldName, fieldValue);
                break;
            case ',':
                tmpValue = fieldValue;
                addJsonValue(jsonFields, fieldName, fieldValue);
                return tmpValue;

            case ':':
                fieldName = fieldValue;
                fieldValue.clear();
                break;

            case '{': {
                JsonFields* pJsonFields = new JsonFields();
                jsonFields[fieldName] = pJsonFields;
                getJsonGroup(buffer, *pJsonFields);
            } break;
            case '}':
                addJsonValue(jsonFields, fieldName, fieldValue);
                return END_GROUP;

            case '"':
                getJsonWord(buffer, '"', fieldValue);
                break;
            case '[': {
                JsonArray* pJsonArray = new JsonArray();
                jsonFields[fieldName] = pJsonArray;
                getJsonArray(buffer, *pJsonArray);
            } break;
            case ']':
                return END_ARRAY;
        }
    }

    return END_PARSE;
}

//-------------------------------------------------------------------------------------------------
lstring  JsonUtil::get(const MapList& mapList, const char* id, const char* defStr)  {
    auto it = mapList.find(id);
    if (it == mapList.cend() || it->second.empty()) {
        return defStr;
    }
    return it->second.front();
}

