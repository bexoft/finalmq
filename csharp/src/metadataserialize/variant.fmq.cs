
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace finalmq { 
namespace variant {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("desc")]
public enum VarTypeId
{
	[finalmq.MetaEnumEntry("desc", "none")]
    T_NONE = 0,
	[finalmq.MetaEnumEntry("desc", "bool")]
    T_BOOL = 1,
	[finalmq.MetaEnumEntry("desc", "int32")]
    T_INT32 = 2,
	[finalmq.MetaEnumEntry("desc", "uint32")]
    T_UINT32 = 3,
	[finalmq.MetaEnumEntry("desc", "int64")]
    T_INT64 = 4,
	[finalmq.MetaEnumEntry("desc", "uint64")]
    T_UINT64 = 5,
	[finalmq.MetaEnumEntry("desc", "float")]
    T_FLOAT = 6,
	[finalmq.MetaEnumEntry("desc", "double")]
    T_DOUBLE = 7,
	[finalmq.MetaEnumEntry("desc", "string")]
    T_STRING = 8,
	[finalmq.MetaEnumEntry("desc", "bytes")]
    T_BYTES = 9,
	[finalmq.MetaEnumEntry("desc", "struct")]
    T_STRUCT = 10,
	[finalmq.MetaEnumEntry("desc", "bool[]")]
    T_ARRAY_BOOL = 1025,
	[finalmq.MetaEnumEntry("desc", "int32[]")]
    T_ARRAY_INT32 = 1026,
	[finalmq.MetaEnumEntry("desc", "uint32[]")]
    T_ARRAY_UINT32 = 1027,
	[finalmq.MetaEnumEntry("desc", "int64[]")]
    T_ARRAY_INT64 = 1028,
	[finalmq.MetaEnumEntry("desc", "uint64[]")]
    T_ARRAY_UINT64 = 1029,
	[finalmq.MetaEnumEntry("desc", "float[]")]
    T_ARRAY_FLOAT = 1030,
	[finalmq.MetaEnumEntry("desc", "double[]")]
    T_ARRAY_DOUBLE = 1031,
	[finalmq.MetaEnumEntry("desc", "string[]")]
    T_ARRAY_STRING = 1032,
	[finalmq.MetaEnumEntry("desc", "bytes[]")]
    T_ARRAY_BYTES = 1033,
	[finalmq.MetaEnumEntry("desc", "list")]
    T_LIST = 1034,
}
class RegisterVarTypeId
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(VarTypeId));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(VarTypeId), metaEnum);
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
public class VarValue : finalmq.StructBase, IEquatable<VarValue>
{
    public VarValue()
	{
	}
	

    public VarValue(string name, finalmq.variant.VarTypeId type, bool valbool, int valint32, uint valuint32, long valint64, ulong valuint64, float valfloat, double valdouble, string valstring, byte[] valbytes, IList<finalmq.variant.VarValue> vallist, bool[] valarrbool, int[] valarrint32, uint[] valarruint32, long[] valarrint64, ulong[] valarruint64, float[] valarrfloat, double[] valarrdouble, IList<string> valarrstring, IList<byte[]> valarrbytes)
	{
		m_name = name;
		m_type = type;
		m_valbool = valbool;
		m_valint32 = valint32;
		m_valuint32 = valuint32;
		m_valint64 = valint64;
		m_valuint64 = valuint64;
		m_valfloat = valfloat;
		m_valdouble = valdouble;
		m_valstring = valstring;
		m_valbytes = valbytes;
		m_vallist = vallist;
		m_valarrbool = valarrbool;
		m_valarrint32 = valarrint32;
		m_valarruint32 = valarruint32;
		m_valarrint64 = valarrint64;
		m_valarruint64 = valarruint64;
		m_valarrfloat = valarrfloat;
		m_valarrdouble = valarrdouble;
		m_valarrstring = valarrstring;
		m_valarrbytes = valarrbytes;
	}

	[finalmq.MetaField("name is only used for elements in valstruct")]
    public string name
	{
		get { return m_name; }
		set { m_name = value; }
	}
	[finalmq.MetaField("")]
    public finalmq.variant.VarTypeId type
	{
		get { return m_type; }
		set { m_type = value; }
	}
	[finalmq.MetaField("")]
    public bool valbool
	{
		get { return m_valbool; }
		set { m_valbool = value; }
	}
	[finalmq.MetaField("")]
    public int valint32
	{
		get { return m_valint32; }
		set { m_valint32 = value; }
	}
	[finalmq.MetaField("")]
    public uint valuint32
	{
		get { return m_valuint32; }
		set { m_valuint32 = value; }
	}
	[finalmq.MetaField("")]
    public long valint64
	{
		get { return m_valint64; }
		set { m_valint64 = value; }
	}
	[finalmq.MetaField("")]
    public ulong valuint64
	{
		get { return m_valuint64; }
		set { m_valuint64 = value; }
	}
	[finalmq.MetaField("")]
    public float valfloat
	{
		get { return m_valfloat; }
		set { m_valfloat = value; }
	}
	[finalmq.MetaField("")]
    public double valdouble
	{
		get { return m_valdouble; }
		set { m_valdouble = value; }
	}
	[finalmq.MetaField("")]
    public string valstring
	{
		get { return m_valstring; }
		set { m_valstring = value; }
	}
	[finalmq.MetaField("")]
    public byte[] valbytes
	{
		get { return m_valbytes; }
		set { m_valbytes = value; }
	}
	[finalmq.MetaField("")]
    public IList<finalmq.variant.VarValue> vallist
	{
		get { return m_vallist; }
		set { m_vallist = value; }
	}
	[finalmq.MetaField("")]
    public bool[] valarrbool
	{
		get { return m_valarrbool; }
		set { m_valarrbool = value; }
	}
	[finalmq.MetaField("")]
    public int[] valarrint32
	{
		get { return m_valarrint32; }
		set { m_valarrint32 = value; }
	}
	[finalmq.MetaField("")]
    public uint[] valarruint32
	{
		get { return m_valarruint32; }
		set { m_valarruint32 = value; }
	}
	[finalmq.MetaField("")]
    public long[] valarrint64
	{
		get { return m_valarrint64; }
		set { m_valarrint64 = value; }
	}
	[finalmq.MetaField("")]
    public ulong[] valarruint64
	{
		get { return m_valarruint64; }
		set { m_valarruint64 = value; }
	}
	[finalmq.MetaField("")]
    public float[] valarrfloat
	{
		get { return m_valarrfloat; }
		set { m_valarrfloat = value; }
	}
	[finalmq.MetaField("")]
    public double[] valarrdouble
	{
		get { return m_valarrdouble; }
		set { m_valarrdouble = value; }
	}
	[finalmq.MetaField("")]
    public IList<string> valarrstring
	{
		get { return m_valarrstring; }
		set { m_valarrstring = value; }
	}
	[finalmq.MetaField("")]
    public IList<byte[]> valarrbytes
	{
		get { return m_valarrbytes; }
		set { m_valarrbytes = value; }
	}

    string m_name = "";
    finalmq.variant.VarTypeId m_type = new finalmq.variant.VarTypeId();
    bool m_valbool = false;
    int m_valint32 = 0;
    uint m_valuint32 = 0;
    long m_valint64 = 0;
    ulong m_valuint64 = 0;
    float m_valfloat = 0.0f;
    double m_valdouble = 0.0;
    string m_valstring = "";
    byte[] m_valbytes = new byte[0];
    IList<finalmq.variant.VarValue> m_vallist = new List<finalmq.variant.VarValue>();
    bool[] m_valarrbool = new bool[0];
    int[] m_valarrint32 = new int[0];
    uint[] m_valarruint32 = new uint[0];
    long[] m_valarrint64 = new long[0];
    ulong[] m_valarruint64 = new ulong[0];
    float[] m_valarrfloat = new float[0];
    double[] m_valarrdouble = new double[0];
    IList<string> m_valarrstring = new string[0];
    IList<byte[]> m_valarrbytes = new List<byte[]>();

	public bool Equals(VarValue? rhs)
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
    		if (m_type != rhs.m_type)
		{
			return false;
		}
    		if (m_valbool != rhs.m_valbool)
		{
			return false;
		}
    		if (m_valint32 != rhs.m_valint32)
		{
			return false;
		}
    		if (m_valuint32 != rhs.m_valuint32)
		{
			return false;
		}
    		if (m_valint64 != rhs.m_valint64)
		{
			return false;
		}
    		if (m_valuint64 != rhs.m_valuint64)
		{
			return false;
		}
    		if (m_valfloat != rhs.m_valfloat)
		{
			return false;
		}
    		if (m_valdouble != rhs.m_valdouble)
		{
			return false;
		}
    		if (m_valstring != rhs.m_valstring)
		{
			return false;
		}
     		if (!m_valbytes.Equals(rhs.m_valbytes))
		{
			return false;
		}
     		if (!m_vallist.SequenceEqual(rhs.m_vallist))
		{
			return false;
		}
   		if (!m_valarrbool.Equals(rhs.m_valarrbool))
		{
			return false;
		}
    		if (!m_valarrint32.Equals(rhs.m_valarrint32))
		{
			return false;
		}
    		if (!m_valarruint32.Equals(rhs.m_valarruint32))
		{
			return false;
		}
    		if (!m_valarrint64.Equals(rhs.m_valarrint64))
		{
			return false;
		}
    		if (!m_valarruint64.Equals(rhs.m_valarruint64))
		{
			return false;
		}
    		if (!m_valarrfloat.Equals(rhs.m_valarrfloat))
		{
			return false;
		}
    		if (!m_valarrdouble.Equals(rhs.m_valarrdouble))
		{
			return false;
		}
     		if (!m_valarrstring.SequenceEqual(rhs.m_valarrstring))
		{
			return false;
		}
    		if (!m_valarrbytes.SequenceEqual(rhs.m_valarrbytes))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(VarValue));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(VarValue));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(VarValue), m_metaStruct, () => { return new VarValue(); } );
    }
}



} // close namespace variant 
} // close namespace finalmq     

#pragma warning restore IDE1006 // Benennungsstile

