#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"
#include "json/JsonParser.h"


#include <string>
#include <deque>



class ParserJson : public IJsonParserVisitor
{
public:
    ParserJson(IParserVisitor& visitor, const char* ptr, int size = CHECK_ON_ZEROTERM);

    bool parseStruct(const std::string& typeName);

private:
    // IJsonParserVisitor
    virtual void syntaxError(const char* str, const char* message) override;
    virtual void enterNull() override;
    virtual void enterBool(bool value) override;
    virtual void enterInt32(std::int32_t value) override;
    virtual void enterUInt32(std::uint32_t value) override;
    virtual void enterInt64(std::int64_t value) override;
    virtual void enterUInt64(std::uint64_t value) override;
    virtual void enterDouble(double value) override;
    virtual void enterString(const char* value, int size) override;
    virtual void enterString(std::string&& value) override;
    virtual void enterArray() override;
    virtual void exitArray() override;
    virtual void enterObject() override;
    virtual void exitObject() override;
    virtual void enterKey(const char* key, int size) override;
    virtual void enterKey(std::string&& key) override;
    virtual void finished() override;


    template<class T>
    void enterNumber(T value);

    struct State
    {
        explicit State(const MetaField* f)
            : field(f)
        {
        }
        const MetaField* field;
    };

    const char*         m_ptr = nullptr;
    int                 m_size = 0;
    IParserVisitor&     m_visitor;
    JsonParser          m_parser;

    std::deque<State>   m_stack;
    const MetaStruct*   m_structCurrent = nullptr;
    const MetaField*    m_fieldCurrent = nullptr;

    std::vector<bool>           m_arrayBool;
    std::vector<std::int32_t>   m_arrayInt32;
    std::vector<std::uint32_t>  m_arrayUInt32;
    std::vector<std::int64_t>   m_arrayInt64;
    std::vector<std::uint64_t>  m_arrayUInt64;
    std::vector<float>          m_arrayFloat;
    std::vector<double>         m_arrayDouble;
    std::vector<std::string>    m_arrayString;
};
