
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace finalmq { 
namespace variant {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("desc")]
enum VarTypeId
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
        finalmq.TypeRegistry.Instance.RegisterType(typeof(VarTypeId));
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
class VarValue
{
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(VarValue), () => { return new VarValue(); } );
    }
}



} // close namespace variant 
} // close namespace finalmq     

#pragma warning restore IDE1006 // Benennungsstile

