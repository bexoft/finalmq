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
#include "finalmq/serializeqt/Qt.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/FmqDefines.h"


#include <assert.h>
#include <memory.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>


namespace finalmq {


    ParserQt::ParserQt(IParserVisitor& visitor, const char* ptr, ssize_t size, Mode mode)
        : m_ptr(reinterpret_cast<const std::uint8_t*>(ptr))
        , m_size(size)
        , m_visitor(visitor)
        , m_mode(mode)
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
            m_visitor.finished();
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
                m_visitor.finished();
                return false;
            }
        }

        m_visitor.startStruct(*stru);
        bool res = parseStructIntern(*stru, m_mode != Mode::NONE);
        m_visitor.finished();
        return res;
    }

    static const std::string ENUM_BITS = "enumbits";
    static const std::string BITS_8 = "8";
    static const std::string BITS_16 = "16";
    static const std::string BITS_32 = "32";

    static const std::string ABORTSTRUCT = "abortstruct";
    static const std::string ABORT_FALSE = "false";
    static const std::string ABORT_TRUE = "true";

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

        const ssize_t numberOfFields = stru.getFieldsSize();
        for (ssize_t i = 0; i < numberOfFields && ok && !abortStruct; ++i)
        {
            if (index >= 0)
            {
                i = index;
                index = INDEX_ABORTSTRUCT;
            }
            const MetaField* field = stru.getFieldByIndex(i);
            assert(field);

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
                            if (((valueAbort == ABORT_TRUE) && value) ||
                                ((valueAbort == ABORT_FALSE) && !value))
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                        checkIndex(*field, value, index);
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
                    std::string value;
                    ok = parse(value);
                    if (ok)
                    {
                        m_visitor.enterString(*field, std::move(value));
                    }
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
                    ok = parseArrayByte(buffer, size);
                    if (ok)
                    {
                        m_visitor.enterBytes(*field, buffer, size);
                    }
                }
                break;
            case MetaTypeId::TYPE_STRUCT:
                if (wrappedByQVariant)
                {
                    ok = parseQVariantHeader(*field);
                }
                if (ok)
                {
                    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field->typeName);
                    if (!stru)
                    {
                        m_visitor.notifyError(reinterpret_cast<const char*>(m_ptr), "typename not found");
                        m_visitor.finished();
                        ok = false;
                    }
                    else
                    {
                        m_visitor.enterStruct(*field);
                        ok = parseStructIntern(*stru, false);
                        m_visitor.exitStruct(*field);
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
                        }
                    }
                    else
                    {
                        ok = false;
                    }
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
                    ok = parseArrayBool(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
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
                    ok = parse(array);
                    if (ok)
                    {
                        m_visitor.enterArrayString(*field, std::move(array));
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
                    ok = parse(array);
                    if (ok)
                    {
                        m_visitor.enterArrayBytes(*field, std::move(array));
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
                            ok = parse(value);
                        }
                        else if (bits == BITS_8)
                        {
                            std::vector<std::int8_t> value8;
                            ok = parse(value);
                            value.resize(value8.size());
                            for (size_t i = 0; i < value8.size(); ++i)
                            {
                                value[i] = static_cast<std::int32_t>(value8[i]);
                            }
                        }
                        else if (bits == BITS_16)
                        {
                            std::vector<std::int16_t> value16;
                            ok = parse(value);
                            value.resize(value16.size());
                            for (size_t i = 0; i < value16.size(); ++i)
                            {
                                value[i] = static_cast<std::int32_t>(value16[i]);
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
            value = static_cast<std::uint16_t>(*m_ptr) << 8;
            ++m_ptr;
            value |= static_cast<std::uint16_t>(*m_ptr);
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


    bool ParserQt::parse(std::string& str)
    {
        str.clear();

        std::u16string utf16;

        std::uint32_t sizeBytes;
        bool ok = parse(sizeBytes);
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
                char16_t c = static_cast<char16_t>(*m_ptr) << 8;
                ++m_ptr;
                c |= static_cast<char16_t>(*m_ptr);
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


    bool ParserQt::parse(Bytes& value)
    {
        value.clear();

        std::uint32_t size;
        bool ok = parse(size);
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


    bool ParserQt::parseArrayByte(const char*& buffer, ssize_t& size)
    {
        buffer = nullptr;
        size = 0;

        std::uint32_t s;
        bool ok = parse(s);
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

    bool ParserQt::parseArrayBool(std::vector<bool>& value)
    {
        value.clear();

        std::uint32_t size;
        bool ok = parse(size);
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
    bool ParserQt::parse(std::vector<T>& value)
    {
        value.clear();

        std::uint32_t size;
        bool ok = parse(size);
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
            bool ok = parse(value[i]);
            if (!ok)
            {
                return false;
            }
        }
        return true;
    }

    bool ParserQt::parseArrayStruct(const MetaField& field)
    {
        if (m_ptr == nullptr)
        {
            return false;
        }

        std::uint32_t size;
        bool ok = parse(size);
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

    void ParserQt::checkIndex(const MetaField& field, std::int64_t value, std::int64_t& index)
    {
        if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
        {
            if (value < 0)
            {
                index = INDEX_ABORTSTRUCT;
            }
            else
            {
                index = field.index + 1 + value;
            }
        }
    }


}   // namespace finalmq
