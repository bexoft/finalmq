
#include "serializestruct/StructBase.h"



class SerializeMetaTypeId
{
public:
    typedef MetaTypeId  Enum;

    SerializeMetaTypeId();
    SerializeMetaTypeId(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const SerializeMetaTypeId& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = TYPE_NONE;
    static const EnumInfo _enumInfo;
};


class SerializeMetaFieldFlags
{
public:
    typedef MetaFieldFlags  Enum;

    SerializeMetaFieldFlags();
    SerializeMetaFieldFlags(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const SerializeMetaFieldFlags& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = METAFLAG_NONE;
    static const EnumInfo _enumInfo;
};



struct SerializeMetaEnumEntry : public StructBase
{
    std::string     name;
    int             id = 0;
    std::string     description;

    SerializeMetaEnumEntry();
    SerializeMetaEnumEntry(const decltype(name)& name_, const decltype(id)& id_, const decltype(description)& description_);
    bool operator ==(const SerializeMetaEnumEntry& rhs) const;
    bool operator !=(const SerializeMetaEnumEntry& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};


struct SerializeMetaEnum : public StructBase
{
    std::string     typeName;
    std::string     description;
    std::vector<SerializeMetaEnumEntry> entries;

    SerializeMetaEnum();
    SerializeMetaEnum(const decltype(typeName)& typeName_, const decltype(description)& description_, const decltype(entries)& entries_);
    bool operator ==(const SerializeMetaEnum& rhs) const;
    bool operator !=(const SerializeMetaEnum& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};


struct SerializeMetaField : public StructBase
{
    SerializeMetaTypeId typeId;                                 ///< type id of the parameter
    std::string         typeName;                               ///< is needed for struct and enum
    std::string         name;                                   ///< parameter name
    std::string         description;                            ///< description of the parameter
    std::vector<SerializeMetaFieldFlags> flags;                 ///< flaggs of the parameter

    SerializeMetaField();
    SerializeMetaField(const decltype(typeId)& typeId_, const decltype(typeName)& typeName_, const decltype(name)& name_, const decltype(description)& description_, const decltype(flags)& flags_);
    bool operator ==(const SerializeMetaField& rhs) const;
    bool operator !=(const SerializeMetaField& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};


struct SerializeMetaStruct : public StructBase
{
    std::string     typeName;
    std::string     description;
    std::vector<SerializeMetaField> fields;

    SerializeMetaStruct();
    SerializeMetaStruct(const decltype(typeName)& typeName_, const decltype(description)& description_, const decltype(fields)& fields_);
    bool operator ==(const SerializeMetaStruct& rhs) const;
    bool operator !=(const SerializeMetaStruct& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};




struct SerializeMetaData : public StructBase
{
    std::vector<SerializeMetaEnum>      enums;
    std::vector<SerializeMetaStruct>    structs;

    SerializeMetaData();
    SerializeMetaData(const decltype(enums)& enums_, const decltype(structs)& structs_);
    bool operator ==(const SerializeMetaData& rhs) const;
    bool operator !=(const SerializeMetaData& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
