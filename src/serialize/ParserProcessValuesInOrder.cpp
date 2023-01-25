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

#include <assert.h>
#include <iostream>
#include <algorithm>


namespace finalmq {



ParserProcessValuesInOrder::ParserProcessValuesInOrder(bool skipDefaultValues, IParserVisitor* visitor)
    : ParserProcessDefaultValues(skipDefaultValues, &m_internal)
    , m_internal(visitor)
{

}

void ParserProcessValuesInOrder::setVisitor(IParserVisitor &visitor)
{
    m_internal.setVisitor(visitor);
}

ParserProcessValuesInOrder::Internal::Internal(IParserVisitor* visitor)
    : m_visitor(visitor)
{

}

void ParserProcessValuesInOrder::Internal::setVisitor(IParserVisitor& visitor)
{
    m_visitor = &visitor;
}


// ParserProcessValuesInOrder
void ParserProcessValuesInOrder::Internal::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}


void ParserProcessValuesInOrder::Internal::startStruct(const MetaStruct& stru)
{
    m_stackCalls.emplace_back(CallNode{ std::vector<std::function<void()>>(stru.getFieldsSize() + 2), false });
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls[0] = [this, stru]() { m_visitor->startStruct(stru); };
}


void ParserProcessValuesInOrder::Internal::finished()
{
    assert(m_stackCalls.size() == 1);
    assert(m_currentCalls);
    assert(!m_currentCalls->arr);

    if (m_currentCalls)
    {
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

void ParserProcessValuesInOrder::Internal::enterStruct(const MetaField& field)
{
    ssize_t size = 0;
    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(field);
    if (metaStruct)
    {
        size = metaStruct->getFieldsSize();
    }
    m_stackCalls.emplace_back(CallNode{ std::vector<std::function<void()>>(size + 2), false});
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls[0] = [this, field]() { m_visitor->enterStruct(field); };
}
void ParserProcessValuesInOrder::Internal::exitStruct(const MetaField& field)
{
    if (m_currentCalls == nullptr)
    {
        return;
    }
    m_currentCalls->calls[m_currentCalls->calls.size() - 1] = [this, field]() { m_visitor->exitStruct(field); };
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


void ParserProcessValuesInOrder::Internal::enterStructNull(const MetaField& field)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field]() {m_visitor->enterStruct(field); };
    }
}




void ParserProcessValuesInOrder::Internal::enterArrayStruct(const MetaField& field)
{
    m_stackCalls.emplace_back(CallNode{ {}, true });
    m_currentCalls = &m_stackCalls.back();
    m_currentCalls->calls.emplace_back([this, field]() { m_visitor->enterArrayStruct(field); });
}
void ParserProcessValuesInOrder::Internal::exitArrayStruct(const MetaField& field)
{
    if (m_currentCalls == nullptr)
    {
        return;
    }
    m_currentCalls->calls.emplace_back([this, field]() { m_visitor->exitArrayStruct(field); });
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

void ParserProcessValuesInOrder::Internal::enterBool(const MetaField& field, bool value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterBool(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterInt32(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterUInt32(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterInt64(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterUInt64(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterFloat(const MetaField& field, float value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterFloat(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterDouble(const MetaField& field, double value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterDouble(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterString(const MetaField& field, std::string&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value) }]() mutable {m_visitor->enterString(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        std::string v(value, size);
        m_currentCalls->calls[(field.index + 1)] = [this, field, v{ std::move(v) }]() mutable {m_visitor->enterString(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value) }]() mutable {m_visitor->enterBytes(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        Bytes v(value, value + size);
        m_currentCalls->calls[(field.index + 1)] = [this, field, v{ std::move(v) }]() mutable {m_visitor->enterBytes(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value]() {m_visitor->enterEnum(field, value); };
    }
}
void ParserProcessValuesInOrder::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value) }]() mutable {m_visitor->enterEnum(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v=std::string(value, size)]() mutable {m_visitor->enterEnum(field, std::move(v)); };
    }
}

void ParserProcessValuesInOrder::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value) }]() mutable {m_visitor->enterArrayBoolMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = value]() mutable {m_visitor->enterArrayBoolMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{std::move(value)}]() mutable {m_visitor->enterArrayInt32(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v=std::vector<std::int32_t>(value, value + size)] () mutable {m_visitor->enterArrayInt32(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayUInt32(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v=std::vector<std::uint32_t>(value, value + size)]() mutable {m_visitor->enterArrayUInt32(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayInt64(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = std::vector<std::int64_t>(value, value + size)]() mutable {m_visitor->enterArrayInt64(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayUInt64(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = std::vector<std::uint64_t>(value, value + size)]() mutable {m_visitor->enterArrayUInt64(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayFloat(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = std::vector<float>(value, value + size)]() mutable {m_visitor->enterArrayFloat(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayDouble(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = std::vector<double>(value, value + size)]() mutable {m_visitor->enterArrayDouble(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayStringMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = value]() mutable {m_visitor->enterArrayStringMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayBytesMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = value]() mutable {m_visitor->enterArrayBytesMove(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayEnum(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = std::vector<std::int32_t>(value, value + size)]() mutable {m_visitor->enterArrayEnum(field, std::move(v)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, value{ std::move(value)}]() mutable {m_visitor->enterArrayEnumMove(field, std::move(value)); };
    }
}
void ParserProcessValuesInOrder::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if ((m_currentCalls != nullptr) && ((field.index + 1) < static_cast<int>(m_currentCalls->calls.size())))
    {
        m_currentCalls->calls[(field.index + 1)] = [this, field, v = value]() mutable {m_visitor->enterArrayEnumMove(field, std::move(v)); };
    }
}


}   // namespace finalmq
