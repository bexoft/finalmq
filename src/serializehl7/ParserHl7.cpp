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



bool ParserHl7::matches(const std::string& segId, const MetaStruct& stru, ssize_t ixStart)
{
    if ((stru.getFlags() & MetaStructFlags::METASTRUCTFLAG_HL7_SEGMENT) != 0)
    {
        return false;
    }
    for (ssize_t i = ixStart; i < stru.getFieldsSize(); ++i)
    {
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field != nullptr);
        if (field->typeId == MetaTypeId::TYPE_STRUCT || field->typeId == MetaTypeId::TYPE_ARRAY_STRUCT)
        {
            if (segId == field->typeNameWithoutNamespace)
            {
                return true;
            }
            const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
            if (subStruct != nullptr)
            {
                bool match = matches(segId, *subStruct, 0);
                if (match)
                {
                    return true;
                }
            }
            if ((field->typeId == MetaTypeId::TYPE_STRUCT) &&
                (field->flags & MetaFieldFlags::METAFLAG_NULLABLE) == 0)
            {
                return false;
            }
            if ((field->typeId == MetaTypeId::TYPE_ARRAY_STRUCT) &&
                (field->flags & MetaFieldFlags::METAFLAG_ONE_REQUIRED) != 0)
            {
                return false;
            }
        }
    }
    return false;
}

bool ParserHl7::matchesUp(const std::string& segId)
{
    if (m_stackStruct.empty())
    {
        return false;
    }
    for (ssize_t i = static_cast<ssize_t>(m_stackStruct.size()) - 1; i >= 0; --i)
    {
        const std::pair<MetaStruct, ssize_t>& entry = m_stackStruct[i];
        bool match = matches(segId, entry.first, entry.second);
        if (match)
        {
            return true;
        }
    }
    return false;
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
    bool isarrayDummy;
    int level = parseStruct(0, *stru, isarrayDummy);
    m_visitor.finished();

    if (level <= PARSER_HL7_ERROR)
    {
        return nullptr;
    }
    return m_parser.getCurrentPosition();
}

int ParserHl7::parseStruct(int levelSegment, const MetaStruct& stru, bool& isarray)
{
    isarray = false;
    // segment ID
    if (levelSegment == 1)
    {
        std::string tokenSegId;
        bool isarrayDummy;
        int levelNew = m_parser.parseToken(levelSegment, tokenSegId, isarrayDummy);
        assert(isarray == false);
        if (levelNew < levelSegment)
        {
            return levelNew;
        }
        if (tokenSegId != stru.getTypeNameWithoutNamespace())
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
                const std::string& typeName = field->typeNameWithoutNamespace;
                std::string segId;
                m_parser.getSegmentId(segId);
                if (segId == "")
                {
                    return levelSegment;
                }
                else if (segId == typeName)
                {
                    processStruct = true;
                }
                else if (matches(segId, *subStruct, 0))
                {
                    processStruct = true;
                }
                else if (matches(segId, stru, i + 1))
                {
                    processStruct = false;
                }
                else if (matchesUp(segId))
                {
                    return levelSegment;
                }
                else
                {
                    processStruct = false;
                    m_parser.parseTillEndOfStruct(0);
                    --i;
                }
            }
            if (processStruct)
            {
                m_visitor.enterStruct(*field);
                m_stackStruct.emplace_back(stru, i + 1);
                int LevelSegmentNext = levelSegment;
                if ((LevelSegmentNext > 0) || (subStruct->getFlags() & METASTRUCTFLAG_HL7_SEGMENT))
                {
                    ++LevelSegmentNext;
                }
                int levelNew = parseStruct(LevelSegmentNext, *subStruct, isarray);
                m_stackStruct.pop_back();
                m_visitor.exitStruct(*field);
                if (isarray)
                {
                    isarray = false;
                    m_parser.parseTillEndOfStruct(levelNew);
                }
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }
                if ((levelSegment == 0) && ((stru.getFlags() & MetaStructFlags::METASTRUCTFLAG_CHOICE) != 0))
                {
                    return levelSegment;
                }
            }
        }
        else if (field->typeId == TYPE_ARRAY_STRUCT)
        {
            const MetaStruct* subStruct = MetaDataGlobal::instance().getStruct(*field);
            if (subStruct == nullptr)
            {
                return PARSER_HL7_ERROR;
            }
            const MetaField* fieldWithoutArray = field->fieldWithoutArray;
            assert(fieldWithoutArray);
            if (levelSegment == 0)
            {
                std::string typeName = field->typeNameWithoutNamespace;
                bool firstLoop = true;
                while (true)
                {
                    std::string segId;
                    m_parser.getSegmentId(segId);
                    bool processStructArray = true;
                    if (segId == "")
                    {
                        return levelSegment;
                    }
                    else if (segId == typeName)
                    {
                        processStructArray = true;
                    }
                    else if (matches(segId, *subStruct, 0))
                    {
                        processStructArray = true;
                    }
                    else if (matches(segId, stru, i + 1))
                    {
                        processStructArray = false;
                    }
                    else if (matchesUp(segId))
                    {
                        return levelSegment;
                    }
                    else
                    {
                        processStructArray = false;
                        m_parser.parseTillEndOfStruct(0);
                        --i;
                    }

                    if (processStructArray)
                    {
                        m_parser.getSegmentId(segId);
                        if (firstLoop)
                        {
                            m_stackStruct.emplace_back(stru, i);
                            firstLoop = false;
                            m_visitor.enterArrayStruct(*field);
                        }
                        m_visitor.enterStruct(*fieldWithoutArray);
                        int levelSegmentNext = levelSegment;
                        if ((subStruct->getFlags() & MetaStructFlags::METASTRUCTFLAG_HL7_SEGMENT) != 0)
                        {
                            ++levelSegmentNext;
                        }
                        int levelNew = parseStruct(levelSegmentNext, *subStruct, isarray);
                        m_visitor.exitStruct(*fieldWithoutArray);
                        (void) levelNew;
                        assert(levelNew == 0);
                        assert(isarray == false);
                    }
                    else
                    {
                        if (!firstLoop)
                        {
                            m_visitor.exitArrayStruct(*field);
                            m_stackStruct.pop_back();
                        }
                        break;
                    }
                }
            }
            else
            {
                bool filled = false;
                int levelNew = m_parser.isNextFieldFilled(levelSegment, filled);
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }

                if (filled)
                {
                    levelNew = levelSegment;
                    m_visitor.enterArrayStruct(*field);
                    while (true)
                    {
                        m_visitor.enterStruct(*fieldWithoutArray);
                        levelNew = parseStruct(levelSegment + 1, *subStruct, isarray);
                        m_visitor.exitStruct(*fieldWithoutArray);
                        if (levelNew < levelSegment || !isarray)
                        {
                            break;
                        }
                    }
                    m_visitor.exitArrayStruct(*field);
                    isarray = false;
                    if (levelNew < levelSegment)
                    {
                        return levelNew;
                    }
                }
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
                int levelNew = m_parser.parseToken(levelSegment, token, isarray);
                if (!token.empty())
                {
                    m_visitor.enterString(*field, std::move(token));
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
