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

#include "finalmq/serializeqt/ParserQt.h"

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>

#include <assert.h>
#include <memory.h>

#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/serializeqt/Qt.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq//helpers/ModulenameFinalmq.h"

namespace finalmq
{
    
static const std::string FIXED_ARRAY = "fixedarray";

    
ParserQt::ParserQt(IParserVisitor& visitor, const char* ptr, ssize_t size, Mode mode)
    : m_ptr(reinterpret_cast<const std::uint8_t*>(ptr)), m_size(size), m_visitor(visitor), m_mode(mode)
{
}

bool ParserQt::parseStruct(const std::string& typeName)
{
    if (!m_ptr || m_size < 0)
    {
        // end of data
        return false;
    }

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (!stru)
    {
        m_visitor.notifyError(reinterpret_cast<const char*>(m_ptr), "typename not found");
        return false;
    }

    if (m_mode == Mode::WRAPPED_BY_QVARIANTLIST)
    {
        const ssize_t numberOfFields = stru->getFieldsSize();
        std::uint32_t count;
        bool ok = parse(count);
        if (ok && count != numberOfFields)
        {
            m_visitor.notifyError(reinterpret_cast<const char*>(m_ptr), "number of fields does not match");
            return false;
        }
    }

    m_visitor.startStruct(*stru);
    bool res = parseStructIntern(*stru, m_mode != Mode::NONE);
    m_visitor.finished();
    return res;
}

static const std::string KEY_QTTYPE = "qttype";
static const std::string QTTYPE_QVARIANT = "QVariant";

static const std::string QT_CODE = "qtcode";
static const std::string QT_CODE_BYTES = "bytes";

static const std::string ENUM_BITS = "enumbits";
static const std::string BITS_8 = "8";
static const std::string BITS_16 = "16";
static const std::string BITS_32 = "32";

static const std::string ABORTSTRUCT = "abortstruct";
static const std::string ABORT_FALSE = "false";
static const std::string ABORT_TRUE = "true";

static const std::string PNG = "png";
static const std::string PNG_TRUE = "true";

bool ParserQt::parseStructIntern(const MetaStruct& stru, bool wrappedByQVariant)
{
    if (!m_ptr || m_size < 0)
    {
        // end of data
        return false;
    }

    bool ok = true;
    bool abortStruct = false;
    std::int64_t index = INDEX_NOT_AVAILABLE;
    std::int64_t indexOffset = 0;

    const ssize_t numberOfFields = stru.getFieldsSize();
    for (ssize_t i = 0; i < numberOfFields && ok && !abortStruct; ++i)
    {
        if (index >= 0)
        {
            if (indexOffset == 0)
            {
                i = index;
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                --indexOffset;
            }
        }
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field);

        try
        {

            switch (field->typeId)
            {
            case MetaTypeId::TYPE_NONE:
                break;
            case MetaTypeId::TYPE_BOOL:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::uint8_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterBool(*field, static_cast<bool>(value));

                        // check abort
                        const std::string& valueAbort = field->getProperty(ABORTSTRUCT);
                        if (!valueAbort.empty())
                        {
                            if (((valueAbort == ABORT_TRUE) && value) || ((valueAbort == ABORT_FALSE) && !value))
                            {
                                abortStruct = true;
                            }
                        }
                    }
                }
                break;
            case MetaTypeId::TYPE_INT8:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::int8_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterInt8(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_UINT8:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::uint8_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterUInt8(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_INT16:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::int16_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterInt16(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_UINT16:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::uint16_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterUInt16(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_INT32:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::int32_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterInt32(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_UINT32:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::uint32_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterUInt32(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_INT64:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::int64_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterInt64(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_UINT64:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::uint64_t value = 0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterUInt64(*field, value);
                        checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                    }
                }
                break;
            case MetaTypeId::TYPE_FLOAT:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    float value = 0.0f;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterFloat(*field, value);
                    }
                }
                break;
            case MetaTypeId::TYPE_DOUBLE:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    double value = 0.0;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterDouble(*field, value);
                    }
                }
                break;
            case MetaTypeId::TYPE_STRING:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const char* buffer = nullptr;
                    ssize_t size = 0;
                    std::string value;

                    const std::string& code = field->getProperty(QT_CODE);
                    if (code == QT_CODE_BYTES)
                    {
                        ok = parseArrayByte(*field, buffer, size);
                        if (ok)
                        {
                            m_visitor.enterString(*field, buffer, size);
                        }
                    }
                    else
                    {
                        ok = parse(value, field);
                        if (ok)
                        {
                            m_visitor.enterString(*field, value.c_str(), value.size());
                        }
                    }

                    if (buffer)
                    {
                        value = std::string(buffer, &buffer[size]);
                    }
                    checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                }
                break;
            case MetaTypeId::TYPE_BYTES:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const char* buffer = nullptr;
                    ssize_t size = 0;
                    const std::string& png = field->getProperty(PNG);
                    if (png == PNG_TRUE)
                    {
                        ok = parsePng(buffer, size);
                    }
                    else
                    {
                        ok = parseArrayByte(*field, buffer, size);
                    }
                    if (ok)
                    {
                        m_visitor.enterBytes(*field, buffer, size);
                    }
                }
                break;
            case MetaTypeId::TYPE_STRUCT:
            {
                const MetaStruct* stru1 = MetaDataGlobal::instance().getStruct(field->typeName);
                if (!stru1)
                {
                    m_visitor.notifyError(reinterpret_cast<const char*>(m_ptr), "typename not found");
                    ok = false;
                }
                else
                {
                    if (wrappedByQVariant)
                    {
                        // parse QVariant header, if qttype not QVariant
                        const std::string& qttypeField = field->getProperty(KEY_QTTYPE);
                        if ((!qttypeField.empty() && (qttypeField != QTTYPE_QVARIANT)) ||
                            (qttypeField.empty() && (stru1->getProperty(KEY_QTTYPE) != QTTYPE_QVARIANT)))
                        {
                            ok = parseQVariantHeader(*field);
                        }
                    }
                    if (ok)
                    {
                        m_visitor.enterStruct(*field);
                        ok = parseStructIntern(*stru1, false);
                        m_visitor.exitStruct(*field);
                    }
                }
            }
            break;
            case MetaTypeId::TYPE_ENUM:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const MetaEnum* en = MetaDataGlobal::instance().getEnum(*field);
                    if (en)
                    {
                        const std::string& bits = en->getProperty(ENUM_BITS, BITS_32);
                        std::int32_t value = 0;
                        if (bits == BITS_32)
                        {
                            ok = parse(value);
                        }
                        else if (bits == BITS_8)
                        {
                            std::int8_t value8;
                            ok = parse(value8);
                            value = static_cast<std::int32_t>(value8);
                        }
                        else if (bits == BITS_16)
                        {
                            std::int16_t value16;
                            ok = parse(value16);
                            value = static_cast<std::int32_t>(value16);
                        }
                        else
                        {
                            ok = false;
                        }
                        if (ok)
                        {
                            m_visitor.enterEnum(*field, value);

                            // check abort
                            const std::string& valueAbort = field->getProperty(ABORTSTRUCT);
                            if (!valueAbort.empty())
                            {
                                std::string strValue = en->getNameByValue(value);
                                std::vector<std::string> valuesAbort;
                                Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
                                abortStruct = (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end());
                                if (!abortStruct)
                                {
                                    std::string aliasValue = en->getAliasByValue(value);
                                    abortStruct = (std::find(valuesAbort.begin(), valuesAbort.end(), aliasValue) != valuesAbort.end());
                                }
                            }
                            else
                            {
                                checkIndex(*field, value, index, indexOffset);
                            }
                        }
                    }
                    else
                    {
                        ok = false;
                    }
                }
                break;
            case MetaTypeId::TYPE_JSON:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const char* buffer = nullptr;
                    ssize_t size = 0;
                    std::string value;

                    const std::string& code = field->getProperty(QT_CODE);
                    if (code == QT_CODE_BYTES)
                    {
                        ok = parseArrayByte(*field, buffer, size);
                        if (ok)
                        {
                            m_visitor.enterJsonString(*field, buffer, size);
                        }
                    }
                    else
                    {
                        ok = parse(value, field);
                        if (ok)
                        {
                            m_visitor.enterJsonString(*field, value.c_str(), value.size());
                        }
                    }

                    if (buffer)
                    {
                        value = std::string(buffer, &buffer[size]);
                    }
                    checkAbortAndIndex(*field, value, index, indexOffset, abortStruct);
                }
                break;
            case MetaTypeId::TYPE_ARRAY_BOOL:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<bool> array;
                    ok = parseArrayBool(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayBool(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_INT8:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::int8_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayInt8(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_INT16:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::int16_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayInt16(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_UINT16:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::uint16_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayUInt16(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_INT32:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::int32_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayInt32(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_UINT32:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::uint32_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayUInt32(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_INT64:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::int64_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayInt64(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_UINT64:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::uint64_t> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayUInt64(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_FLOAT:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<float> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayFloat(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_DOUBLE:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<double> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayDouble(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_STRING:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<std::string> array;
                    ok = parseArray(*field, array);
                    if (ok)
                    {
                        m_visitor.enterArrayStringMove(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_BYTES:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    std::vector<Bytes> array;
                    const std::string& png = field->getProperty(PNG);
                    if (png == PNG_TRUE)
                    {
                        ok = parseArrayPng(*field, array);
                    }
                    else
                    {
                        ok = parseArray(*field, array);
                    }
                    if (ok)
                    {
                        m_visitor.enterArrayBytesMove(*field, std::move(array));
                    }
                }
                break;
            case MetaTypeId::TYPE_ARRAY_STRUCT:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    ok = parseArrayStruct(*field);
                }
                break;
            case MetaTypeId::TYPE_ARRAY_ENUM:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const MetaEnum* en = MetaDataGlobal::instance().getEnum(*field);
                    if (en)
                    {
                        const std::string& bits = en->getProperty(ENUM_BITS, BITS_32);
                        std::vector<std::int32_t> value;
                        if (bits == BITS_32)
                        {
                            ok = parseArray(*field, value);
                        }
                        else if (bits == BITS_8)
                        {
                            std::vector<std::int8_t> value8;
                            ok = parseArray(*field, value);
                            value.resize(value8.size());
                            for (size_t n = 0; n < value8.size(); ++n)
                            {
                                value[n] = static_cast<std::int32_t>(value8[n]);
                            }
                        }
                        else if (bits == BITS_16)
                        {
                            std::vector<std::int16_t> value16;
                            ok = parseArray(*field, value);
                            value.resize(value16.size());
                            for (size_t n = 0; n < value16.size(); ++n)
                            {
                                value[n] = static_cast<std::int32_t>(value16[n]);
                            }
                        }
                        if (ok)
                        {
                            m_visitor.enterArrayEnum(*field, std::move(value));
                        }
                    }
                    else
                    {
                        ok = false;
                    }
                }
                break;
            case MetaTypeId::OFFSET_ARRAY_FLAG:
                assert(false);
                break;
            default:
                assert(false);
                break;
            }

            if ((index == INDEX_ABORTSTRUCT) || (index >= numberOfFields))
            {
                abortStruct = true;
            }

            if (!ok)
            {
                streamError << "ParserQt error inside struct " << stru.getTypeName() << " at " << field->name;
            }
        }
        catch (std::exception& e)
        {
            streamError << "exception inside struct " << stru.getTypeName() << " at " << field->name << " exception: " << e.what();
            ok = false;
        }
    }

    return ok;
}

bool ParserQt::parse(std::int8_t& value)
{
    return parse(*reinterpret_cast<std::uint8_t*>(&value));
}

bool ParserQt::parse(std::uint8_t& value)
{
    if (m_size >= static_cast<ssize_t>(sizeof(std::uint8_t)))
    {
        assert(m_ptr);
        value = *m_ptr;
        ++m_ptr;
        --m_size;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parse(std::int16_t& value)
{
    return parse(*reinterpret_cast<std::uint16_t*>(&value));
}

bool ParserQt::parse(std::uint16_t& value)
{
    if (m_size >= static_cast<ssize_t>(sizeof(std::uint16_t)))
    {
        assert(m_ptr);
        value = static_cast<std::uint16_t>(static_cast<std::uint16_t>(*m_ptr) << 8);
        ++m_ptr;
        value = static_cast<std::uint16_t>(value | static_cast<std::uint16_t>(*m_ptr));
        ++m_ptr;
        m_size -= sizeof(std::uint16_t);
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parse(std::int32_t& value)
{
    return parse(*reinterpret_cast<std::uint32_t*>(&value));
}

bool ParserQt::parse(std::uint32_t& value)
{
    if (m_size >= static_cast<ssize_t>(sizeof(std::uint32_t)))
    {
        assert(m_ptr);
        value = static_cast<std::uint32_t>(*m_ptr) << 24;
        ++m_ptr;
        value |= static_cast<std::uint32_t>(*m_ptr) << 16;
        ++m_ptr;
        value |= static_cast<std::uint32_t>(*m_ptr) << 8;
        ++m_ptr;
        value |= static_cast<std::uint32_t>(*m_ptr);
        ++m_ptr;
        m_size -= sizeof(std::uint32_t);
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parse(std::int64_t& value)
{
    return parse(*reinterpret_cast<std::uint64_t*>(&value));
}

bool ParserQt::parse(std::uint64_t& value)
{
    if (m_size >= static_cast<ssize_t>(sizeof(std::uint64_t)))
    {
        assert(m_ptr);
        value = static_cast<std::uint64_t>(*m_ptr) << 56;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 48;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 40;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 32;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 24;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 16;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr) << 8;
        ++m_ptr;
        value |= static_cast<std::uint64_t>(*m_ptr);
        ++m_ptr;
        m_size -= sizeof(std::uint64_t);
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parse(float& value)
{
    union
    {
        std::uint64_t v;
        double d;
    } u;
    const bool ok = parse(u.v);
    value = static_cast<float>(u.d);
    return ok;
}

bool ParserQt::parse(double& value)
{
    union
    {
        std::uint64_t v;
        double d;
    } u;
    const bool ok = parse(u.v);
    value = u.d;
    return ok;
}

bool ParserQt::parse(std::string& str, const MetaField* field)
{
    str.clear();

    std::u16string utf16;

    std::uint32_t sizeBytes;
    bool ok = field ? parseSize(*field, sizeBytes) : parse(sizeBytes);
    if (!ok)
    {
        return false;
    }
    if (sizeBytes == 0xFFFFFFFF || sizeBytes == 0)
    {
        return true;
    }

    if (m_size >= static_cast<ssize_t>(sizeBytes))
    {
        std::uint32_t sizeChar = sizeBytes / 2;
        utf16.resize(sizeChar);
        for (std::uint32_t i = 0; i < sizeChar; ++i)
        {
            char16_t c = static_cast<char16_t>(static_cast<char16_t>(*m_ptr) << 8);
            ++m_ptr;
            c = static_cast<char16_t>(c | static_cast<char16_t>(*m_ptr));
            ++m_ptr;
            utf16[i] = c;
        }
        str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(utf16);
        m_size -= sizeBytes;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parse(Bytes& value, const MetaField* field)
{
    value.clear();

    std::uint32_t size;
    bool ok = field ? parseSize(*field, size) : parse(size);
    if (!ok)
    {
        return false;
    }
    if (size == 0xFFFFFFFF || size == 0)
    {
        return true;
    }

    if (m_size >= static_cast<ssize_t>(size))
    {
        assert(m_ptr);
        value = Bytes(m_ptr, m_ptr + size);
        m_ptr += size;
        m_size -= size;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parsePng(Bytes& value)
{
    value.clear();

    std::int32_t available = 0;
    bool ok = parse(available);
    if (!ok)
    {
        return false;
    }
    if (available == 0)
    {
        return true;
    }

    std::uint32_t size;
    ok = getPngSize(size);
    if (!ok)
    {
        return false;
    }
    if (size == 0)
    {
        return true;
    }

    if (m_size >= static_cast<ssize_t>(size))
    {
        assert(m_ptr);
        value = Bytes(m_ptr, m_ptr + size);
        m_ptr += size;
        m_size -= size;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parseArrayByte(const MetaField& field, const char*& buffer, ssize_t& size)
{
    buffer = nullptr;
    size = 0;

    std::uint32_t s;
    bool ok = parseSize(field, s);
    if (!ok)
    {
        return false;
    }
    if (s == 0xFFFFFFFF || s == 0)
    {
        return true;
    }

    size = static_cast<ssize_t>(s);

    if (m_size >= size)
    {
        assert(m_ptr);
        buffer = reinterpret_cast<const char*>(m_ptr);
        m_ptr += size;
        m_size -= size;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}


bool ParserQt::parsePng(const char*& buffer, ssize_t& size)
{
    buffer = nullptr;
    size = 0;

    std::int32_t available = 0;
    bool ok = parse(available);
    if (!ok)
    {
        return false;
    }
    if (available == 0)
    {
        return true;
    }

    std::uint32_t s;
    ok = getPngSize(s);
    if (!ok)
    {
        return false;
    }
    if (s == 0)
    {
        return true;
    }

    size = static_cast<ssize_t>(s);

    if (m_size >= size)
    {
        assert(m_ptr);
        buffer = reinterpret_cast<const char*>(m_ptr);
        m_ptr += size;
        m_size -= size;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}



bool ParserQt::parseArrayBool(const MetaField& field, std::vector<bool>& value)
{
    value.clear();

    std::uint32_t size;
    bool ok = parseSize(field, size);
    if (!ok)
    {
        return false;
    }

    if (size == 0xFFFFFFFF || size == 0)
    {
        return true;
    }

    value.resize(size);

    const ssize_t sizeBytes = (size + 7) / 8;

    if (m_size >= sizeBytes)
    {
        assert(m_ptr);
        for (std::uint32_t i = 0; i < size; ++i)
        {
            const std::uint32_t indexBytes = i / 8;
            const std::uint32_t indexBits = i % 8;
            bool v = (m_ptr[indexBytes] & (1 << indexBits)) ? true : false;
            value[i] = v;
        }
        m_ptr += sizeBytes;
        m_size -= sizeBytes;
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

template<class T>
bool ParserQt::parseArray(const MetaField& field, std::vector<T>& value)
{
    value.clear();

    std::uint32_t size;
    bool ok = parseSize(field, size);
    if (!ok)
    {
        return false;
    }

    if (size == 0)
    {
        return true;
    }

    if (m_size >= static_cast<ssize_t>(size))   // m_size must  be at least size, the exact value cannot be calculated if T is an object 
    {
        value.resize(size);

        for (std::uint32_t i = 0; i < size; ++i)
        {
            ok = parse(value[i]);
            if (!ok)
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
        return false;
    }
}

bool ParserQt::parseArrayStruct(const MetaField& field)
{
    if (m_ptr == nullptr)
    {
        return false;
    }

    std::uint32_t size;
    bool ok = parseSize(field, size);
    if (!ok)
    {
        return false;
    }

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    if (!stru)
    {
        m_visitor.notifyError(reinterpret_cast<const char*>(m_ptr), "typename not found");
        return false;
    }

    const MetaField* fieldWithoutArray = field.fieldWithoutArray;
    assert(fieldWithoutArray);
    m_visitor.enterArrayStruct(field);
    for (std::uint32_t i = 0; i < size && ok; ++i)
    {
        m_visitor.enterStruct(*fieldWithoutArray);
        ok = parseStructIntern(*stru, false);
        m_visitor.exitStruct(*fieldWithoutArray);
    }
    m_visitor.exitArrayStruct(field);

    return (m_ptr != nullptr);
}

bool ParserQt::parseArrayPng(const MetaField& field, std::vector<Bytes>& value)
{
    value.clear();

    std::uint32_t size;
    bool ok = parseSize(field, size);
    if (!ok)
    {
        return false;
    }

    if (size == 0)
    {
        return true;
    }

    value.resize(size);

    for (std::uint32_t i = 0; i < size; ++i)
    {
        ok = parsePng(value[i]);
        if (!ok)
        {
            return false;
        }
    }
    return true;
}

bool ParserQt::parseQVariantHeader(const MetaField& /*field*/)
{
    std::uint32_t typeId;
    bool ok = parse(typeId);
    if (ok)
    {
        std::uint8_t isNull;
        ok = parse(isNull);
    }
    if (ok)
    {
        if (typeId == static_cast<std::uint32_t>(QtType::User))
        {
            Bytes typeName;
            ok = parse(typeName);
        }
    }
    return ok;
}


static const std::string INDEXMODE = "indexmode";
static const std::string INDEXMODE_MAPPING = "mapping";
static const std::string INDEXOFFSET = "indexoffset";


void ParserQt::checkIndex(const MetaField& field, std::int64_t value, std::int64_t& index, std::int64_t& indexOffset)
{
    if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
    {
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
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                int indexMapped = atoi(strIndexMapped.c_str());
                index = field.index + indexOffset + 1 + indexMapped;
            }
        }
        else
        {
            if (value < 0)
            {
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                index = field.index + indexOffset + 1 + value;
            }
        }
    }
}

void ParserQt::checkIndex(const MetaField& field, const std::string& value, std::int64_t& index, std::int64_t& indexOffset)
{
    if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
    {
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
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                int indexMapped = atoi(strIndexMapped.c_str());
                index = field.index + indexOffset + 1 + indexMapped;
            }
        }
        else
        {
            const std::int64_t indexValue = atoll(value.c_str());
            if (indexValue < 0)
            {
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                index = field.index + indexOffset + 1 + indexValue;
            }
        }
    }
}

void ParserQt::checkAbortAndIndex(const MetaField& field, const std::string& value, std::int64_t& index, std::int64_t& indexOffset, bool& abortStruct)
{
    // check abort
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty())
    {
        std::vector<std::string> valuesAbort;
        Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
        abortStruct = (std::find(valuesAbort.begin(), valuesAbort.end(), value) != valuesAbort.end());
    }
    else
    {
        checkIndex(field, value, index, indexOffset);
    }
}

void ParserQt::checkAbortAndIndex(const MetaField& field, std::int64_t value, std::int64_t& index, std::int64_t& indexOffset, bool& abortStruct)
{
    // check abort
    const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
    if (!valueAbort.empty())
    {
        std::string strValue = std::to_string(value);
        std::vector<std::string> valuesAbort;
        Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
        abortStruct = (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end());
    }
    else
    {
        checkIndex(field, value, index, indexOffset);
    }
}


bool ParserQt::getPngSize(std::uint32_t& size)
{
    static const std::uint8_t PNG_PREFIX[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    static const char* IEND = "IEND";

    bool ok = true;
    size = 0;

    if (m_size < 8 || (memcmp(m_ptr, PNG_PREFIX, 8) != 0))
    {
        return false;
    }

    const std::uint8_t* const ptrStore = m_ptr;
    const ssize_t sizeStore = m_size;
    m_ptr += 8;
    m_size -= 8;

    size += 8;

    while (ok)
    {
        std::uint32_t chunkSize = 0;
        ok = parse(chunkSize);
        size += 12 + chunkSize;

        if (!ok)
        {
            break;
        }
        if (m_size < 4)
        {
            ok = false;
            break;
        }
        if (memcmp(m_ptr, IEND, 4) == 0)
        {
            break;
        }
        m_size -= 8 + chunkSize;
        m_ptr += 8 + chunkSize;
    }
    if (ok)
    {
        m_ptr = ptrStore;
        m_size = sizeStore;
    }
    else
    {
        size = 0;
    }
    return ok;
}

bool ParserQt::parseSize(const MetaField& field, std::uint32_t& value)
{
    const std::string& fixedArray = field.getProperty(FIXED_ARRAY);
    if (!fixedArray.empty())
    {
        value = atoi(fixedArray.c_str());
        return true;
    }
    else
    {
        return parse(value);
    }
}


} // namespace finalmq
