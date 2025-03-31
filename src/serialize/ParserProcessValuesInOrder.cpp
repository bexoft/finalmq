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


#include "finalmq/serialize/ParserProcessValuesInOrder.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/variant/Variant.h"


#include <assert.h>
#include <iostream>
#include <algorithm>


namespace finalmq {



ParserProcessValuesInOrder::ParserProcessValuesInOrder(IParserVisitor* visitor)
    : m_visitor(visitor)
{

}

void ParserProcessValuesInOrder::setVisitor(IParserVisitor &visitor)
{
    m_visitor = &visitor;
}



// ParserProcessValuesInOrder
void ParserProcessValuesInOrder::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}


void ParserProcessValuesInOrder::startStruct(const MetaStruct& stru)
{
    m_stackCalls.emplace_back(CallNode{ std::vector<std::function<void()>>(stru.getFieldsSize() + 2), false });
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls[0] = [this, stru]() { m_visitor->startStruct(stru); };
}


void ParserProcessValuesInOrder::finished()
{
    if (m_currentCalls)
    {
        assert(m_stackCalls.size() == 1);
        assert(!m_currentCalls->arr);

        auto& calls = m_currentCalls->calls;
        for (size_t i = 0; i < calls.size(); ++i)
        {
            if (calls[i])
            {
                calls[i]();
            }
        }
    }
    assert(m_visitor);
    m_visitor->finished();
}

void ParserProcessValuesInOrder::enterStruct(const MetaField& field)
{
    ssize_t size = 0;
    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(field);
    if (metaStruct)
    {
        size = metaStruct->getFieldsSize();
    }
    m_stackCalls.emplace_back(CallNode{ std::vector<std::function<void()>>(size + 2), false});
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls[0] = [this, &field]() { m_visitor->enterStruct(field); };
}
void ParserProcessValuesInOrder::exitStruct(const MetaField& field)
{
    if (m_currentCalls == nullptr)
    {
        return;
    }
    m_currentCalls->calls[m_currentCalls->calls.size() - 1] = [this, &field]() { m_visitor->exitStruct(field); };
    std::vector<std::function<void()>> calls = std::move(m_currentCalls->calls);
    auto callsSub = [calls]() {
        for (size_t i = 0; i < calls.size(); ++i)
        {
            if (calls[i])
            {
                calls[i]();
            }
        }
    };

    m_currentCalls = nullptr;
    if (!m_stackCalls.empty())
    {
        m_stackCalls.pop_back();
        if (!m_stackCalls.empty())
        {
            m_currentCalls = &m_stackCalls.back();
            if (!m_currentCalls->arr)
            {
                // no struct array
                assert(field.index >= 0);
                if ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size()))
                {
                    m_currentCalls->calls[field.index + 1] = callsSub;
                }
            }
            else
            {
                // struct array
                m_currentCalls->calls.emplace_back(callsSub);
            }
        }
    }
}


void ParserProcessValuesInOrder::enterStructNull(const MetaField& field)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field]() {m_visitor->enterStructNull(field); };
    }
}




void ParserProcessValuesInOrder::enterArrayStruct(const MetaField& field)
{
    m_stackCalls.emplace_back(CallNode{ {}, true });
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls.emplace_back([this, &field]() { m_visitor->enterArrayStruct(field); });
}
void ParserProcessValuesInOrder::exitArrayStruct(const MetaField& field)
{
    if (m_currentCalls == nullptr)
    {
        return;
    }
    m_currentCalls->calls.emplace_back([this, &field]() { m_visitor->exitArrayStruct(field); });
    auto callsSub = [calls = std::move(m_currentCalls->calls)]() {
        for (size_t i = 0; i < calls.size(); ++i)
        {
            if (calls[i])
            {
                calls[i]();
            }
        }
    };

    m_currentCalls = nullptr;
    if (!m_stackCalls.empty())
    {
        m_stackCalls.pop_back();
        if (!m_stackCalls.empty())
        {
            m_currentCalls = &m_stackCalls.back();
            assert(!m_currentCalls->arr);
            assert(field.index >= 0);
            if ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size()))
            {
                m_currentCalls->calls[field.index + 1] = callsSub;
            }
        }
    }
}

void ParserProcessValuesInOrder::enterBool(const MetaField& field, bool value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterBool(field, value); };
    }
}
void ParserProcessValuesInOrder::enterInt8(const MetaField& field, std::int8_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterInt8(field, value); };
    }
}
void ParserProcessValuesInOrder::enterUInt8(const MetaField& field, std::uint8_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterUInt8(field, value); };
    }
}
void ParserProcessValuesInOrder::enterInt16(const MetaField& field, std::int16_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterInt16(field, value); };
    }
}
void ParserProcessValuesInOrder::enterUInt16(const MetaField& field, std::uint16_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterUInt16(field, value); };
    }
}
void ParserProcessValuesInOrder::enterInt32(const MetaField& field, std::int32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterInt32(field, value); };
    }
}
void ParserProcessValuesInOrder::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterUInt32(field, value); };
    }
}
void ParserProcessValuesInOrder::enterInt64(const MetaField& field, std::int64_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterInt64(field, value); };
    }
}
void ParserProcessValuesInOrder::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterUInt64(field, value); };
    }
}
void ParserProcessValuesInOrder::enterFloat(const MetaField& field, float value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterFloat(field, value); };
    }
}
void ParserProcessValuesInOrder::enterDouble(const MetaField& field, double value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterDouble(field, value); };
    }
}
void ParserProcessValuesInOrder::enterString(const MetaField& field, std::string&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterString(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        std::string v(value, size);
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v{ std::move(v) }]() mutable {m_visitor->enterString(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterBytes(const MetaField& field, Bytes&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterBytes(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        Bytes v(value, value + size);
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v{ std::move(v) }]() mutable {m_visitor->enterBytes(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterEnum(const MetaField& field, std::int32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() {m_visitor->enterEnum(field, value); };
    }
}
void ParserProcessValuesInOrder::enterEnum(const MetaField& field, std::string&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterEnum(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v=std::string(value, size)]() mutable {m_visitor->enterEnum(field, std::move(v)); };
    }
}

void ParserProcessValuesInOrder::enterJsonString(const MetaField& field, std::string&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterJsonString(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterJsonString(const MetaField& field, const char* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        std::string v(value, size);
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v{ std::move(v) }]() mutable {m_visitor->enterJsonString(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterJsonVariant(const MetaField& field, const Variant& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value]() mutable {m_visitor->enterJsonVariant(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterJsonVariantMove(const MetaField& field, Variant&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterJsonVariantMove(field, std::move(value)); };
    }
}


void ParserProcessValuesInOrder::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterArrayBoolMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = value]() mutable {m_visitor->enterArrayBoolMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{std::move(value)}]() mutable {m_visitor->enterArrayInt8(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v=std::vector<std::int8_t>(value, value + size)] () mutable {m_visitor->enterArrayInt8(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterArrayInt16(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::int16_t>(value, value + size)]() mutable {m_visitor->enterArrayInt16(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterArrayUInt16(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::uint16_t>(value, value + size)]() mutable {m_visitor->enterArrayUInt16(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterArrayInt32(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::int32_t>(value, value + size)]() mutable {m_visitor->enterArrayInt32(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value) }]() mutable {m_visitor->enterArrayUInt32(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::uint32_t>(value, value + size)]() mutable {m_visitor->enterArrayUInt32(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayInt64(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::int64_t>(value, value + size)]() mutable {m_visitor->enterArrayInt64(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayUInt64(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::uint64_t>(value, value + size)]() mutable {m_visitor->enterArrayUInt64(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayFloat(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<float>(value, value + size)]() mutable {m_visitor->enterArrayFloat(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayDouble(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<double>(value, value + size)]() mutable {m_visitor->enterArrayDouble(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayStringMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = value]() mutable {m_visitor->enterArrayStringMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayBytesMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = value]() mutable {m_visitor->enterArrayBytesMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayEnum(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = std::vector<std::int32_t>(value, value + size)]() mutable {m_visitor->enterArrayEnum(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, value{ std::move(value)}]() mutable {m_visitor->enterArrayEnumMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, &field, v = value]() mutable {m_visitor->enterArrayEnumMove(field, std::move(v)); };
    }
}


}   // namespace finalmq
