#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"
#include "variant/Variant.h"


#include <string>
#include <deque>



class ParserVariant
{
public:
    ParserVariant(IParserVisitor& visitor, const Variant& variant);

    bool parseStruct(const std::string& typeName);

private:
    bool parseStruct(const MetaStruct& stru, const Variant& variant);
    void processField(const Variant* sub, const MetaField* field);
    void processEmptyField(const MetaField* field);

    IParserVisitor&     m_visitor;
    const Variant&      m_root;
};
