
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
        finalmq.TypeRegistry.Instance.RegisterType(typeof(Foo));
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
public class TestBool
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestBool), () => { return new TestBool(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestInt32
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestInt32), () => { return new TestInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestUInt32
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestUInt32), () => { return new TestUInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestInt64
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestInt64), () => { return new TestInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestUInt64
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestUInt64), () => { return new TestUInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestFloat
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestFloat), () => { return new TestFloat(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestDouble
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestDouble), () => { return new TestDouble(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestString
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestString), () => { return new TestString(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestBytes
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestBytes), () => { return new TestBytes(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestStruct
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestStruct), () => { return new TestStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestStructBlockSize
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestStructBlockSize), () => { return new TestStructBlockSize(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestEnum
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestEnum), () => { return new TestEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestVariant
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestVariant), () => { return new TestVariant(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayBool
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayBool), () => { return new TestArrayBool(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayInt32
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayInt32), () => { return new TestArrayInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayUInt32
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayUInt32), () => { return new TestArrayUInt32(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayInt64
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayInt64), () => { return new TestArrayInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayUInt64
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayUInt64), () => { return new TestArrayUInt64(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayFloat
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayFloat), () => { return new TestArrayFloat(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayDouble
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayDouble), () => { return new TestArrayDouble(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayString
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayString), () => { return new TestArrayString(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayBytes
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayBytes), () => { return new TestArrayBytes(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayStruct
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayStruct), () => { return new TestArrayStruct(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestArrayEnum
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestArrayEnum), () => { return new TestArrayEnum(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestRequest
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestRequest), () => { return new TestRequest(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class TestReply
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

#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterStruct()
    {
        finalmq.TypeRegistry.Instance.RegisterType(typeof(TestReply), () => { return new TestReply(); } );
    }
}



} // close namespace test     

#pragma warning restore IDE1006 // Benennungsstile

