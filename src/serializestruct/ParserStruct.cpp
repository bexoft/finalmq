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


#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/metadata/MetaData.h"

#include <assert.h>


namespace finalmq {


ParserStruct::ParserStruct(IParserVisitor& visitor, const StructBase& structBase)
    : m_visitor(visitor)
    , m_root(structBase)
{
}


bool ParserStruct::parseStruct()
{
    MetaField field{MetaTypeId::TYPE_STRUCT, m_root.getStructInfo().getTypeName()};
    m_visitor.enterStruct(field);
    parseStruct(m_root);
    m_visitor.exitStruct(field);
    m_visitor.finished();
    return true;
}



void ParserStruct::processField(const StructBase& structBase, const FieldInfo& fieldInfo)
{
    const MetaField* f = fieldInfo.getField();
    assert(f);
    const MetaField& field = *f;
    switch (field.typeId)
    {
    case TYPE_BOOL:
        m_visitor.enterBool(field, structBase.getValue<bool>(field.index, field.typeId));
        break;
    case TYPE_INT32:
        m_visitor.enterInt32(field, structBase.getValue<std::int32_t>(field.index, field.typeId));
        break;
    case TYPE_UINT32:
        m_visitor.enterUInt32(field, structBase.getValue<std::uint32_t>(field.index, field.typeId));
        break;
    case TYPE_INT64:
        m_visitor.enterInt64(field, structBase.getValue<std::int64_t>(field.index, field.typeId));
        break;
    case TYPE_UINT64:
        m_visitor.enterUInt64(field, structBase.getValue<std::uint64_t>(field.index, field.typeId));
        break;
    case TYPE_FLOAT:
        m_visitor.enterFloat(field, structBase.getValue<float>(field.index, field.typeId));
        break;
    case TYPE_DOUBLE:
        m_visitor.enterDouble(field, structBase.getValue<double>(field.index, field.typeId));
        break;
    case TYPE_STRING:
        {
            const std::string& value = structBase.getValue<std::string>(field.index, field.typeId);
            m_visitor.enterString(field, value.data(), value.size());
        }
        break;
    case TYPE_BYTES:
        {
            const Bytes& value = structBase.getValue<Bytes>(field.index, field.typeId);
            m_visitor.enterBytes(field, value.data(), value.size());
        }
        break;
    case TYPE_STRUCT:
        {
            const StructBase* value = structBase.getData<StructBase>(field.index, field.typeId);
            if (value)
            {
                m_visitor.enterStruct(field);
                parseStruct(*value);
                m_visitor.exitStruct(field);
            }
        }
        break;
    case TYPE_ENUM:
        m_visitor.enterEnum(field, structBase.getValue<std::int32_t>(field.index, field.typeId));
        break;
    case TYPE_ARRAY_BOOL:
        m_visitor.enterArrayBool(field, structBase.getValue<std::vector<bool>>(field.index, field.typeId));
        break;
    case TYPE_ARRAY_INT32:
        {
            const std::vector<std::int32_t>& value = structBase.getValue<std::vector<std::int32_t>>(field.index, field.typeId);
            m_visitor.enterArrayInt32(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_UINT32:
        {
            const std::vector<std::uint32_t>& value = structBase.getValue<std::vector<std::uint32_t>>(field.index, field.typeId);
            m_visitor.enterArrayUInt32(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_INT64:
        {
            const std::vector<std::int64_t>& value = structBase.getValue<std::vector<std::int64_t>>(field.index, field.typeId);
            m_visitor.enterArrayInt64(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_UINT64:
        {
            const std::vector<std::uint64_t>& value = structBase.getValue<std::vector<std::uint64_t>>(field.index, field.typeId);
            m_visitor.enterArrayUInt64(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_FLOAT:
        {
            const std::vector<float>& value = structBase.getValue<std::vector<float>>(field.index, field.typeId);
            m_visitor.enterArrayFloat(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_DOUBLE:
        {
            const std::vector<double>& value = structBase.getValue<std::vector<double>>(field.index, field.typeId);
            m_visitor.enterArrayDouble(field, value.data(), value.size());
        }
        break;
    case TYPE_ARRAY_STRING:
        m_visitor.enterArrayString(field, structBase.getValue<std::vector<std::string>>(field.index, field.typeId));
        break;
    case TYPE_ARRAY_BYTES:
        m_visitor.enterArrayBytes(field, structBase.getValue<std::vector<Bytes>>(field.index, field.typeId));
        break;
    case TYPE_ARRAY_STRUCT:
        {
            m_visitor.enterArrayStruct(field);
            const void* array = structBase.getData<void>(field.index, field.typeId);
            if (array)
            {
                IArrayStructAdapter* arrayStructAdapter = fieldInfo.getArrayStructAdapter();
                if (arrayStructAdapter)
                {
                    const MetaField* fieldWithoutArray = MetaDataGlobal::instance().getArrayField(field);
                    assert(fieldWithoutArray);
                    ssize_t size = arrayStructAdapter->size(array);
                    for (ssize_t i = 0; i < size; ++i)
                    {
                        const StructBase& sub = arrayStructAdapter->at(array, i);
                        m_visitor.enterStruct(*fieldWithoutArray);
                        parseStruct(sub);
                        m_visitor.exitStruct(*fieldWithoutArray);
                    }
                }
            }
            m_visitor.exitArrayStruct(field);
        }
        break;
    case TYPE_ARRAY_ENUM:
        {
            const std::vector<std::int32_t>& value = structBase.getValue<std::vector<std::int32_t>>(field.index, field.typeId);
            m_visitor.enterArrayEnum(field, value.data(), value.size());
        }
        break;
    default:
        assert(false);
        break;
    }
}




void ParserStruct::parseStruct(const StructBase& structBase)
{
    const std::vector<FieldInfo>& fields = structBase.getStructInfo().getFields();
    for (size_t i = 0; i < fields.size(); ++i)
    {
        const FieldInfo& fieldInfo = fields[i];
        const MetaField* field = fieldInfo.getField();
        assert(field);
        const_cast<MetaField*>(field)->index = static_cast<int>(i);
        processField(structBase, fieldInfo);
    }
}

}   // namespace finalmq
