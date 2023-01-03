
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace finalmq {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("desc")]
public enum SerializeMetaTypeId
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
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(SerializeMetaTypeId));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(SerializeMetaTypeId), metaEnum);
    }
}

[finalmq.MetaEnum("desc")]
public enum SerializeMetaFieldFlags
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
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(SerializeMetaFieldFlags));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(SerializeMetaFieldFlags), metaEnum);
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
public class SerializeMetaEnumEntry : finalmq.StructBase, IEquatable<SerializeMetaEnumEntry>
{
    public SerializeMetaEnumEntry()
	{
	}
	

    public SerializeMetaEnumEntry(string name, int id, string desc, string alias)
	{
		m_name = name;
		m_id = id;
		m_desc = desc;
		m_alias = alias;
	}

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

	public bool Equals(SerializeMetaEnumEntry? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_name != rhs.m_name)
		{
			return false;
		}
		if (m_id != rhs.m_id)
		{
			return false;
		}
		if (m_desc != rhs.m_desc)
		{
			return false;
		}
		if (m_alias != rhs.m_alias)
		{
			return false;
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaEnumEntry));
			}
			return m_metaStruct;
		}
	}
	static finalmq.MetaStruct? m_metaStruct = null;

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaEnumEntry));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SerializeMetaEnumEntry), m_metaStruct, () => { return new SerializeMetaEnumEntry(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SerializeMetaEnum : finalmq.StructBase, IEquatable<SerializeMetaEnum>
{
    public SerializeMetaEnum()
	{
	}
	

    public SerializeMetaEnum(string type, string desc, IList<finalmq.SerializeMetaEnumEntry> entries)
	{
		m_type = type;
		m_desc = desc;
		m_entries = entries;
	}

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

	public bool Equals(SerializeMetaEnum? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_type != rhs.m_type)
		{
			return false;
		}
		if (m_desc != rhs.m_desc)
		{
			return false;
		}
		if (!m_entries.SequenceEqual(rhs.m_entries))
		{
			return false;
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaEnum));
			}
			return m_metaStruct;
		}
	}
	static finalmq.MetaStruct? m_metaStruct = null;

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaEnum));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SerializeMetaEnum), m_metaStruct, () => { return new SerializeMetaEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SerializeMetaField : finalmq.StructBase, IEquatable<SerializeMetaField>
{
    public SerializeMetaField()
	{
	}
	

    public SerializeMetaField(finalmq.SerializeMetaTypeId tid, string type, string name, string desc, IList<finalmq.SerializeMetaFieldFlags> flags)
	{
		m_tid = tid;
		m_type = type;
		m_name = name;
		m_desc = desc;
		m_flags = flags;
	}

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

	public bool Equals(SerializeMetaField? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_tid != rhs.m_tid)
		{
			return false;
		}
		if (m_type != rhs.m_type)
		{
			return false;
		}
		if (m_name != rhs.m_name)
		{
			return false;
		}
		if (m_desc != rhs.m_desc)
		{
			return false;
		}
		if (!m_flags.SequenceEqual(rhs.m_flags))
		{
			return false;
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaField));
			}
			return m_metaStruct;
		}
	}
	static finalmq.MetaStruct? m_metaStruct = null;

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaField));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SerializeMetaField), m_metaStruct, () => { return new SerializeMetaField(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SerializeMetaStruct : finalmq.StructBase, IEquatable<SerializeMetaStruct>
{
    public SerializeMetaStruct()
	{
	}
	

    public SerializeMetaStruct(string type, string desc, IList<finalmq.SerializeMetaField> fields)
	{
		m_type = type;
		m_desc = desc;
		m_fields = fields;
	}

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

	public bool Equals(SerializeMetaStruct? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_type != rhs.m_type)
		{
			return false;
		}
		if (m_desc != rhs.m_desc)
		{
			return false;
		}
		if (!m_fields.SequenceEqual(rhs.m_fields))
		{
			return false;
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaStruct));
			}
			return m_metaStruct;
		}
	}
	static finalmq.MetaStruct? m_metaStruct = null;

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaStruct));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SerializeMetaStruct), m_metaStruct, () => { return new SerializeMetaStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SerializeMetaData : finalmq.StructBase, IEquatable<SerializeMetaData>
{
    public SerializeMetaData()
	{
	}
	

    public SerializeMetaData(IList<finalmq.SerializeMetaEnum> enums, IList<finalmq.SerializeMetaStruct> structs)
	{
		m_enums = enums;
		m_structs = structs;
	}

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

	public bool Equals(SerializeMetaData? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_enums.SequenceEqual(rhs.m_enums))
		{
			return false;
		}
		if (!m_structs.SequenceEqual(rhs.m_structs))
		{
			return false;
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaData));
			}
			return m_metaStruct;
		}
	}
	static finalmq.MetaStruct? m_metaStruct = null;

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SerializeMetaData));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SerializeMetaData), m_metaStruct, () => { return new SerializeMetaData(); } );
    }
}



} // close namespace finalmq     

#pragma warning restore IDE1006 // Benennungsstile

