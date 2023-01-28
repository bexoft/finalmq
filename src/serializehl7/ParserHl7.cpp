//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


#include "finalmq/serializehl7/ParserHl7.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/helpers/base64.h"

#include "finalmq/conversions/itoa.h"
#include "finalmq/conversions/dtoa.h"

#include <assert.h>


namespace finalmq {



ParserHl7::ParserHl7(IParserVisitor& visitor, const char* ptr, ssize_t size)
    : m_ptr(ptr)
    , m_size(size)
    , m_visitor(visitor)
//    , m_parser(*this)
{
}





const char* ParserHl7::parseStruct(const std::string& typeName)
{
    assert(m_ptr);
    assert(m_size >= CHECK_ON_ZEROTERM);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        m_visitor.finished();
        return nullptr;
    }

    bool ok = m_parser.startParse(m_ptr, m_size);
    if (!ok)
    {
        return nullptr;
    }
        
    m_visitor.startStruct(*stru);
    int level = parseStruct(0, 0, *stru);
    m_visitor.finished();

    if (level < 0)
    {
        return nullptr;
    }
    return m_parser.getCurrentPosition();
}

static std::string removeNamespace(const std::string& typeName)
{
    return typeName.substr(typeName.find_last_of('.') + 1);
}

int ParserHl7::parseStruct(int levelStruct, int levelSegment, const MetaStruct& stru)
{
    // segment ID
    if (levelSegment == 1)
    {
        std::string tokenSegId;
        int levelNew = m_parser.parseToken(levelSegment, tokenSegId);
        if (levelNew == -1)
        {
            return -1;
        }
        if (levelNew < levelSegment)
        {
            return levelNew;
        }
        if ((tokenSegId != removeNamespace(stru.getTypeName())) && (tokenSegId + "_STRUCT" != removeNamespace(stru.getTypeName()))) //todo: optimize
        {
            return m_parser.parseTillEndOfStruct(0);
        }
    }

    ssize_t size = stru.getFieldsSize();
    for (ssize_t i = 0; i < size; ++i)
    {
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field != nullptr);

        if (field->typeId == TYPE_STRUCT)
        {
            bool processStruct = true;
            if (levelSegment == 0)
            {
                std::string segId;
                m_parser.getSegmentId(segId);
                if (segId != removeNamespace(field->typeName))
                {
                    processStruct = false;
                    m_parser.parseTillEndOfStruct(0);
                }
            }
            if (processStruct)
            {
                const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
                if (subStruct == nullptr)
                {
                    return -1;
                }
                m_visitor.enterStruct(*field);
                if ((levelSegment > 0) || (subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT))
                {
                    int levelNew = parseStruct(levelStruct + 1, levelSegment + 1, *subStruct);
                    if (levelNew == -1)
                    {
                        return -1;
                    }
                    if (levelNew < levelSegment)
                    {
                        m_visitor.exitStruct(*field);
                        return levelNew;
                    }
                }
                m_visitor.exitStruct(*field);
            }
        }
        else if (field->typeId == TYPE_ARRAY_STRUCT)
        {
            if (levelSegment == 0)
            {
                std::string typeName = removeNamespace(field->typeName);
                typeName = typeName.substr(0, 3);   // todo: remove _STRUCT
                bool firstLoop = true;
                do
                {
                    std::string segId;
                    m_parser.getSegmentId(segId);
                    if (segId == typeName)
                    {
                        if (firstLoop)
                        {
                            firstLoop = false;
                            m_visitor.enterArrayStruct(*field);
                        }
                        const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
                        if (subStruct == nullptr)
                        {
                            return -1;
                        }
                        m_visitor.enterStruct(*field);
                        int LevelSegmentNext = levelSegment;
                        if (subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT)
                        {
                            ++LevelSegmentNext;
                        }
                        int levelNew = parseStruct(levelStruct + 1, LevelSegmentNext, *subStruct);
                        if (levelNew == -1)
                        {
                            return -1;
                        }
                        if (levelNew < levelSegment)
                        {
                            m_visitor.exitStruct(*field);
                            return levelNew;
                        }
                        m_visitor.exitStruct(*field);
                    }
                    else
                    {
                        if (!firstLoop)
                        {
                            m_visitor.exitArrayStruct(*field);
                        }
                        break;
                    }
                } while (true);
            }
        }
        else if (field->typeId & OFFSET_ARRAY_FLAG)
        {
            if (levelSegment > 0)
            {
                std::vector<std::string> array;
                int levelNew = m_parser.parseTokenArray(levelSegment, array);
                m_visitor.enterArrayString(*field, std::move(array));
                if (levelNew == -1)
                {
                    return -1;
                }
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }
            }
        }
        else
        {
            if (levelSegment > 0)
            {
                std::string token;
                int levelNew = m_parser.parseToken(levelSegment, token);
                m_visitor.enterString(*field, std::move(token));
                if (levelNew == -1)
                {
                    return -1;
                }
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }
            }
        }
    }
    if (levelSegment > 0)
    {
        return m_parser.parseTillEndOfStruct(levelSegment - 1);
    }
    return levelSegment;
}



}   // namespace finalmq
