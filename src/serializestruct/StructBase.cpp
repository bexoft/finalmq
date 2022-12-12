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

#include "finalmq/serializestruct/StructBase.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"
#include "finalmq/metadata/MetaData.h"

#include <algorithm>


namespace finalmq {


FieldInfo::FieldInfo(int offset, IArrayStructAdapter* arrayStructAdapter)
    : m_offset(offset)
    , m_arrayStructAdapter(arrayStructAdapter)
{

}

void FieldInfo::setField(const MetaField* field)
{
    m_field = field;
}



StructInfo::StructInfo(const std::string& typeName, const std::string& description, FuncStructBaseFactory factory, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos)
    : m_metaStruct(MetaDataGlobal::instance().addStruct({typeName, description, std::move(fields)}))
    , m_fieldInfos(std::move(fieldInfos))
{
    StructFactoryRegistry::instance().registerFactory(typeName, factory);
    ssize_t size = std::min(m_metaStruct.getFieldsSize(), static_cast<ssize_t>(m_fieldInfos.size()));
    for (ssize_t i = 0; i < size; ++i)
    {
        m_fieldInfos[i].setField(m_metaStruct.getFieldByIndex(i));
    }
}




EnumInfo::EnumInfo(const std::string& typeName, const std::string& description, std::vector<MetaEnumEntry>&& entries)
    : m_metaEnum(MetaDataGlobal::instance().addEnum({typeName, description, std::move(entries)}))
{
}

const MetaEnum& EnumInfo::getMetaEnum() const
{
    return m_metaEnum;
}




StructBase* StructBase::add(ssize_t index)
{
    const StructInfo& structInfo = getStructInfo();
    const FieldInfo* fieldInfo = structInfo.getField(index);
    if (fieldInfo)
    {
        const MetaField* field = fieldInfo->getField();
        if (field)
        {
            if (field->typeId == TYPE_ARRAY_STRUCT)
            {
                IArrayStructAdapter* arrayStructAdapter = fieldInfo->getArrayStructAdapter();
                if (arrayStructAdapter)
                {
                    int offset = fieldInfo->getOffset();
                    char& array = *(reinterpret_cast<char*>(this) + offset);
                    return arrayStructAdapter->add(array);
                }
            }
        }
    }
    return nullptr;
}

}   // namespace finalmq
