#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"


#include <string>


enum WireType
{
    WIRETYPE_VARINT = 0,
    WIRETYPE_FIXED64 = 1,
    WIRETYPE_LENGTH_DELIMITED = 2,
    WIRETYPE_START_GROUP = 3,
    WIRETYPE_END_GROUP = 4,
    WIRETYPE_FIXED32 = 5,
};




class ParserProto
{
public:
    ParserProto(IParserVisitor& visitor, const char* ptr, int size);

    bool parseStruct(const std::string& typeName);

private:
    bool parseString(const char*& buffer, int& size);
    void parseStructWire(const MetaField& field);
    bool parseStructIntern(const MetaStruct& stru);
//    bool parseArrayBytes(std::vector<Bytes>& array);
    void parseArrayStruct(const MetaField& field);

    template<class T>
    bool parseArrayString(std::vector<T>& array);

    template<class T, int WIRETYPE>
    bool parseFixedValue(T& value);

    template<class T>
    bool parseVarint(T& value);

    template<class T, class D>
    bool parseZigZag(T& value);

    template<class T, int WIRETYPE>
    bool parseArrayFixed(std::vector<T>& array);

    template<class T, bool ZIGZAG = false>
    bool parseArrayVarint(std::vector<T>& array);

    std::uint64_t parseVarint();

    template<class T>
    T parseFixed();

    inline std::int32_t zigzag(std::uint32_t value);
    inline std::int64_t zigzag(std::uint64_t value);

    void skip(WireType wireType);

    const char*         m_ptr = nullptr;
    int                 m_size = 0;
    IParserVisitor&     m_visitor;

    std::uint32_t       m_tag = 0;
};
