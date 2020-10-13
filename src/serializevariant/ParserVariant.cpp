
#include "serializevariant/ParserVariant.h"
#include "metadata/MetaData.h"
#include "variant/VariantValues.h"
#include "variant/VariantValueList.h"

#include <assert.h>




ParserVariant::ParserVariant(IParserVisitor& visitor, const Variant& variant)
    : m_visitor(visitor)
    , m_root(variant)
{
}





bool ParserVariant::parseStruct(const std::string& typeName)
{
    bool ok = false;
    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (stru)
    {
        MetaField field = {MetaTypeId::TYPE_STRUCT, typeName};
        field.metaStruct = stru;
        m_visitor.enterStruct(field);
        ok = parseStruct(*stru, m_root);
        m_visitor.exitStruct(field);
    }
    else
    {
        m_visitor.notifyError(nullptr, "typename not found");
    }

    m_visitor.finished();
    return ok;
}



void ParserVariant::processField(const Variant* sub, const MetaField* field)
{

    switch (field->typeId)
    {
    case TYPE_BOOL:
        m_visitor.enterBool(*field, *sub);
        break;
    case TYPE_INT32:
        m_visitor.enterInt32(*field, *sub);
        break;
    case TYPE_UINT32:
        m_visitor.enterUInt32(*field, *sub);
        break;
    case TYPE_INT64:
        m_visitor.enterInt64(*field, *sub);
        break;
    case TYPE_UINT64:
        m_visitor.enterUInt64(*field, *sub);
        break;
    case TYPE_FLOAT:
        m_visitor.enterFloat(*field, *sub);
        break;
    case TYPE_DOUBLE:
        m_visitor.enterDouble(*field, *sub);
        break;
    case TYPE_STRING:
        m_visitor.enterString(*field, *sub);
        break;
    case TYPE_BYTES:
        m_visitor.enterBytes(*field, *sub);
        break;
    case TYPE_STRUCT:
        {
            m_visitor.enterStruct(*field);
            const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*field);
            if (stru)
            {
                parseStruct(*stru, *sub);
            }
            else
            {
                m_visitor.notifyError(nullptr, "typename not found");
            }
            m_visitor.exitStruct(*field);
        }
        break;
    case TYPE_ENUM:
        {
            if (sub->getType() == TYPE_STRING)
            {
                std::string value = *sub;
                m_visitor.enterEnum(*field, std::move(value));
            }
            else
            {
                std::int32_t value = *sub;
                m_visitor.enterEnum(*field, value);
            }
        }
        break;
    case TYPE_ARRAY_BOOL:
        m_visitor.enterArrayBool(*field, *sub);
        break;
    case TYPE_ARRAY_INT32:
        m_visitor.enterArrayInt32(*field, *sub);
        break;
    case TYPE_ARRAY_UINT32:
        m_visitor.enterArrayUInt32(*field, *sub);
        break;
    case TYPE_ARRAY_INT64:
        m_visitor.enterArrayInt64(*field, *sub);
        break;
    case TYPE_ARRAY_UINT64:
        m_visitor.enterArrayUInt64(*field, *sub);
        break;
    case TYPE_ARRAY_FLOAT:
        m_visitor.enterArrayFloat(*field, *sub);
        break;
    case TYPE_ARRAY_DOUBLE:
        m_visitor.enterArrayDouble(*field, *sub);
        break;
    case TYPE_ARRAY_STRING:
        m_visitor.enterArrayString(*field, *sub);
        break;
    case TYPE_ARRAY_BYTES:
        m_visitor.enterArrayBytes(*field, *sub);
        break;
    case TYPE_ARRAY_STRUCT:
        {
            m_visitor.enterArrayStruct(*field);
            const VariantList* list = *sub;
            if (list)
            {
                assert(field->fieldWithoutArray);
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*field->fieldWithoutArray);
                if (stru)
                {
                    int size = list->size();
                    for (int i = 0; i < size; ++i)
                    {
                        const Variant& element = list->at(i);
                        m_visitor.enterStruct(*field->fieldWithoutArray);
                        parseStruct(*stru, element);
                        m_visitor.exitStruct(*field->fieldWithoutArray);

                    }
                }
            }
            m_visitor.exitArrayStruct(*field);
        }
        break;
    case TYPE_ARRAY_ENUM:
        {
            if (sub->getType() == TYPE_ARRAY_STRING)
            {
                std::vector<std::string> value = *sub;
                m_visitor.enterArrayEnum(*field, std::move(value));
            }
            else
            {
                std::vector<std::int32_t> value = *sub;
                m_visitor.enterArrayEnum(*field, std::move(value));
            }
        }
        break;
    default:
        assert(false);
        break;
    }
}



void ParserVariant::processEmptyField(const MetaField* field)
{

    switch (field->typeId)
    {
    case TYPE_BOOL:
        m_visitor.enterBool(*field, bool());
        break;
    case TYPE_INT32:
        m_visitor.enterInt32(*field, std::int32_t());
        break;
    case TYPE_UINT32:
        m_visitor.enterUInt32(*field, std::uint32_t());
        break;
    case TYPE_INT64:
        m_visitor.enterInt64(*field, std::int64_t());
        break;
    case TYPE_UINT64:
        m_visitor.enterUInt64(*field, std::uint64_t());
        break;
    case TYPE_FLOAT:
        m_visitor.enterFloat(*field, float());
        break;
    case TYPE_DOUBLE:
        m_visitor.enterDouble(*field, double());
        break;
    case TYPE_STRING:
        m_visitor.enterString(*field, std::string());
        break;
    case TYPE_BYTES:
        m_visitor.enterBytes(*field, Bytes());
        break;
    case TYPE_STRUCT:
        m_visitor.enterStruct(*field);
        m_visitor.exitStruct(*field);
        break;
    case TYPE_ENUM:
        m_visitor.enterEnum(*field, std::int32_t());
        break;
    case TYPE_ARRAY_BOOL:
        m_visitor.enterArrayBool(*field, std::vector<bool>());
        break;
    case TYPE_ARRAY_INT32:
        m_visitor.enterArrayInt32(*field, std::vector<std::int32_t>());
        break;
    case TYPE_ARRAY_UINT32:
        m_visitor.enterArrayUInt32(*field, std::vector<std::uint32_t>());
        break;
    case TYPE_ARRAY_INT64:
        m_visitor.enterArrayInt64(*field, std::vector<std::int64_t>());
        break;
    case TYPE_ARRAY_UINT64:
        m_visitor.enterArrayUInt64(*field, std::vector<std::uint64_t>());
        break;
    case TYPE_ARRAY_FLOAT:
        m_visitor.enterArrayFloat(*field, std::vector<float>());
        break;
    case TYPE_ARRAY_DOUBLE:
        m_visitor.enterArrayDouble(*field, std::vector<double>());
        break;
    case TYPE_ARRAY_STRING:
        m_visitor.enterArrayString(*field, std::vector<std::string>());
        break;
    case TYPE_ARRAY_BYTES:
        m_visitor.enterArrayBytes(*field, std::vector<Bytes>());
        break;
    case TYPE_ARRAY_STRUCT:
        m_visitor.enterArrayStruct(*field);
        m_visitor.exitArrayStruct(*field);
        break;
    case TYPE_ARRAY_ENUM:
        m_visitor.enterArrayEnum(*field, std::vector<std::int32_t>());
        break;
    default:
        assert(false);
        break;
    }
}


bool ParserVariant::parseStruct(const MetaStruct& stru, const Variant& variant)
{
    bool ok = false;
    int size = stru.getFieldsSize();
    for (int i = 0; i < size; ++i)
    {
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field);
        const Variant* sub = variant.getVariant(field->name);
        if (sub)
        {
            processField(sub, field);
        }
        else
        {
            processEmptyField(field);
        }
    }
    return ok;
}
