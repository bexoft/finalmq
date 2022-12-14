
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace finalmq {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("desc")]
enum SerializeMetaTypeId
{
	[finalmq.MetaEnumEntry("desc", "none")]
    TYPE_NONE = 0,
	[finalmq.MetaEnumEntry("desc", "bool")]
    TYPE_BOOL = 1,
	[finalmq.MetaEnumEntry("desc", "int32")]
    TYPE_INT32 = 2,
	[finalmq.MetaEnumEntry("desc", "uint32")]
    TYPE_UINT32 = 3,
	[finalmq.MetaEnumEntry("desc", "int64")]
    TYPE_INT64 = 4,
	[finalmq.MetaEnumEntry("desc", "uint64")]
    TYPE_UINT64 = 5,
	[finalmq.MetaEnumEntry("desc", "float")]
    TYPE_FLOAT = 6,
	[finalmq.MetaEnumEntry("desc", "double")]
    TYPE_DOUBLE = 7,
	[finalmq.MetaEnumEntry("desc", "string")]
    TYPE_STRING = 8,
	[finalmq.MetaEnumEntry("desc", "bytes")]
    TYPE_BYTES = 9,
	[finalmq.MetaEnumEntry("desc", "struct")]
    TYPE_STRUCT = 10,
	[finalmq.MetaEnumEntry("desc", "enum")]
    TYPE_ENUM = 11,
	[finalmq.MetaEnumEntry("desc", "variant")]
    TYPE_VARIANT = 12,
	[finalmq.MetaEnumEntry("desc", "")]
    OFFSET_ARRAY_FLAG = 1024,
	[finalmq.MetaEnumEntry("desc", "bool[]")]
    TYPE_ARRAY_BOOL = 1025,
	[finalmq.MetaEnumEntry("desc", "int32[]")]
    TYPE_ARRAY_INT32 = 1026,
	[finalmq.MetaEnumEntry("desc", "uint32[]")]
    TYPE_ARRAY_UINT32 = 1027,
	[finalmq.MetaEnumEntry("desc", "int64[]")]
    TYPE_ARRAY_INT64 = 1028,
	[finalmq.MetaEnumEntry("desc", "uint64[]")]
    TYPE_ARRAY_UINT64 = 1029,
	[finalmq.MetaEnumEntry("desc", "float[]")]
    TYPE_ARRAY_FLOAT = 1030,
	[finalmq.MetaEnumEntry("desc", "double[]")]
    TYPE_ARRAY_DOUBLE = 1031,
	[finalmq.MetaEnumEntry("desc", "string[]")]
    TYPE_ARRAY_STRING = 1032,
	[finalmq.MetaEnumEntry("desc", "bytes[]")]
    TYPE_ARRAY_BYTES = 1033,
	[finalmq.MetaEnumEntry("desc", "struct[]")]
    TYPE_ARRAY_STRUCT = 1034,
	[finalmq.MetaEnumEntry("desc", "enum[]")]
    TYPE_ARRAY_ENUM = 1035,
}
class RegisterSerializeMetaTypeId
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaTypeId));
    }
}

[finalmq.MetaEnum("desc")]
enum SerializeMetaFieldFlags
{
	[finalmq.MetaEnumEntry("desc", "")]
    METAFLAG_NONE = 0,
	[finalmq.MetaEnumEntry("desc", "")]
    METAFLAG_PROTO_VARINT = 1,
	[finalmq.MetaEnumEntry("desc", "")]
    METAFLAG_PROTO_ZIGZAG = 2,
}
class RegisterSerializeMetaFieldFlags
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaFieldFlags));
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
class SerializeMetaEnumEntry
{
	[finalmq.MetaField("desc")]
    public string name
	{
		get { return m_name; }
		set { m_name = value; }
	}

	[finalmq.MetaField("desc")]
    public int id
	{
		get { return m_id; }
		set { m_id = value; }
	}

	[finalmq.MetaField("desc")]
    public string desc
	{
		get { return m_desc; }
		set { m_desc = value; }
	}

	[finalmq.MetaField("desc")]
    public string alias
	{
		get { return m_alias; }
		set { m_alias = value; }
	}

    string m_name = "";
    int m_id = 0;
    string m_desc = "";
    string m_alias = "";

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaEnumEntry), () => { return new SerializeMetaEnumEntry(); } );
    }
}

[finalmq.MetaStruct("desc")]
class SerializeMetaEnum
{
	[finalmq.MetaField("desc")]
    public string type
	{
		get { return m_type; }
		set { m_type = value; }
	}

	[finalmq.MetaField("desc")]
    public string desc
	{
		get { return m_desc; }
		set { m_desc = value; }
	}

	[finalmq.MetaField("desc")]
    public IList<finalmq.SerializeMetaEnumEntry> entries
	{
		get { return m_entries; }
		set { m_entries = value; }
	}

    string m_type = "";
    string m_desc = "";
    IList<finalmq.SerializeMetaEnumEntry> m_entries = new List<finalmq.SerializeMetaEnumEntry>();

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaEnum), () => { return new SerializeMetaEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
class SerializeMetaField
{
	[finalmq.MetaField("desc")]
    public finalmq.SerializeMetaTypeId tid
	{
		get { return m_tid; }
		set { m_tid = value; }
	}

	[finalmq.MetaField("desc")]
    public string type
	{
		get { return m_type; }
		set { m_type = value; }
	}

	[finalmq.MetaField("desc")]
    public string name
	{
		get { return m_name; }
		set { m_name = value; }
	}

	[finalmq.MetaField("desc")]
    public string desc
	{
		get { return m_desc; }
		set { m_desc = value; }
	}

	[finalmq.MetaField("desc")]
    public IList<finalmq.SerializeMetaFieldFlags> flags
	{
		get { return m_flags; }
		set { m_flags = value; }
	}

    finalmq.SerializeMetaTypeId m_tid = new finalmq.SerializeMetaTypeId();
    string m_type = "";
    string m_name = "";
    string m_desc = "";
    IList<finalmq.SerializeMetaFieldFlags> m_flags = new List<finalmq.SerializeMetaFieldFlags>();

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaField), () => { return new SerializeMetaField(); } );
    }
}

[finalmq.MetaStruct("desc")]
class SerializeMetaStruct
{
	[finalmq.MetaField("desc")]
    public string type
	{
		get { return m_type; }
		set { m_type = value; }
	}

	[finalmq.MetaField("desc")]
    public string desc
	{
		get { return m_desc; }
		set { m_desc = value; }
	}

	[finalmq.MetaField("desc")]
    public IList<finalmq.SerializeMetaField> fields
	{
		get { return m_fields; }
		set { m_fields = value; }
	}

    string m_type = "";
    string m_desc = "";
    IList<finalmq.SerializeMetaField> m_fields = new List<finalmq.SerializeMetaField>();

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaStruct), () => { return new SerializeMetaStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
class SerializeMetaData
{
	[finalmq.MetaField("desc")]
    public IList<finalmq.SerializeMetaEnum> enums
	{
		get { return m_enums; }
		set { m_enums = value; }
	}

	[finalmq.MetaField("desc")]
    public IList<finalmq.SerializeMetaStruct> structs
	{
		get { return m_structs; }
		set { m_structs = value; }
	}

    IList<finalmq.SerializeMetaEnum> m_enums = new List<finalmq.SerializeMetaEnum>();
    IList<finalmq.SerializeMetaStruct> m_structs = new List<finalmq.SerializeMetaStruct>();

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(SerializeMetaData), () => { return new SerializeMetaData(); } );
    }
}



} // close namespace finalmq     

#pragma warning restore IDE1006 // Benennungsstile

