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
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/FmqDefines.h"


#include <assert.h>
#include <memory.h>
#include <iostream>
#include <codecvt>
#include <locale>


namespace finalmq {


    ParserQt::ParserQt(IParserVisitor& visitor, const char* ptr, ssize_t size)
        : m_ptr(ptr)
        , m_size(size)
        , m_visitor(visitor)
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
            m_visitor.notifyError(m_ptr, "typename not found");
            m_visitor.finished();
            return false;
        }

        m_visitor.startStruct(*stru);
        bool res = parseStructIntern(*stru);
        m_visitor.finished();
        return res;
    }


    bool ParserQt::parseStructIntern(const MetaStruct& stru)
    {
        if (!m_ptr || m_size < 0)
        {
            // end of data
            return false;
        }

        const ssize_t numberOfFields = stru.getFieldsSize();
        for (ssize_t i = 0; i < numberOfFields; ++i)
        {
            const MetaField* field = stru.getFieldByIndex(i);
            assert(field);

            bool ok = true;

            switch (field->typeId)
            {
            case MetaTypeId::TYPE_NONE:
                break;
            case MetaTypeId::TYPE_BOOL:
            {
                std::uint8_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterBool(*field, static_cast<bool>(value));
                }
            }
            break;
            case MetaTypeId::TYPE_INT8:
            {
                std::int8_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterInt8(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_UINT8:
            {
                std::uint8_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterUInt8(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_INT16:
            {
                std::int16_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterInt16(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_UINT16:
            {
                std::uint16_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterUInt16(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_INT32:
            {
                std::int32_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterInt32(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_UINT32:
            {
                std::uint32_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterUInt32(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_INT64:
            {
                std::int64_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterInt64(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_UINT64:
            {
                std::uint64_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterUInt64(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_FLOAT:
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
            {
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field->typeName);
                if (!stru)
                {
                    m_visitor.notifyError(m_ptr, "typename not found");
                    m_visitor.finished();
                    ok = false;
                }
                else
                {
                    m_visitor.enterStruct(*field);
                    ok = parseStructIntern(*stru);
                    m_visitor.exitStruct(*field);
                }
            }
            break;
            case MetaTypeId::TYPE_ENUM:
            {
                std::int32_t value = 0;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterEnum(*field, value);
                }
            }
            break;
            case MetaTypeId::TYPE_ARRAY_BOOL:
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
                ok = parseArrayStruct(*field);
                break;
            case MetaTypeId::TYPE_ARRAY_ENUM:
            {
                std::vector<std::int32_t> value;
                ok = parse(value);
                if (ok)
                {
                    m_visitor.enterArrayEnum(*field, std::move(value));
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

            if (!ok)
            {
                return false;
            }
        }

        return (m_ptr != nullptr);
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
            buffer = m_ptr;
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
            m_visitor.notifyError(m_ptr, "typename not found");
            return false;
        }

        const MetaField* fieldWithoutArray = field.fieldWithoutArray;
        assert(fieldWithoutArray);
        m_visitor.enterArrayStruct(field);
        for (std::uint32_t i = 0; i < size && ok; ++i)
        {
            m_visitor.enterStruct(*fieldWithoutArray);
            ok = parseStructIntern(*stru);
            m_visitor.exitStruct(*fieldWithoutArray);
        }
        m_visitor.exitArrayStruct(field);

        return (m_ptr != nullptr);
    }


}   // namespace finalmq
