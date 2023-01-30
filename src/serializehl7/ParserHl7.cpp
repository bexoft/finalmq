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
#include "finalmq/helpers/base64.h"

#include "finalmq/conversions/itoa.h"
#include "finalmq/conversions/dtoa.h"

#include <assert.h>


namespace finalmq {

static const int PARSER_HL7_ERROR = -2;

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

    if (level <= PARSER_HL7_ERROR)
    {
        return nullptr;
    }
    return m_parser.getCurrentPosition();
}

static std::string removeNamespace(const std::string& typeName)
{
    return typeName.substr(typeName.find_last_of('.') + 1, 3);
}

int ParserHl7::parseStruct(int levelStruct, int levelSegment, const MetaStruct& stru)
{
    // segment ID
    if (levelSegment == 1)
    {
        std::string tokenSegId;
        int levelNew = m_parser.parseToken(levelSegment, tokenSegId);
        if (levelNew < levelSegment)
        {
            return levelNew;
        }
        if (tokenSegId != removeNamespace(stru.getTypeName()))
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
            const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
            if (subStruct == nullptr)
            {
                return PARSER_HL7_ERROR;
            }
            bool processStruct = true;
            if (levelSegment == 0)
            {
                std::string typeName = removeNamespace(field->typeName);
                std::string segId;
                m_parser.getSegmentId(segId);
                const MetaField* subField = nullptr;
                if ((segId != typeName) && !(subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT) && (subStruct->getFieldsSize() > 0))
                {
                    subField = subStruct->getFieldByIndex(0);
                }
                if ((segId == typeName) || (subField != nullptr && segId == removeNamespace(subField->typeName)))
                {
                    processStruct = true;
                }
                else
                {
                    processStruct = false;
                    m_parser.parseTillEndOfStruct(0);
                }
            }
            if (processStruct)
            {
                m_visitor.enterStruct(*field);
                int LevelSegmentNext = levelSegment;
                if ((LevelSegmentNext > 0) || (subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT))
                {
                    ++LevelSegmentNext;
                }
                int levelNew = parseStruct(levelStruct + 1, LevelSegmentNext, *subStruct);
                m_visitor.exitStruct(*field);
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }
            }
        }
        else if (field->typeId == TYPE_ARRAY_STRUCT)
        {
            if (levelSegment == 0)
            {
                const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
                if (subStruct == nullptr)
                {
                    return PARSER_HL7_ERROR;
                }
                const MetaField* subField = nullptr;
                if (!(subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT) && (subStruct->getFieldsSize() > 0))
                {
                    subField = subStruct->getFieldByIndex(0);
                }
                std::string typeName = removeNamespace(field->typeName);
                bool firstLoop = true;
                do
                {
                    std::string segId;
                    m_parser.getSegmentId(segId);
                    if ((segId == typeName) || (subField != nullptr && segId == removeNamespace(subField->typeName)))
                    {
                        if (firstLoop)
                        {
                            firstLoop = false;
                            m_visitor.enterArrayStruct(*field);
                        }
                        m_visitor.enterStruct(*field);
                        int LevelSegmentNext = levelSegment;
                        if ((LevelSegmentNext > 0) || (subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT))
                        {
                            ++LevelSegmentNext;
                        }
                        int levelNew = parseStruct(levelStruct + 1, LevelSegmentNext, *subStruct);
                        m_visitor.exitStruct(*field);
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }
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
