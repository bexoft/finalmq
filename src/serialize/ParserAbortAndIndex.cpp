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


#include "finalmq/serialize/ParserAbortAndIndex.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"

#include <assert.h>
#include <iostream>
#include <algorithm>


namespace finalmq {



ParserAbortAndIndex::ParserAbortAndIndex(IParserVisitor* visitor)
    : m_visitor(visitor)
{

}

void ParserAbortAndIndex::setVisitor(IParserVisitor &visitor)
{
    m_visitor = &visitor;
}


// ParserAbortAndIndex
void ParserAbortAndIndex::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}


void ParserAbortAndIndex::startStruct(const MetaStruct& stru)
{
    m_visitor->startStruct(stru);

    m_levelState.push_back(LevelState());
}


void ParserAbortAndIndex::finished()
{
    if (!m_levelState.empty())
    {
        m_levelState.pop_back();
    }

    assert(m_visitor);
    m_visitor->finished();
}

void ParserAbortAndIndex::enterStruct(const MetaField& field)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();

    if (levelState.abortStruct || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index))
    {
        m_levelState.push_back(LevelState());
        m_levelState.back().abortStruct = ABORT_STRUCT;
        return;
    }

    m_visitor->enterStruct(field);

    m_levelState.push_back(LevelState());
}

void ParserAbortAndIndex::exitStruct(const MetaField& field)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if (levelState.abortStruct == ABORT_STRUCT)
    {
        m_levelState.pop_back();
        return;
    }

    m_visitor->exitStruct(field);

    m_levelState.pop_back();
}


void ParserAbortAndIndex::enterStructNull(const MetaField& field)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }

    m_visitor->enterStructNull(field);
}




void ParserAbortAndIndex::enterArrayStruct(const MetaField& field)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();

    if (levelState.abortStruct || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        m_levelState.push_back(LevelState());
        m_levelState.back().abortStruct = ABORT_STRUCT;
        return;
    }

    m_visitor->enterArrayStruct(field);

    m_levelState.push_back(LevelState());
}

void ParserAbortAndIndex::exitArrayStruct(const MetaField& field)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();

    if (levelState.abortStruct == ABORT_STRUCT)
    {
        m_levelState.pop_back();
        return;
    }

    m_visitor->exitArrayStruct(field);

    m_levelState.pop_back();
}

static const std::string ABORTSTRUCT = "abortstruct";
static const std::string ABORT_FALSE = "false";
static const std::string ABORT_TRUE = "true";

void ParserAbortAndIndex::enterBool(const MetaField& field, bool value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterBool(field, value);
    
    // check abort
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty())
    {
        if (((valueAbort == ABORT_TRUE) && value) ||
            ((valueAbort == ABORT_FALSE) && !value))
        {
            levelState.abortStruct = ABORT_FIELD;
        }
    }
}
void ParserAbortAndIndex::enterInt8(const MetaField& field, std::int8_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }

    m_visitor->enterInt8(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterUInt8(const MetaField& field, std::uint8_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    m_visitor->enterUInt8(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterInt16(const MetaField& field, std::int16_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterInt16(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterUInt16(const MetaField& field, std::uint16_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterUInt16(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterInt32(const MetaField& field, std::int32_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterInt32(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterUInt32(const MetaField& field, std::uint32_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterUInt32(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterInt64(const MetaField& field, std::int64_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterInt64(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterUInt64(const MetaField& field, std::uint64_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterUInt64(field, value);
    checkIndex(field, value);
}
void ParserAbortAndIndex::enterFloat(const MetaField& field, float value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterFloat(field, value);
}
void ParserAbortAndIndex::enterDouble(const MetaField& field, double value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterDouble(field, value);
}
void ParserAbortAndIndex::enterString(const MetaField& field, std::string&& value)
{
    enterString(field, value.c_str(), value.size());
}
void ParserAbortAndIndex::enterString(const MetaField& field, const char* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterString(field, value, size);

    // check abort
    std::string strValue;
    if (value)
    {
        strValue = std::string(value, &value[size]);
    }
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty())
    {
        std::vector<std::string> valuesAbort;
        Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
        if (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end())
        {
            levelState.abortStruct = ABORT_FIELD;
        }
    }
    else
    {
        checkIndex(field, strValue);
    }
}
void ParserAbortAndIndex::enterBytes(const MetaField& field, Bytes&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterBytes(field, move(value));
}
void ParserAbortAndIndex::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterBytes(field, value, size);
}
void ParserAbortAndIndex::enterEnum(const MetaField& field, std::int32_t value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterEnum(field, value);

    // check abort
    const MetaEnum* en = MetaDataGlobal::instance().getEnum(field.typeName);
    if (en == nullptr)
    {
        streamError << "enum not found " << field.typeName;
    }
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty() && en)
    {
        std::string strValue = en->getNameByValue(value);
        std::vector<std::string> valuesAbort;
        Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
        if (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end())
        {
            levelState.abortStruct = ABORT_FIELD;
        }
        else
        {
            std::string aliasValue = en->getAliasByValue(value);
            if (std::find(valuesAbort.begin(), valuesAbort.end(), aliasValue) != valuesAbort.end())
            {
                levelState.abortStruct = ABORT_FIELD;
            }
        }
    }
    else
    {
        checkIndex(field, value);
    }
}
void ParserAbortAndIndex::enterEnum(const MetaField& field, std::string&& value)
{
    m_visitor->enterEnum(field, value.data(), value.size());
}
void ParserAbortAndIndex::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterEnum(field, value, size);

    // check abort
    const MetaEnum* en = MetaDataGlobal::instance().getEnum(field.typeName);
    if (en == nullptr)
    {
        streamError << "enum not found " << field.typeName;
    }
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty() && en)
    {
        std::int32_t v = en->getValueByName(value);
        std::string strValue = en->getNameByValue(v);
        std::vector<std::string> valuesAbort;
        Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
        if (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end())
        {
            levelState.abortStruct = ABORT_FIELD;
        }
        else
        {
            std::string aliasValue = en->getAliasByValue(v);
            if (std::find(valuesAbort.begin(), valuesAbort.end(), aliasValue) != valuesAbort.end())
            {
                levelState.abortStruct = ABORT_FIELD;
            }
        }
    }
    else
    {
        if (en)
        {
            std::int32_t v = en->getValueByName(value);
            checkIndex(field, v);
        }
    }
}

void ParserAbortAndIndex::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayBoolMove(field, move(value));
}
void ParserAbortAndIndex::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayBool(field, value);
}
void ParserAbortAndIndex::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt8(field, move(value));
}
void ParserAbortAndIndex::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt8(field, value, size);
}
void ParserAbortAndIndex::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt16(field, move(value));
}
void ParserAbortAndIndex::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt16(field, value, size);
}
void ParserAbortAndIndex::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt16(field, move(value));
}
void ParserAbortAndIndex::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt16(field, value, size);
}
void ParserAbortAndIndex::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt32(field, move(value));
}
void ParserAbortAndIndex::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt32(field, value, size);
}
void ParserAbortAndIndex::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt32(field, move(value));
}
void ParserAbortAndIndex::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt32(field, value, size);
}
void ParserAbortAndIndex::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt64(field, move(value));
}
void ParserAbortAndIndex::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayInt64(field, value, size);
}
void ParserAbortAndIndex::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt64(field, move(value));
}
void ParserAbortAndIndex::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayUInt64(field, value, size);
}
void ParserAbortAndIndex::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayFloat(field, move(value));
}
void ParserAbortAndIndex::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayFloat(field, value, size);
}
void ParserAbortAndIndex::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayDouble(field, move(value));
}
void ParserAbortAndIndex::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayDouble(field, value, size);
}
void ParserAbortAndIndex::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayStringMove(field, move(value));
}
void ParserAbortAndIndex::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayString(field, value);
}
void ParserAbortAndIndex::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayBytesMove(field, move(value));
}
void ParserAbortAndIndex::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayBytes(field, value);
}
void ParserAbortAndIndex::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayEnum(field, move(value));
}
void ParserAbortAndIndex::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayEnum(field, value, size);
}
void ParserAbortAndIndex::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayEnumMove(field, move(value));
}
void ParserAbortAndIndex::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    assert(!m_levelState.empty());
    LevelState& levelState = m_levelState.back();
    if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index && levelState.indexOfIndexField < field.index ))
    {
        return;
    }
    
    m_visitor->enterArrayEnum(field, value);
}


static const std::string INDEXMODE = "indexmode";
static const std::string INDEXMODE_MAPPING = "mapping";
static const std::string INDEXOFFSET = "indexoffset";


void ParserAbortAndIndex::checkIndex(const MetaField& field, std::int64_t value)
{
    if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();

        std::int64_t indexOffset = 0;
        const std::string& strIndexOffset = field.getProperty(INDEXOFFSET);
        if (!strIndexOffset.empty())
        {
            indexOffset = atoll(strIndexOffset.c_str());
        }
        const std::string& indexmode = field.getProperty(INDEXMODE);
        if (indexmode == INDEXMODE_MAPPING)
        {
            const std::string strIndex = std::to_string(value);
            const std::string& strIndexMapped = field.getProperty(strIndex);
            if (strIndexMapped.empty())
            {
                levelState.abortStruct = ABORT_FIELD;
            }
            else
            {
                int indexMapped = atoi(strIndexMapped.c_str());
                levelState.indexOfIndexField = field.index + indexOffset;
                levelState.index = field.index + indexOffset + 1 + indexMapped;
            }
        }
        else
        {
            if (value < 0)
            {
                levelState.abortStruct = ABORT_FIELD;
            }
            else
            {
                levelState.indexOfIndexField = field.index + indexOffset;
                levelState.index = field.index + indexOffset + 1 + value;
            }
        }
    }
}

void ParserAbortAndIndex::checkIndex(const MetaField& field, const std::string& value)
{
    if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();

        std::int64_t indexOffset = 0;
        const std::string& strIndexOffset = field.getProperty(INDEXOFFSET);
        if (!strIndexOffset.empty())
        {
            indexOffset = atoll(strIndexOffset.c_str());
        }
        const std::string& indexmode = field.getProperty(INDEXMODE);
        if (indexmode == INDEXMODE_MAPPING)
        {
            const std::string& strIndexMapped = field.getProperty(value);
            if (strIndexMapped.empty())
            {
                levelState.abortStruct = ABORT_FIELD;
            }
            else
            {
                int indexMapped = atoi(strIndexMapped.c_str());
                levelState.indexOfIndexField = field.index + indexOffset;
                levelState.index = field.index + indexOffset + 1 + indexMapped;
            }
        }
        else
        {
            const std::int64_t indexValue = atoll(value.c_str());
            if (indexValue < 0)
            {
                levelState.abortStruct = ABORT_FIELD;
            }
            else
            {
                levelState.indexOfIndexField = field.index + indexOffset;
                levelState.index = field.index + indexOffset + 1 + indexValue;
            }
        }
    }
}



}   // namespace finalmq
