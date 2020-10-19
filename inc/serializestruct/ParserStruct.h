#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"
#include "StructBase.h"


#include <string>
#include <deque>



class ParserStruct
{
public:
    ParserStruct(IParserVisitor& visitor, const StructBase& structBase);

    bool parseStruct();

private:
    void parseStruct(const StructBase& structBase);
    void processField(const StructBase& structBase, const FieldInfo& fieldInfo);

    IParserVisitor&     m_visitor;
    const StructBase&   m_root;
};
