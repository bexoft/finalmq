
#pragma warning disable IDE1006 // Benennungsstile


namespace test {     


//////////////////////////////
// Enums
//////////////////////////////


[MetaEnum("desc")]
enum Foo
{
	[MetaEnumEntry("desc", "")]
    FOO_WORLD = 0,
	[MetaEnumEntry("desc", "")]
    FOO_HELLO = -2,
	[MetaEnumEntry("desc", "world2")]
    FOO_WORLD2 = 1,
}




//////////////////////////////
// Structs
//////////////////////////////



[MetaStruct("desc")]
class TestBool
{
	[MetaField("desc")]
    public bool value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    bool m_value = false;
}

[MetaStruct("desc")]
class TestInt32
{
	[MetaField("desc")]
    public int value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    int m_value = 0;
}

[MetaStruct("desc")]
class TestUInt32
{
	[MetaField("desc")]
    public uint value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    uint m_value = 0;
}

[MetaStruct("desc")]
class TestInt64
{
	[MetaField("desc")]
    public long value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    long m_value = 0;
}

[MetaStruct("desc")]
class TestUInt64
{
	[MetaField("desc")]
    public ulong value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    ulong m_value = 0;
}

[MetaStruct("desc")]
class TestFloat
{
	[MetaField("desc")]
    public float value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    float m_value = 0.0;
}

[MetaStruct("desc")]
class TestDouble
{
	[MetaField("desc")]
    public double value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    double m_value = 0.0;
}

[MetaStruct("desc")]
class TestString
{
	[MetaField("desc")]
    public string value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    string m_value = "";
}

[MetaStruct("desc")]
class TestBytes
{
	[MetaField("desc")]
    public byte[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    byte[] m_value = new bytes[0];
}

[MetaStruct("desc")]
class TestStruct
{
	[MetaField("desc")]
    public test.TestInt32 struct_int32
	{
		get { return m_struct_int32; }
		set { m_struct_int32 = value; }
	}

	[MetaField("desc")]
    public test.TestString struct_string
	{
		get { return m_struct_string; }
		set { m_struct_string = value; }
	}

	[MetaField("desc")]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    test.TestInt32 m_struct_int32 = new test.TestInt32();
    test.TestString m_struct_string = new test.TestString();
    uint m_last_value = 0;
}

[MetaStruct("desc")]
class TestStructBlockSize
{
	[MetaField("desc")]
    public test.TestInt32 struct_int32
	{
		get { return m_struct_int32; }
		set { m_struct_int32 = value; }
	}

	[MetaField("desc")]
    public test.TestString struct_string
	{
		get { return m_struct_string; }
		set { m_struct_string = value; }
	}

	[MetaField("desc", MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    test.TestInt32 m_struct_int32 = new test.TestInt32();
    test.TestString m_struct_string = new test.TestString();
    uint m_last_value = 0;
}

[MetaStruct("desc")]
class TestEnum
{
	[MetaField("desc")]
    public test.Foo value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    test.Foo m_value = new test.Foo();
}

[MetaStruct("desc")]
class TestVariant
{
	[MetaField("desc")]
    public finalmq.Variant value
	{
		get { return m_value; }
		set { m_value = value; }
	}

	[MetaField("desc")]
    public int valueInt32
	{
		get { return m_valueInt32; }
		set { m_valueInt32 = value; }
	}

	[MetaField("desc")]
    public finalmq.Variant value2
	{
		get { return m_value2; }
		set { m_value2 = value; }
	}

    finalmq.Variant m_value = new fianlmq.Variant();
    int m_valueInt32 = 0;
    finalmq.Variant m_value2 = new fianlmq.Variant();
}

[MetaStruct("desc")]
class TestArrayBool
{
	[MetaField("desc")]
    public bool[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    bool[] m_value = new bool[0];
}

[MetaStruct("desc")]
class TestArrayInt32
{
	[MetaField("desc")]
    public int[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    int[] m_value = new int[0];
}

[MetaStruct("desc")]
class TestArrayUInt32
{
	[MetaField("desc")]
    public uint[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    uint[] m_value = new uint[0];
}

[MetaStruct("desc")]
class TestArrayInt64
{
	[MetaField("desc")]
    public long[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    long[] m_value = new long[0];
}

[MetaStruct("desc")]
class TestArrayUInt64
{
	[MetaField("desc")]
    public ulong[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    ulong[] m_value = new ulong[0];
}

[MetaStruct("desc")]
class TestArrayFloat
{
	[MetaField("desc")]
    public float[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    float[] m_value = new float[0];
}

[MetaStruct("desc")]
class TestArrayDouble
{
	[MetaField("desc")]
    public double[] value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    double[] m_value = new double[0];
}

[MetaStruct("desc")]
class TestArrayString
{
	[MetaField("desc")]
    public IList<string> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<string> m_value = new string[0];
}

[MetaStruct("desc")]
class TestArrayBytes
{
	[MetaField("desc")]
    public IList<byte[]> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<byte[]> m_value = new IList<byte[]>();
}

[MetaStruct("desc")]
class TestArrayStruct
{
	[MetaField("desc")]
    public IList<test.TestStruct> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

	[MetaField("desc")]
    public uint last_value
	{
		get { return m_last_value; }
		set { m_last_value = value; }
	}

    IList<test.TestStruct> m_value = new IList<test.TestStruct>();
    uint m_last_value = 0;
}

[MetaStruct("desc")]
class TestArrayEnum
{
	[MetaField("desc")]
    public IList<test.Foo> value
	{
		get { return m_value; }
		set { m_value = value; }
	}

    IList<test.Foo> m_value = new IList<test.Foo>();
}

[MetaStruct("desc")]
class TestRequest
{
	[MetaField("desc")]
    public string datarequest
	{
		get { return m_datarequest; }
		set { m_datarequest = value; }
	}

    string m_datarequest = "";
}

[MetaStruct("desc")]
class TestReply
{
	[MetaField("desc")]
    public string datareply
	{
		get { return m_datareply; }
		set { m_datareply = value; }
	}

    string m_datareply = "";
}



} // close namespace test     

#pragma warning restore IDE1006 // Benennungsstile

