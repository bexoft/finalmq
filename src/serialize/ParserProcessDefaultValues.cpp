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

#include "finalmq/serialize/ParserProcessDefaultValues.h"

#include <algorithm>
#include <iostream>

#include <assert.h>

#include "finalmq/metadata/MetaData.h"

namespace finalmq
{
static const std::string STR_VARVALUE = "finalmq.variant.VarValue";
static const std::string FIXED_ARRAY = "fixedarray";

ParserProcessDefaultValues::ParserProcessDefaultValues(bool skipDefaultValues, IParserVisitor* visitor)
    : m_visitor(visitor), m_skipDefaultValues(skipDefaultValues)
{
}

void ParserProcessDefaultValues::setVisitor(IParserVisitor& visitor)
{
    m_visitor = &visitor;
}

void ParserProcessDefaultValues::resetVarValueActive()
{
    m_varValueActive = 0;
}

// ParserProcessDefaultValues
void ParserProcessDefaultValues::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}

void ParserProcessDefaultValues::startStruct(const MetaStruct& stru)
{
    m_struct = &stru;
    if (!m_skipDefaultValues)
    {
        m_stackFieldsDone.emplace_back(stru.getFieldsSize(), false);
    }
    else
    {
        m_stackSkipDefault.emplace_back(nullptr, true);
    }
    assert(m_visitor);
    m_visitor->startStruct(stru);
}

void ParserProcessDefaultValues::finished()
{
    if (!m_skipDefaultValues && m_struct)
    {
        assert(!m_stackFieldsDone.empty());
        const std::vector<bool>& fieldsDone = m_stackFieldsDone.back();
        processDefaultValues(*m_struct, fieldsDone);
        m_stackFieldsDone.pop_back();
    }

    if (!m_stackSkipDefault.empty())
    {
        m_stackSkipDefault.pop_back();
    }

    assert(m_visitor);
    m_visitor->finished();
}

void ParserProcessDefaultValues::executeEnterStruct()
{
    for (ssize_t i = m_stackSkipDefault.size() - 1; i >= 0; --i)
    {
        EntrySkipDefault& entry1 = m_stackSkipDefault[i];
        if (entry1.enterStructCalled())
        {
            ++i;
            for (; i < static_cast<ssize_t>(m_stackSkipDefault.size()); ++i)
            {
                EntrySkipDefault& entry2 = m_stackSkipDefault[i];
                m_visitor->enterStruct(*entry2.field());
                entry2.enterStructCalled() = true;
            }
            break;
        }
    }
}

void ParserProcessDefaultValues::enterStruct(const MetaField& field)
{
    if (m_blockVisitor)
    {
        ++m_blockVisitor;
        return;
    }
        
    assert(m_visitor);

    if (!m_skipDefaultValues)
    {
        markAsDone(field);
        const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
        if (stru)
        {
            m_stackFieldsDone.emplace_back(stru->getFieldsSize(), false);
        }
        else
        {
            m_stackFieldsDone.emplace_back(0, false);
        }
        if (field.typeName == STR_VARVALUE)
        {
            m_varValueActive++;
        }

        if (!m_stackArrayStructState.empty())
        {
            EntryArrayStructState& arrayStructState = m_stackArrayStructState.back();
            if (arrayStructState.level == 0)
            {
                if (arrayStructState.numberOfArrayEntries < arrayStructState.fixedSize)
                {
                    ++arrayStructState.numberOfArrayEntries;
                }
                else
                {
                    ++m_blockVisitor;
                }
            }
            ++arrayStructState.level;
        }

        if (!m_blockVisitor)
        {
            m_visitor->enterStruct(field);
        }
    }
    else
    {
        if (!m_stackSkipDefault.empty())
        {
            auto& entry = m_stackSkipDefault.back();
            if (entry.fieldArrayStruct() != nullptr)
            {
                if (!entry.enterArrayStructCalled())
                {
                    executeEnterStruct();
                    m_visitor->enterArrayStruct(*entry.fieldArrayStruct());
                    entry.enterArrayStructCalled() = true;
                }
                m_visitor->enterStruct(field);
                m_stackSkipDefault.emplace_back(&field, true);
            }
            else
            {
                m_stackSkipDefault.emplace_back(&field, false);
            }
        }
    }
}

void ParserProcessDefaultValues::exitStruct(const MetaField& field)
{
    if (m_blockVisitor)
    {
        --m_blockVisitor;
        return;
    }

    if (!m_skipDefaultValues)
    {
        assert(!m_stackFieldsDone.empty());
        const std::vector<bool>& fieldsDone = m_stackFieldsDone.back();
        const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
        if (stru)
        {
            processDefaultValues(*stru, fieldsDone);
        }

        m_stackFieldsDone.pop_back();
    }
    if (m_varValueActive > 0)
    {
        m_varValueActive--;
    }

    if (!m_skipDefaultValues)
    {
        m_visitor->exitStruct(field);
        if (!m_stackArrayStructState.empty())
        {
            EntryArrayStructState& arrayStructState = m_stackArrayStructState.back();
            if (arrayStructState.level > 0)
            {
                --arrayStructState.level;
            }
        }
    }
    else
    {
        if (!m_stackSkipDefault.empty())
        {
            auto& entry = m_stackSkipDefault.back();
            if (entry.enterStructCalled())
            {
                m_visitor->exitStruct(field);
            }
            m_stackSkipDefault.pop_back();
        }
    }
}

void ParserProcessDefaultValues::enterStructNull(const MetaField& field)
{
    if (m_blockVisitor)
    {
        return;
    }

    markAsDone(field);
    if (!m_skipDefaultValues)
    {
        m_visitor->enterStructNull(field);
    }
}

void ParserProcessDefaultValues::processDefaultValues(const MetaStruct& stru, const std::vector<bool>& fieldsDone)
{
    assert(!m_skipDefaultValues);
    for (size_t index = 0; index < fieldsDone.size(); ++index)
    {
        if (!fieldsDone[index])
        {
            const MetaField* field = stru.getFieldByIndex(index);
            if (field)
            {
                switch(field->typeId)
                {
                    case MetaTypeId::TYPE_NONE:
                        break;
                    case MetaTypeId::TYPE_BOOL:
                        m_visitor->enterBool(*field, false);
                        break;
                    case MetaTypeId::TYPE_INT8:
                        m_visitor->enterInt8(*field, 0);
                        break;
                    case MetaTypeId::TYPE_UINT8:
                        m_visitor->enterUInt8(*field, 0);
                        break;
                    case MetaTypeId::TYPE_INT16:
                        m_visitor->enterInt16(*field, 0);
                        break;
                    case MetaTypeId::TYPE_UINT16:
                        m_visitor->enterUInt16(*field, 0);
                        break;
                    case MetaTypeId::TYPE_INT32:
                        m_visitor->enterInt32(*field, 0);
                        break;
                    case MetaTypeId::TYPE_UINT32:
                        m_visitor->enterUInt32(*field, 0);
                        break;
                    case MetaTypeId::TYPE_INT64:
                        m_visitor->enterInt64(*field, 0);
                        break;
                    case MetaTypeId::TYPE_UINT64:
                        m_visitor->enterUInt64(*field, 0);
                        break;
                    case MetaTypeId::TYPE_FLOAT:
                        m_visitor->enterFloat(*field, 0.0);
                        break;
                    case MetaTypeId::TYPE_DOUBLE:
                        m_visitor->enterDouble(*field, 0.0);
                        break;
                    case MetaTypeId::TYPE_STRING:
                        m_visitor->enterString(*field, "", 0);
                        break;
                    case MetaTypeId::TYPE_BYTES:
                    {
                        static const BytesElement dummy{};
                        m_visitor->enterBytes(*field, &dummy, 0);
                    }
                    break;
                    case MetaTypeId::TYPE_STRUCT:
                        if (!(field->flags & METAFLAG_NULLABLE))
                        {
                            m_visitor->enterStruct(*field);
                            const MetaStruct* substru = MetaDataGlobal::instance().getStruct(*field);
                            if (substru)
                            {
                                std::vector<bool> subfieldsDone(substru->getFieldsSize(), false);
                                processDefaultValues(*substru, subfieldsDone);
                            }
                            m_visitor->exitStruct(*field);
                        }
                        else
                        {
                            m_visitor->enterStructNull(*field);
                        }
                        break;
                    case MetaTypeId::TYPE_ENUM:
                        m_visitor->enterEnum(*field, 0);
                        break;
                    case MetaTypeId::TYPE_ARRAY_BOOL:
                        m_visitor->enterArrayBoolMove(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_INT8:
                        m_visitor->enterArrayInt8(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_INT16:
                        m_visitor->enterArrayInt16(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_UINT16:
                        m_visitor->enterArrayUInt16(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_INT32:
                        m_visitor->enterArrayInt32(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_UINT32:
                        m_visitor->enterArrayUInt32(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_INT64:
                        m_visitor->enterArrayInt64(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_UINT64:
                        m_visitor->enterArrayUInt64(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_FLOAT:
                        m_visitor->enterArrayFloat(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_DOUBLE:
                        m_visitor->enterArrayDouble(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_STRING:
                        m_visitor->enterArrayStringMove(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_BYTES:
                        m_visitor->enterArrayBytesMove(*field, {});
                        break;
                    case MetaTypeId::TYPE_ARRAY_STRUCT:
                        m_visitor->enterArrayStruct(*field);
                        m_visitor->exitArrayStruct(*field);
                        break;
                    case MetaTypeId::TYPE_ARRAY_ENUM:
                        m_visitor->enterArrayEnum(*field, std::vector<std::int32_t>());
                        break;
                    case MetaTypeId::OFFSET_ARRAY_FLAG:
                        assert(false);
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }
    }
}

void ParserProcessDefaultValues::markAsDone(const MetaField& field)
{
    if (!m_skipDefaultValues)
    {
        assert(!m_stackFieldsDone.empty());
        std::vector<bool>& fieldsDone = m_stackFieldsDone.back();
        ssize_t index = field.index;
        if (index >= 0 && index < static_cast<ssize_t>(fieldsDone.size()))
        {
            fieldsDone[index] = true;
        }
    }
}

void ParserProcessDefaultValues::enterArrayStruct(const MetaField& field)
{
    if (m_blockVisitor)
    {
        return;
    }

    markAsDone(field);
    if (!m_skipDefaultValues)
    {
        const std::string& fixedArray = field.getProperty(FIXED_ARRAY);
        if (!fixedArray.empty())
        {
            const int fixedSize = atoi(fixedArray.c_str());
            m_stackArrayStructState.push_back({ fixedSize, 0, 0 });
        }
        m_visitor->enterArrayStruct(field);
    }
    else
    {
        if (!m_stackSkipDefault.empty())
        {
            // call enterArrayStruct before the first element at enterStruct
            auto& entry = m_stackSkipDefault.back();
            entry.fieldArrayStruct() = &field;
            entry.enterArrayStructCalled() = false;
        }
    }
}
void ParserProcessDefaultValues::exitArrayStruct(const MetaField& field)
{
    if (m_blockVisitor)
    {
        return;
    }

    if (!m_skipDefaultValues)
    {
        if (!m_stackArrayStructState.empty())
        {
            const EntryArrayStructState& entry = m_stackArrayStructState.back();
            const int fixedSize = entry.fixedSize;
            const int currentSize = entry.numberOfArrayEntries;
            for (int i = currentSize; i < fixedSize; ++i)
            {
                m_visitor->enterStruct(*field.fieldWithoutArray);
                m_visitor->exitStruct(*field.fieldWithoutArray);
            }
            m_stackArrayStructState.pop_back();
        }
        m_visitor->exitArrayStruct(field);
    }
    else
    {
        if (!m_stackSkipDefault.empty())
        {
            auto& entry = m_stackSkipDefault.back();
            if (entry.enterArrayStructCalled())
            {
                m_visitor->exitArrayStruct(field);
            }
            entry.fieldArrayStruct() = nullptr;
            entry.enterArrayStructCalled() = false;
        }
    }
}

void ParserProcessDefaultValues::enterBool(const MetaField& field, bool value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != false || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterBool(field, value);
    }
}
void ParserProcessDefaultValues::enterInt8(const MetaField& field, std::int8_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterInt8(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt8(const MetaField& field, std::uint8_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterUInt8(field, value);
    }
}
void ParserProcessDefaultValues::enterInt16(const MetaField& field, std::int16_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterInt16(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt16(const MetaField& field, std::uint16_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterUInt16(field, value);
    }
}
void ParserProcessDefaultValues::enterInt32(const MetaField& field, std::int32_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterInt32(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterUInt32(field, value);
    }
}
void ParserProcessDefaultValues::enterInt64(const MetaField& field, std::int64_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterInt64(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterUInt64(field, value);
    }
}
void ParserProcessDefaultValues::enterFloat(const MetaField& field, float value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    if (value != 0 || !m_skipDefaultValues)
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterFloat(field, value);
    }
}
void ParserProcessDefaultValues::enterDouble(const MetaField& field, double value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    if (value != 0 || !m_skipDefaultValues)
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterDouble(field, value);
    }
}
void ParserProcessDefaultValues::enterString(const MetaField& field, std::string&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterString(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterString(field, value, size);
    }
}
void ParserProcessDefaultValues::enterBytes(const MetaField& field, Bytes&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterBytes(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterBytes(field, value, size);
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, std::int32_t value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterEnum(field, value);
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, std::string&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, value);
    if (v != 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    enterEnum(field, std::string(value, size));
}

void ParserProcessDefaultValues::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayBoolMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayBool(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt8(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt8(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt16(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt16(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt16(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt16(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt32(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt32(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt32(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt32(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt64(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayInt64(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt64(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayUInt64(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayFloat(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayFloat(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayDouble(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayDouble(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayStringMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayString(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayBytesMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayBytes(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayEnum(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if (m_blockVisitor)
    {
        return;
    }
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        if (m_skipDefaultValues)
        {
            executeEnterStruct();
        }
        m_visitor->enterArrayEnum(field, value);
    }
}

} // namespace finalmq
