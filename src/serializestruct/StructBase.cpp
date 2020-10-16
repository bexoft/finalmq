#include "serializestruct/StructBase.h"
#include "metadata/MetaData.h"

#include <algorithm>



FieldInfo::FieldInfo(int offset)
    : m_offset(offset)
{

}

void FieldInfo::setField(const MetaField* field)
{
    m_field = field;
}



StructInfo::StructInfo(const std::string& typeName, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos)
    : m_fieldInfos(std::move(fieldInfos))
{
    const MetaStruct& stru = MetaDataGlobal::instance().addStruct({typeName, std::move(fields)});

    int size = std::min(stru.getFieldsSize(), static_cast<int>(m_fieldInfos.size()));
    for (int i = 0; i < size; ++i)
    {
        m_fieldInfos[i].setField(stru.getFieldByIndex(i));
    }
}




EnumInfo::EnumInfo(const std::string& typeName, std::vector<MetaEnumEntry>&& entries)
    : m_metaEnum(MetaDataGlobal::instance().addEnum({typeName, std::move(entries)}))
{
}

const MetaEnum& EnumInfo::getMetaEnum() const
{
    return m_metaEnum;
}


StructBase* StructBase::add(int index)
{
    return nullptr;
}


