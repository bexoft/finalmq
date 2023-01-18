
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace test {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("desc")]
public enum Foo
{
	[finalmq.MetaEnumEntry("desc", "")]
    FOO_WORLD = 0,
	[finalmq.MetaEnumEntry("desc", "")]
    FOO_HELLO = -2,
	[finalmq.MetaEnumEntry("desc", "world2")]
    FOO_WORLD2 = 1,
}
class RegisterFoo
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(Foo));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(Foo), metaEnum);
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
public class TestBool : finalmq.StructBase, IEquatable<TestBool>
{
    public TestBool()
	{
	}
	

    public TestBool(bool value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public bool value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    bool m_value = false;

	public bool Equals(TestBool? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestBool));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestBool));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestBool), m_metaStruct, () => { return new TestBool(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestInt32 : finalmq.StructBase, IEquatable<TestInt32>
{
    public TestInt32()
	{
	}
	

    public TestInt32(int value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public int value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    int m_value = 0;

	public bool Equals(TestInt32? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt32));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt32));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestInt32), m_metaStruct, () => { return new TestInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestInt32ZigZag : finalmq.StructBase, IEquatable<TestInt32ZigZag>
{
    public TestInt32ZigZag()
	{
	}
	

    public TestInt32ZigZag(int value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc", finalmq.MetaFieldFlags.METAFLAG_PROTO_ZIGZAG)]
    public int value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    int m_value = 0;

	public bool Equals(TestInt32ZigZag? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt32ZigZag));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt32ZigZag));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestInt32ZigZag), m_metaStruct, () => { return new TestInt32ZigZag(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestUInt32 : finalmq.StructBase, IEquatable<TestUInt32>
{
    public TestUInt32()
	{
	}
	

    public TestUInt32(uint value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public uint value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    uint m_value = 0;

	public bool Equals(TestUInt32? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestUInt32));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestUInt32));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestUInt32), m_metaStruct, () => { return new TestUInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestInt64 : finalmq.StructBase, IEquatable<TestInt64>
{
    public TestInt64()
	{
	}
	

    public TestInt64(long value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public long value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    long m_value = 0;

	public bool Equals(TestInt64? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt64));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestInt64));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestInt64), m_metaStruct, () => { return new TestInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestUInt64 : finalmq.StructBase, IEquatable<TestUInt64>
{
    public TestUInt64()
	{
	}
	

    public TestUInt64(ulong value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public ulong value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    ulong m_value = 0;

	public bool Equals(TestUInt64? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestUInt64));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestUInt64));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestUInt64), m_metaStruct, () => { return new TestUInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestFloat : finalmq.StructBase, IEquatable<TestFloat>
{
    public TestFloat()
	{
	}
	

    public TestFloat(float value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public float value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    float m_value = 0.0f;

	public bool Equals(TestFloat? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestFloat));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestFloat));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestFloat), m_metaStruct, () => { return new TestFloat(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestDouble : finalmq.StructBase, IEquatable<TestDouble>
{
    public TestDouble()
	{
	}
	

    public TestDouble(double value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public double value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    double m_value = 0.0;

	public bool Equals(TestDouble? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestDouble));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestDouble));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestDouble), m_metaStruct, () => { return new TestDouble(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestString : finalmq.StructBase, IEquatable<TestString>
{
    public TestString()
	{
	}
	

    public TestString(string value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public string value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    string m_value = "";

	public bool Equals(TestString? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestString));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestString));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestString), m_metaStruct, () => { return new TestString(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestBytes : finalmq.StructBase, IEquatable<TestBytes>
{
    public TestBytes()
	{
	}
	

    public TestBytes(byte[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public byte[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    byte[] m_value = new byte[0];

	public bool Equals(TestBytes? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestBytes));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestBytes));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestBytes), m_metaStruct, () => { return new TestBytes(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestStruct : finalmq.StructBase, IEquatable<TestStruct>
{
    public TestStruct()
	{
	}
	

    public TestStruct(test.TestInt32 struct_int32, test.TestString struct_string, uint last_value)
	{
		m_struct_int32 = struct_int32;
		m_struct_string = struct_string;
		m_last_value = last_value;
	}

	[finalmq.MetaField("desc")]
    public test.TestInt32 struct_int32
	{
		get { return m_struct_int32; }
		set { m_struct_int32 = value; }
	}
	[finalmq.MetaField("desc")]
    public test.TestString struct_string
	{
		get { return m_struct_string; }
		set { m_struct_string = value; }
	}
	[finalmq.MetaField("desc")]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    test.TestInt32 m_struct_int32 = new test.TestInt32();
    test.TestString m_struct_string = new test.TestString();
    uint m_last_value = 0;

	public bool Equals(TestStruct? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_struct_int32.Equals(rhs.m_struct_int32))
		{
			return false;
		}
		if (!m_struct_string.Equals(rhs.m_struct_string))
		{
			return false;
		}
		if (m_last_value != rhs.m_last_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestStruct));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestStruct));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestStruct), m_metaStruct, () => { return new TestStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestStructBlockSize : finalmq.StructBase, IEquatable<TestStructBlockSize>
{
    public TestStructBlockSize()
	{
	}
	

    public TestStructBlockSize(test.TestInt32 struct_int32, test.TestString struct_string, uint last_value)
	{
		m_struct_int32 = struct_int32;
		m_struct_string = struct_string;
		m_last_value = last_value;
	}

	[finalmq.MetaField("desc")]
    public test.TestInt32 struct_int32
	{
		get { return m_struct_int32; }
		set { m_struct_int32 = value; }
	}
	[finalmq.MetaField("desc")]
    public test.TestString struct_string
	{
		get { return m_struct_string; }
		set { m_struct_string = value; }
	}
	[finalmq.MetaField("desc", finalmq.MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    test.TestInt32 m_struct_int32 = new test.TestInt32();
    test.TestString m_struct_string = new test.TestString();
    uint m_last_value = 0;

	public bool Equals(TestStructBlockSize? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_struct_int32.Equals(rhs.m_struct_int32))
		{
			return false;
		}
		if (!m_struct_string.Equals(rhs.m_struct_string))
		{
			return false;
		}
		if (m_last_value != rhs.m_last_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestStructBlockSize));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestStructBlockSize));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestStructBlockSize), m_metaStruct, () => { return new TestStructBlockSize(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestEnum : finalmq.StructBase, IEquatable<TestEnum>
{
    public TestEnum()
	{
	}
	

    public TestEnum(test.Foo value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public test.Foo value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    test.Foo m_value = new test.Foo();

	public bool Equals(TestEnum? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_value != rhs.m_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestEnum));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestEnum));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestEnum), m_metaStruct, () => { return new TestEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestVariant : finalmq.StructBase, IEquatable<TestVariant>
{
    public TestVariant()
	{
	}
	

    public TestVariant(finalmq.Variant value, int valueInt32, finalmq.Variant value2)
	{
		m_value = value;
		m_valueInt32 = valueInt32;
		m_value2 = value2;
	}

	[finalmq.MetaField("desc")]
    public finalmq.Variant value
	{
		get { return m_value; }
		set { m_value = value; }
	}
	[finalmq.MetaField("desc")]
    public int valueInt32
	{
		get { return m_valueInt32; }
		set { m_valueInt32 = value; }
	}
	[finalmq.MetaField("desc")]
    public finalmq.Variant value2
	{
		get { return m_value2; }
		set { m_value2 = value; }
	}

    finalmq.Variant m_value = new finalmq.Variant();
    int m_valueInt32 = 0;
    finalmq.Variant m_value2 = new finalmq.Variant();

	public bool Equals(TestVariant? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.Equals(rhs.m_value))
		{
			return false;
		}
		if (m_valueInt32 != rhs.m_valueInt32)
		{
			return false;
		}
		if (!m_value2.Equals(rhs.m_value2))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestVariant));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestVariant));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestVariant), m_metaStruct, () => { return new TestVariant(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayBool : finalmq.StructBase, IEquatable<TestArrayBool>
{
    public TestArrayBool()
	{
	}
	

    public TestArrayBool(bool[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public bool[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    bool[] m_value = new bool[0];

	public bool Equals(TestArrayBool? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayBool));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayBool));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayBool), m_metaStruct, () => { return new TestArrayBool(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayInt32 : finalmq.StructBase, IEquatable<TestArrayInt32>
{
    public TestArrayInt32()
	{
	}
	

    public TestArrayInt32(int[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public int[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    int[] m_value = new int[0];

	public bool Equals(TestArrayInt32? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayInt32));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayInt32));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayInt32), m_metaStruct, () => { return new TestArrayInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayUInt32 : finalmq.StructBase, IEquatable<TestArrayUInt32>
{
    public TestArrayUInt32()
	{
	}
	

    public TestArrayUInt32(uint[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public uint[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    uint[] m_value = new uint[0];

	public bool Equals(TestArrayUInt32? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayUInt32));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayUInt32));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayUInt32), m_metaStruct, () => { return new TestArrayUInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayInt64 : finalmq.StructBase, IEquatable<TestArrayInt64>
{
    public TestArrayInt64()
	{
	}
	

    public TestArrayInt64(long[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public long[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    long[] m_value = new long[0];

	public bool Equals(TestArrayInt64? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayInt64));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayInt64));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayInt64), m_metaStruct, () => { return new TestArrayInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayUInt64 : finalmq.StructBase, IEquatable<TestArrayUInt64>
{
    public TestArrayUInt64()
	{
	}
	

    public TestArrayUInt64(ulong[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public ulong[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    ulong[] m_value = new ulong[0];

	public bool Equals(TestArrayUInt64? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayUInt64));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayUInt64));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayUInt64), m_metaStruct, () => { return new TestArrayUInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayFloat : finalmq.StructBase, IEquatable<TestArrayFloat>
{
    public TestArrayFloat()
	{
	}
	

    public TestArrayFloat(float[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public float[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    float[] m_value = new float[0];

	public bool Equals(TestArrayFloat? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayFloat));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayFloat));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayFloat), m_metaStruct, () => { return new TestArrayFloat(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayDouble : finalmq.StructBase, IEquatable<TestArrayDouble>
{
    public TestArrayDouble()
	{
	}
	

    public TestArrayDouble(double[] value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public double[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    double[] m_value = new double[0];

	public bool Equals(TestArrayDouble? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayDouble));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayDouble));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayDouble), m_metaStruct, () => { return new TestArrayDouble(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayString : finalmq.StructBase, IEquatable<TestArrayString>
{
    public TestArrayString()
	{
	}
	

    public TestArrayString(IList<string> value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public IList<string> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<string> m_value = new string[0];

	public bool Equals(TestArrayString? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayString));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayString));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayString), m_metaStruct, () => { return new TestArrayString(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayBytes : finalmq.StructBase, IEquatable<TestArrayBytes>
{
    public TestArrayBytes()
	{
	}
	

    public TestArrayBytes(IList<byte[]> value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public IList<byte[]> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<byte[]> m_value = new List<byte[]>();

	public bool Equals(TestArrayBytes? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

        if (m_value.Count != rhs.m_value.Count)
		{
			return false;
		}
		for (int i = 0; i < m_value.Count; i++)
		{
			byte[] arr1 = m_value[i];
			byte[] arr2 = rhs.m_value[i];
			if (!arr1.SequenceEqual(arr2))
			{
				return false;
			}
		}

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayBytes));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayBytes));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayBytes), m_metaStruct, () => { return new TestArrayBytes(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayStruct : finalmq.StructBase, IEquatable<TestArrayStruct>
{
    public TestArrayStruct()
	{
	}
	

    public TestArrayStruct(IList<test.TestStruct> value, uint last_value)
	{
		m_value = value;
		m_last_value = last_value;
	}

	[finalmq.MetaField("desc")]
    public IList<test.TestStruct> value
	{
		get { return m_value; }
		set { m_value = value; }
	}
	[finalmq.MetaField("desc")]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    IList<test.TestStruct> m_value = new List<test.TestStruct>();
    uint m_last_value = 0;

	public bool Equals(TestArrayStruct? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
		{
			return false;
		}
		if (m_last_value != rhs.m_last_value)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayStruct));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayStruct));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayStruct), m_metaStruct, () => { return new TestArrayStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayEnum : finalmq.StructBase, IEquatable<TestArrayEnum>
{
    public TestArrayEnum()
	{
	}
	

    public TestArrayEnum(IList<test.Foo> value)
	{
		m_value = value;
	}

	[finalmq.MetaField("desc")]
    public IList<test.Foo> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<test.Foo> m_value = new List<test.Foo>();

	public bool Equals(TestArrayEnum? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_value.SequenceEqual(rhs.m_value))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayEnum));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestArrayEnum));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestArrayEnum), m_metaStruct, () => { return new TestArrayEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestRequest : finalmq.StructBase, IEquatable<TestRequest>
{
    public TestRequest()
	{
	}
	

    public TestRequest(string datarequest)
	{
		m_datarequest = datarequest;
	}

	[finalmq.MetaField("desc")]
    public string datarequest
	{
		get { return m_datarequest; }
		set { m_datarequest = value; }
	}

    string m_datarequest = "";

	public bool Equals(TestRequest? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_datarequest != rhs.m_datarequest)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestRequest));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestRequest));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestRequest), m_metaStruct, () => { return new TestRequest(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestReply : finalmq.StructBase, IEquatable<TestReply>
{
    public TestReply()
	{
	}
	

    public TestReply(string datareply)
	{
		m_datareply = datareply;
	}

	[finalmq.MetaField("desc")]
    public string datareply
	{
		get { return m_datareply; }
		set { m_datareply = value; }
	}

    string m_datareply = "";

	public bool Equals(TestReply? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_datareply != rhs.m_datareply)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestReply));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TestReply));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TestReply), m_metaStruct, () => { return new TestReply(); } );
    }
}



} // close namespace test     

#pragma warning restore IDE1006 // Benennungsstile

