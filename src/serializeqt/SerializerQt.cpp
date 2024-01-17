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


#include "finalmq/serializeqt/SerializerQt.h"
#include "finalmq/serializeqt/Qt.h"
#include "finalmq/serialize/ParserProcessValuesInOrder.h"
#include "finalmq/helpers/Utils.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"


#include <assert.h>
#include <codecvt>
#include <codecvt>
#include <locale>
#include <algorithm>

namespace finalmq {


    SerializerQt::SerializerQt(IZeroCopyBuffer& buffer, Mode mode, int maxBlockSize)
        : ParserConverter()
        , m_internal(buffer, maxBlockSize, mode)
        , m_parserProcessValuesInOrder()
    {
        m_parserProcessValuesInOrder = std::make_unique<ParserProcessValuesInOrder>(false, &m_internal);
        setVisitor(*m_parserProcessValuesInOrder);
    }



    SerializerQt::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize, Mode mode)
        : m_zeroCopybuffer(buffer)
        , m_maxBlockSize(maxBlockSize)
        , m_mode(mode)
    {
    }


    // IParserVisitor
    void SerializerQt::Internal::notifyError(const char* /*str*/, const char* /*message*/)
    {
    }

    void SerializerQt::Internal::startStruct(const MetaStruct& stru)
    {
        if (m_mode == Mode::WRAPPED_BY_QVARIANTLIST)
        {
            reserveSpace(sizeof(std::uint32_t));
            const std::uint32_t count = static_cast<std::uint32_t>(stru.getFieldsSize());
            serialize(count);
        }

        m_levelState.push_back(LevelState());
    }


    void SerializerQt::Internal::finished()
    {
        resizeBuffer();
        m_levelState.pop_back();
    }


    void SerializerQt::Internal::enterStruct(const MetaField& field)
    {
        assert(!m_levelState.empty());
        bool abortStruct = m_levelState.back().abortStruct;
        std::int64_t index = m_levelState.back().index;
        m_levelState.push_back(LevelState());
        m_levelState.back().abortStruct = abortStruct;

        if (abortStruct || (index != INDEX_NOT_AVAILABLE && index != field.index))
        {
            return;
        }

        LevelState& levelState = m_levelState.back();

        assert(field.typeId == MetaTypeId::TYPE_STRUCT);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }

        if (levelState.arrayStructCounter >= 0)
        {
            ++levelState.arrayStructCounter;
        }
    }

    void SerializerQt::Internal::exitStruct(const MetaField& /*field*/)
    {
        m_levelState.pop_back();
    }

    void SerializerQt::Internal::enterStructNull(const MetaField& /*field*/)
    {
    }


    void SerializerQt::Internal::enterArrayStruct(const MetaField& field)
    {
        assert(!m_levelState.empty());
        bool abortStruct = m_levelState.back().abortStruct;
        std::int64_t index = m_levelState.back().index;
        m_levelState.push_back(LevelState());
        m_levelState.back().abortStruct = abortStruct;

        if (abortStruct || (index != INDEX_NOT_AVAILABLE && index != field.index))
        {
            return;
        }

        LevelState& levelState = m_levelState.back();

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_STRUCT);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }

        reserveSpace(sizeof(std::uint32_t));
        levelState.arrayStructCounterBuffer = m_buffer;
        levelState.arrayStructCounter = 0;
        serialize(levelState.arrayStructCounter);
        levelState.arrayStructCounter = 0;
    }

    void SerializerQt::Internal::exitArrayStruct(const MetaField& /*field*/)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();

        if (levelState.abortStruct)
        {
            m_levelState.pop_back();
            return;
        }

        if (levelState.arrayStructCounterBuffer)
        {
            char* buffer = m_buffer;
            m_buffer = levelState.arrayStructCounterBuffer;
            serialize(levelState.arrayStructCounter);
            m_buffer = buffer;
            levelState.arrayStructCounter = -1;
        }

        m_levelState.pop_back();
    }

    static const std::string ABORTSTRUCT = "abortstruct";
    static const std::string ABORT_FALSE = "false";
    static const std::string ABORT_TRUE = "true";

    void SerializerQt::Internal::enterBool(const MetaField& field, bool value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_BOOL);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint8_t));
        serialize(static_cast<std::uint8_t>(value ? 1 : 0));

        // check abort
        const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
        if (!valueAbort.empty())
        {
            if (((valueAbort == ABORT_TRUE) && value) ||
                ((valueAbort == ABORT_FALSE) && !value))
            {
                levelState.abortStruct = true;
            }
        }
    }

    void SerializerQt::Internal::enterInt8(const MetaField& field, std::int8_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_INT8);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int8_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_UINT8);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint8_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterInt16(const MetaField& field, std::int16_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_INT16);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int16_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_UINT16);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint16_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterInt32(const MetaField& field, std::int32_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_INT32);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_UINT32);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint32_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterInt64(const MetaField& field, std::int64_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_INT64);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int64_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_UINT64);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint64_t));
        serialize(value);
        checkIndex(field, value);
    }

    void SerializerQt::Internal::enterFloat(const MetaField& field, float value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_FLOAT);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(double));
        serialize(value);
    }

    void SerializerQt::Internal::enterDouble(const MetaField& field, double value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_DOUBLE);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(double));
        serialize(value);
    }

    void SerializerQt::Internal::enterString(const MetaField& field, std::string&& value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_STRING);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint32_t) + 2 * value.size());
        serializeString(value);
    }

    void SerializerQt::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_STRING);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint32_t) + 2 * size);
        serializeString(value, size);
    }

    void SerializerQt::Internal::enterBytes(const MetaField& field, Bytes&& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_BYTES);
        enterBytes(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_BYTES);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::uint32_t) + size);
        serialize(value, size);
    }

    static const std::string QT_ENUM_BITS = "enumbits";
    static const std::string BITS_8 = "8";
    static const std::string BITS_16 = "16";
    static const std::string BITS_32 = "32";

    void SerializerQt::Internal::enterEnum(const MetaField& field, std::int32_t value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ENUM);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t));
        const std::string* bits = &field.getProperty(QT_ENUM_BITS);
        const MetaEnum* en = MetaDataGlobal::instance().getEnum(field.typeName);
        if (en == nullptr)
        {
            streamError << "enum not found " << field.typeName;
        }
        if (bits->empty())
        {
            if (en)
            {
                bits = &en->getProperty(QT_ENUM_BITS);
            }
        }
        if (*bits == BITS_8)
        {
            serialize(static_cast<std::int8_t>(value));
        }
        else if (*bits == BITS_16)
        {
            serialize(static_cast<std::int16_t>(value));
        }
        else
        {
            serialize(value);
        }

        // check abort
        const std::string& valueAbort = field.getProperty(ABORTSTRUCT);
        if (!valueAbort.empty() && en)
        {
            std::string strValue = en->getNameByValue(value);
            std::vector<std::string> valuesAbort;
            Utils::split(valueAbort, 0, valueAbort.size(), '|', valuesAbort);
            if (std::find(valuesAbort.begin(), valuesAbort.end(), strValue) != valuesAbort.end())
            {
                levelState.abortStruct = true;
            }
            else
            {
                std::string aliasValue = en->getAliasByValue(value);
                if (std::find(valuesAbort.begin(), valuesAbort.end(), aliasValue) != valuesAbort.end())
                {
                    levelState.abortStruct = true;
                }
            }
        }
    }

    void SerializerQt::Internal::enterEnum(const MetaField& field, std::string&& value)
    {
        std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(field, value);
        enterEnum(field, enumValue);
    }

    void SerializerQt::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
    {
        enterEnum(field, std::string(value, size));
    }

    void SerializerQt::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
    {
        enterArrayBool(field, value);
    }

    void SerializerQt::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_BOOL);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        const ssize_t sizeBytes = (value.size() + 7) / 8;
        reserveSpace(sizeof(std::int32_t) + sizeBytes);
        serializeArrayBool(value);
    }

    void SerializerQt::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
    {
        enterArrayInt8(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT8);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int8_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
    {
        enterArrayInt16(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT16);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int16_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
    {
        enterArrayUInt16(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT16);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint16_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
    {
        enterArrayInt32(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT32);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int32_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
    {
        enterArrayUInt32(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT32);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint32_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
    {
        enterArrayInt64(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT64);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int64_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
    {
        enterArrayUInt64(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT64);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint64_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
    {
        enterArrayFloat(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_FLOAT);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(double));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
    {
        enterArrayDouble(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_DOUBLE);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(double));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
    {
        enterArrayString(field, value);
    }

    void SerializerQt::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_STRING);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        std::uint32_t len = sizeof(std::uint32_t);
        for (size_t i = 0; i < value.size(); ++i)
        {
            len += sizeof(std::uint32_t) + 2 * static_cast<std::uint32_t>(value.size());
        }
        reserveSpace(len);
        serializeArrayString(value);
    }

    void SerializerQt::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
    {
        enterArrayBytes(field, value);
    }

    void SerializerQt::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_BYTES);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        std::uint32_t len = sizeof(std::uint32_t);
        for (size_t i = 0; i < value.size(); ++i)
        {
            len += sizeof(std::uint32_t) + static_cast<std::uint32_t>(value.size());
        }
        reserveSpace(len);
        serializeArrayBytes(value);
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
    {
        enterArrayEnum(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
        if (isWrappedByQVariant())
        {
            serializeQVariantHeader(field);
        }
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int32_t));

        const std::string* bits = &field.getProperty(QT_ENUM_BITS);
        if (bits->empty())
        {
            const MetaEnum* en = MetaDataGlobal::instance().getEnum(field.typeName);
            if (en)
            {
                bits = &en->getProperty(QT_ENUM_BITS);
            }
        }
        if (*bits == BITS_8)
        {
            std::vector<std::int8_t> value8;
            value8.resize(size);
            for (ssize_t i = 0; i < size; ++i)
            {
                value8[i] = static_cast<std::int8_t>(value[i]);
            }
            serialize(value8.data(), size);
        }
        else if (*bits == BITS_16)
        {
            std::vector<std::int16_t> value16;
            value16.resize(size);
            for (ssize_t i = 0; i < size; ++i)
            {
                value16[i] = static_cast<std::int16_t>(value[i]);
            }
            serialize(value16.data(), size);
        }
        else
        {
            serialize(value, size);
        }
    }

    void SerializerQt::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
    {
        enterArrayEnum(field, value);
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
    {
        assert(!m_levelState.empty());
        LevelState& levelState = m_levelState.back();
        if ((levelState.abortStruct) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.index))
        {
            return;
        }

        std::vector<std::int32_t> enums;
        enums.resize(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            const std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(field, value[i]);
            enums[i] = enumValue;
        }
        enterArrayEnum(field, enums.data(), enums.size());
    }


    void SerializerQt::Internal::serialize(char value)
    {
        serialize(static_cast<std::uint8_t>(value));
    }

    void SerializerQt::Internal::serialize(std::int8_t value)
    {
        serialize(static_cast<std::uint8_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint8_t value)
    {
        *m_buffer = value;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int16_t value)
    {
        serialize(static_cast<std::uint16_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint16_t value)
    {
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int32_t value)
    {
        serialize(static_cast<std::uint32_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint32_t value)
    {
        *m_buffer = (value >> 24) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 16) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int64_t value)
    {
        serialize(static_cast<std::uint64_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint64_t value)
    {
        *m_buffer = (value >> 56) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 48) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 40) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 32) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 24) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 16) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(float value)
    {
        union
        {
            std::uint64_t v;
            double d;
        } u;
        u.d = static_cast<double>(value);
        serialize(u.v);
    }

    void SerializerQt::Internal::serialize(double value)
    {
        union
        {
            std::uint64_t v;
            double d;
        } u;
        u.d = value;
        serialize(u.v);
    }

    void SerializerQt::Internal::serializeString(const std::string& value)
    {
        std::u16string u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(value);
        assert(u16.size() <= value.size());
        serialize((std::int16_t*)(u16.data()), u16.size(), true);
    }
        
    void SerializerQt::Internal::serializeString(const char* value, ssize_t size)
    {
        serializeString(std::string(value, value + size));
    }

    template<class T>
    void SerializerQt::Internal::serialize(const T* value, ssize_t size, bool sizeTimesTwo)
    {
        std::uint32_t s = static_cast<std::uint32_t>(size);
        if (sizeTimesTwo)
        {
            serialize(s * 2);
        }
        else
        {
            serialize(s);
        }

        for (std::uint32_t i = 0; i < s; ++i)
        {
            serialize(value[i]);
        }
    }

    void SerializerQt::Internal::serializeArrayBool(const std::vector<bool>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        const ssize_t sizeBytes = (value.size() + 7) / 8;
        for (ssize_t i = 0; i < sizeBytes; ++i)
        {
            m_buffer[i] = 0;
        }
        for (std::uint32_t i = 0; i < s; ++i)
        {
            if (value[i])
            {
                const std::uint32_t indexBytes = i / 8;
                const std::uint32_t indexBits = i % 8;
                m_buffer[indexBytes] |= (1 << indexBits);
            }
        }
        m_buffer += sizeBytes;
    }

    void SerializerQt::Internal::serializeArrayString(const std::vector<std::string>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        for (std::uint32_t i = 0; i < s; ++i)
        {
            serializeString(value[i]);
        }
    }

    void SerializerQt::Internal::serializeArrayBytes(const std::vector<Bytes>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        for (std::uint32_t i = 0; i < s; ++i)
        {
            serialize(value[i].data(), value[i].size());
        }
    }



    void SerializerQt::Internal::serializeQVariantHeader(const MetaField& field)
    {
        std::uint32_t typeId;
        std::string typeName;
        getQVariantType(field, typeId, typeName);

        reserveSpace(sizeof(std::int32_t) + sizeof(std::int8_t) + ((typeId == static_cast<std::uint32_t>(QtType::User)) ? typeName.size() : 0));
        
        serialize(typeId);
        serialize(static_cast<std::uint8_t>(0));

        if (typeId == static_cast<std::uint32_t>(QtType::User))
        {
            serialize(typeName.data(), typeName.size() + 1);
        }
    }

    bool SerializerQt::Internal::isWrappedByQVariant() const
    {
        return (m_levelState.size() == 1 && (m_mode != Mode::NONE));
    }

    bool SerializerQt::Internal::getQVariantTypeFromMetaTypeId(const MetaField& field, std::uint32_t& typeId, std::string& typeName)
    {
        typeId = static_cast<std::uint32_t>(QtType::UnknownType);
        typeName.clear();

        switch (field.typeId)
        {
            case MetaTypeId::TYPE_BOOL:
                typeId = static_cast<std::uint32_t>(QtType::Bool);
                break;
            case MetaTypeId::TYPE_INT8:
                typeId = static_cast<std::uint32_t>(QtType::Char);
                break;
            case MetaTypeId::TYPE_UINT8:
                typeId = static_cast<std::uint32_t>(QtType::UChar);
                break;
            case MetaTypeId::TYPE_INT16:
                typeId = static_cast<std::uint32_t>(QtType::Short);
                break;
            case MetaTypeId::TYPE_UINT16:
                typeId = static_cast<std::uint32_t>(QtType::UShort);
                break;
            case MetaTypeId::TYPE_INT32:
                typeId = static_cast<std::uint32_t>(QtType::Int);
                break;
            case MetaTypeId::TYPE_UINT32:
                typeId = static_cast<std::uint32_t>(QtType::UInt);
                break;
            case MetaTypeId::TYPE_INT64:
                typeId = static_cast<std::uint32_t>(QtType::LongLong);
                break;
            case MetaTypeId::TYPE_UINT64:
                typeId = static_cast<std::uint32_t>(QtType::ULongLong);
                break;
            case MetaTypeId::TYPE_FLOAT:
                typeId = static_cast<std::uint32_t>(QtType::Float);
                break;
            case MetaTypeId::TYPE_DOUBLE:
                typeId = static_cast<std::uint32_t>(QtType::Double);
                break;
            case MetaTypeId::TYPE_STRING:
                typeId = static_cast<std::uint32_t>(QtType::QString);
                break;
            case MetaTypeId::TYPE_BYTES:
                typeId = static_cast<std::uint32_t>(QtType::QByteArray);
                break;
            case MetaTypeId::TYPE_STRUCT:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = Utils::replaceAll(field.typeName, ".", "::");
                break;
            case MetaTypeId::TYPE_ENUM:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = Utils::replaceAll(field.typeName, ".", "::");
                break;
            case MetaTypeId::TYPE_ARRAY_BOOL:
                typeId = static_cast<std::uint32_t>(QtType::QBitArray);
                break;
            case MetaTypeId::TYPE_ARRAY_INT8:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<char>";
                break;
            case MetaTypeId::TYPE_ARRAY_INT16:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<short>";
                break;
            case MetaTypeId::TYPE_ARRAY_UINT16:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<ushort>";
                break;
            case MetaTypeId::TYPE_ARRAY_INT32:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<int>";
                break;
            case MetaTypeId::TYPE_ARRAY_UINT32:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<uint>";
                break;
            case MetaTypeId::TYPE_ARRAY_INT64:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<qlonglong>";
                break;
            case MetaTypeId::TYPE_ARRAY_UINT64:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<uqlonglong>";
                break;
            case MetaTypeId::TYPE_ARRAY_FLOAT:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<float>";
                break;
            case MetaTypeId::TYPE_ARRAY_DOUBLE:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QVector<double>";
                break;
            case MetaTypeId::TYPE_ARRAY_STRING:
                typeId = static_cast<std::uint32_t>(QtType::QStringList);
                break;
            case MetaTypeId::TYPE_ARRAY_BYTES:
                typeId = static_cast<std::uint32_t>(QtType::QByteArrayList);
                break;
            case MetaTypeId::TYPE_ARRAY_STRUCT:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QList<" + Utils::replaceAll(field.typeName, ".", "::") + ">";
                break;
            case MetaTypeId::TYPE_ARRAY_ENUM:
                typeId = static_cast<std::uint32_t>(QtType::User);
                typeName = "QList<" + Utils::replaceAll(field.typeName, ".", "::") + ">";
                break;
            default:
                break;
        }

        return (typeId != static_cast<std::uint32_t>(QtType::UnknownType));
    }

    std::uint32_t SerializerQt::Internal::getTypeIdByName(const std::string& typeName)
    {
        std::unordered_map<std::string, std::uint32_t> table{ 
            { "bool", static_cast<std::uint32_t>(QtType::Bool) },
            { "int", static_cast<std::uint32_t>(QtType::Int) },
            { "uint", static_cast<std::uint32_t>(QtType::UInt) },
            { "qlonglong", static_cast<std::uint32_t>(QtType::LongLong) },
            { "qulonglong", static_cast<std::uint32_t>(QtType::ULongLong) },
            { "double", static_cast<std::uint32_t>(QtType::Double) },
            { "long", static_cast<std::uint32_t>(QtType::Long) },
            { "short", static_cast<std::uint32_t>(QtType::Short) },
            { "char", static_cast<std::uint32_t>(QtType::Char) },
            { "ulong", static_cast<std::uint32_t>(QtType::ULong) },
            { "ushort", static_cast<std::uint32_t>(QtType::UShort) },
            { "uchar", static_cast<std::uint32_t>(QtType::UChar) },
            { "float", static_cast<std::uint32_t>(QtType::Float) },
            { "QChar", static_cast<std::uint32_t>(QtType::QChar) },
            { "QString", static_cast<std::uint32_t>(QtType::QString) },
            { "QStringList", static_cast<std::uint32_t>(QtType::QStringList) },
            { "QByteArray", static_cast<std::uint32_t>(QtType::QByteArray) },
            { "QBitArray", static_cast<std::uint32_t>(QtType::QBitArray) },
            { "QDate", static_cast<std::uint32_t>(QtType::QDate) },
            { "QTime", static_cast<std::uint32_t>(QtType::QTime) },
            { "QDateTime", static_cast<std::uint32_t>(QtType::QDateTime) },
            { "QUrl", static_cast<std::uint32_t>(QtType::QUrl) },
            { "QLocale", static_cast<std::uint32_t>(QtType::QLocale) },
            { "QRect", static_cast<std::uint32_t>(QtType::QRect) },
            { "QRectF", static_cast<std::uint32_t>(QtType::QRectF) },
            { "QSize", static_cast<std::uint32_t>(QtType::QSize) },
            { "QSizeF", static_cast<std::uint32_t>(QtType::QSizeF) },
            { "QLine", static_cast<std::uint32_t>(QtType::QLine) },
            { "QLineF", static_cast<std::uint32_t>(QtType::QLineF) },
            { "QPoint", static_cast<std::uint32_t>(QtType::QPoint) },
            { "QPointF", static_cast<std::uint32_t>(QtType::QPointF) },
            { "QRegExp", static_cast<std::uint32_t>(QtType::QRegExp) },
            { "QEasingCurve", static_cast<std::uint32_t>(QtType::QEasingCurve) },
            { "QUuid", static_cast<std::uint32_t>(QtType::QUuid) },
            { "QVariant", static_cast<std::uint32_t>(QtType::QVariant) },
            { "QModelIndex", static_cast<std::uint32_t>(QtType::QModelIndex) },
            { "QPersistentModelIndex", static_cast<std::uint32_t>(QtType::QPersistentModelIndex) },
            { "QRegularExpression", static_cast<std::uint32_t>(QtType::QRegularExpression) },
            { "QJsonValue", static_cast<std::uint32_t>(QtType::QJsonValue) },
            { "QJsonObject", static_cast<std::uint32_t>(QtType::QJsonObject) },
            { "QJsonArray", static_cast<std::uint32_t>(QtType::QJsonArray) },
            { "QJsonDocument", static_cast<std::uint32_t>(QtType::QJsonDocument) },
            { "QByteArrayList", static_cast<std::uint32_t>(QtType::QByteArrayList) },
            { "QObjectStar", static_cast<std::uint32_t>(QtType::QObjectStar) },
            { "SChar", static_cast<std::uint32_t>(QtType::SChar) },
            { "Void", static_cast<std::uint32_t>(QtType::Void) },
            { "Nullptr", static_cast<std::uint32_t>(QtType::Nullptr) },
            { "QVariantMap", static_cast<std::uint32_t>(QtType::QVariantMap) },
            { "QVariantList", static_cast<std::uint32_t>(QtType::QVariantList) },
            { "QVariantHash", static_cast<std::uint32_t>(QtType::QVariantHash) },
            { "QCborSimpleType", static_cast<std::uint32_t>(QtType::QCborSimpleType) },
            { "QCborValue", static_cast<std::uint32_t>(QtType::QCborValue) },
            { "QCborArray", static_cast<std::uint32_t>(QtType::QCborArray) },
            { "QCborMap", static_cast<std::uint32_t>(QtType::QCborMap) },
            { "QFont", static_cast<std::uint32_t>(QtType::QFont) },
            { "QPixmap", static_cast<std::uint32_t>(QtType::QPixmap) },
            { "QBrush", static_cast<std::uint32_t>(QtType::QBrush) },
            { "QColor", static_cast<std::uint32_t>(QtType::QColor) },
            { "QPalette", static_cast<std::uint32_t>(QtType::QPalette) },
            { "QIcon", static_cast<std::uint32_t>(QtType::QIcon) },
            { "QImage", static_cast<std::uint32_t>(QtType::QImage) },
            { "QPolygon", static_cast<std::uint32_t>(QtType::QPolygon) },
            { "QRegion", static_cast<std::uint32_t>(QtType::QRegion) },
            { "QBitmap", static_cast<std::uint32_t>(QtType::QBitmap) },
            { "QCursor", static_cast<std::uint32_t>(QtType::QCursor) },
            { "QKeySequence", static_cast<std::uint32_t>(QtType::QKeySequence) },
            { "QPen", static_cast<std::uint32_t>(QtType::QPen) },
            { "QTextLength", static_cast<std::uint32_t>(QtType::QTextLength) },
            { "QTextFormat", static_cast<std::uint32_t>(QtType::QTextFormat) },
            { "QMatrix", static_cast<std::uint32_t>(QtType::QMatrix) },
            { "QTransform", static_cast<std::uint32_t>(QtType::QTransform) },
            { "QMatrix4x4", static_cast<std::uint32_t>(QtType::QMatrix4x4) },
            { "QVector2D", static_cast<std::uint32_t>(QtType::QVector2D) },
            { "QVector3D", static_cast<std::uint32_t>(QtType::QVector3D) },
            { "QVector4D", static_cast<std::uint32_t>(QtType::QVector4D) },
            { "QQuaternion", static_cast<std::uint32_t>(QtType::QQuaternion) },
            { "QPolygonF", static_cast<std::uint32_t>(QtType::QPolygonF) },
            { "QColorSpace", static_cast<std::uint32_t>(QtType::QColorSpace) },
            { "QSizePolicy", static_cast<std::uint32_t>(QtType::QSizePolicy) },
        };

        const auto it = table.find(typeName);
        if (it != table.end())
        {
            return it->second;
        }
        return static_cast<std::uint32_t>(QtType::User);
    }

    void SerializerQt::Internal::getQVariantType(const MetaField& field, std::uint32_t& typeId, std::string& typeName)
    {
        typeName.clear();

        static const std::string KEY_QTTYPE = "qttype";
        typeName = field.getProperty(KEY_QTTYPE);

        if (typeName.empty())
        {
            if (field.typeId == MetaTypeId::TYPE_STRUCT)
            {
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
                if (stru)
                {
                    typeName = stru->getProperty(KEY_QTTYPE);
                }
            }
            else if (field.typeId == MetaTypeId::TYPE_ENUM)
            {
                const MetaEnum* enu = MetaDataGlobal::instance().getEnum(field);
                if (enu)
                {
                    typeName = enu->getProperty(KEY_QTTYPE);
                }
            }
        }

        if (!typeName.empty())
        {
            typeId = getTypeIdByName(typeName);
        }
        else
        {
            getQVariantTypeFromMetaTypeId(field, typeId, typeName);
        }
    }

    void SerializerQt::Internal::reserveSpace(ssize_t space)
    {
        ssize_t sizeRemaining = m_bufferEnd - m_buffer;
        if (sizeRemaining < space)
        {
            if (m_buffer != nullptr)
            {
                ssize_t size = m_buffer - m_bufferStart;
                assert(size >= 0);
                m_zeroCopybuffer.downsizeLastBuffer(size);
            }

            const ssize_t sizeRemainingZeroCopyBuffer = m_zeroCopybuffer.getRemainingSize();
            ssize_t sizeNew = m_maxBlockSize;
            if (space <= sizeRemainingZeroCopyBuffer)
            {
                sizeNew = sizeRemainingZeroCopyBuffer;
            }
            sizeNew = std::max(sizeNew, space);
            char* bufferStartNew = m_zeroCopybuffer.addBuffer(sizeNew);

            m_bufferStart = bufferStartNew;
            m_bufferEnd = m_bufferStart + sizeNew;
            m_buffer = m_bufferStart;
        }
    }

    void SerializerQt::Internal::resizeBuffer()
    {
        if (m_buffer != nullptr)
        {
            ssize_t size = m_buffer - m_bufferStart;
            assert(size >= 0);
            m_zeroCopybuffer.downsizeLastBuffer(size);
            m_bufferStart = nullptr;
            m_bufferEnd = nullptr;
            m_buffer = nullptr;
        }
    }

    void SerializerQt::Internal::checkIndex(const MetaField& field, std::int64_t value)
    {
        if ((field.flags & MetaFieldFlags::METAFLAG_INDEX) != 0)
        {
            assert(!m_levelState.empty());
            LevelState& levelState = m_levelState.back();
            if (value < 0)
            {
                levelState.abortStruct = true;
            }
            else
            {
                levelState.index = field.index + 1 + value;
            }
        }
    }

}   // namespace finalmq
