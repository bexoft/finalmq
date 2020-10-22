#include "serializestruct/StructBase.h"
#include "metadata/MetaData.h"

#include <algorithm>



FieldInfo::FieldInfo(int offset, IArrayStructAdapter* arrayStructAdapter)
    : m_offset(offset)
    , m_arrayStructAdapter(arrayStructAdapter)
{

}

void FieldInfo::setField(const MetaField* field)
{
    m_field = field;
}



StructInfo::StructInfo(const std::string& typeName, const std::string& description, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos)
    : m_metaStruct(MetaDataGlobal::instance().addStruct({typeName, description, std::move(fields)}))
    , m_fieldInfos(std::move(fieldInfos))
{
    int size = std::min(m_metaStruct.getFieldsSize(), static_cast<int>(m_fieldInfos.size()));
    for (int i = 0; i < size; ++i)
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




StructBase* StructBase::add(int index)
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
                    void* array = reinterpret_cast<void*>(reinterpret_cast<char*>(this) + offset);
                    return arrayStructAdapter->add(array);
                }
            }
        }
    }
    return nullptr;
}

