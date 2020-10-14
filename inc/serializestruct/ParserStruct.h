#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"
#include "variant/Variant.h"


#include <string>
#include <deque>



class ParserStruct
{
public:
    ParserStruct(IParserVisitor& visitor, const Variant& variant);

    bool parseStruct(const std::string& typeName);

private:
    void parseStruct(const MetaStruct& stru, const Variant& variant);
    void parseStruct(const MetaStruct& stru);
    void processField(const Variant* sub, const MetaField* field);
    void processEmptyField(const MetaField* field);

    IParserVisitor&     m_visitor;
    const Variant&      m_root;
};
