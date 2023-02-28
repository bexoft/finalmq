
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace testhl7 {     


//////////////////////////////
// Enums
//////////////////////////////





//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("Address")]
public class L31 : finalmq.StructBase, IEquatable<L31>
{
    public L31()
	{
	}
	

    public L31(string a, string b, string c, string d, string e)
	{
		m_a = a;
		m_b = b;
		m_c = c;
		m_d = d;
		m_e = e;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}
	[finalmq.MetaField("")]
    public string c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public string d
	{
		get { return m_d; }
		set { m_d = value; }
	}
	[finalmq.MetaField("")]
    public string e
	{
		get { return m_e; }
		set { m_e = value; }
	}

    string m_a = string.Empty;
    string m_b = string.Empty;
    string m_c = string.Empty;
    string m_d = string.Empty;
    string m_e = string.Empty;

	public bool Equals(L31? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
		{
			return false;
		}
		if (m_b != rhs.m_b)
		{
			return false;
		}
		if (m_c != rhs.m_c)
		{
			return false;
		}
		if (m_d != rhs.m_d)
		{
			return false;
		}
		if (m_e != rhs.m_e)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L31));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L31));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(L31), m_metaStruct, () => { return new L31(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class L21 : finalmq.StructBase, IEquatable<L21>
{
    public L21()
	{
	}
	

    public L21(string a, string b, string c, string d, string e)
	{
		m_a = a;
		m_b = b;
		m_c = c;
		m_d = d;
		m_e = e;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}
	[finalmq.MetaField("")]
    public string c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public string d
	{
		get { return m_d; }
		set { m_d = value; }
	}
	[finalmq.MetaField("")]
    public string e
	{
		get { return m_e; }
		set { m_e = value; }
	}

    string m_a = string.Empty;
    string m_b = string.Empty;
    string m_c = string.Empty;
    string m_d = string.Empty;
    string m_e = string.Empty;

	public bool Equals(L21? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
		{
			return false;
		}
		if (m_b != rhs.m_b)
		{
			return false;
		}
		if (m_c != rhs.m_c)
		{
			return false;
		}
		if (m_d != rhs.m_d)
		{
			return false;
		}
		if (m_e != rhs.m_e)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L21));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L21));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(L21), m_metaStruct, () => { return new L21(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class L11 : finalmq.StructBase, IEquatable<L11>
{
    public L11()
	{
	}
	

    public L11(string a, string b, string c, string d, string e)
	{
		m_a = a;
		m_b = b;
		m_c = c;
		m_d = d;
		m_e = e;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}
	[finalmq.MetaField("")]
    public string c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public string d
	{
		get { return m_d; }
		set { m_d = value; }
	}
	[finalmq.MetaField("")]
    public string e
	{
		get { return m_e; }
		set { m_e = value; }
	}

    string m_a = string.Empty;
    string m_b = string.Empty;
    string m_c = string.Empty;
    string m_d = string.Empty;
    string m_e = string.Empty;

	public bool Equals(L11? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
		{
			return false;
		}
		if (m_b != rhs.m_b)
		{
			return false;
		}
		if (m_c != rhs.m_c)
		{
			return false;
		}
		if (m_d != rhs.m_d)
		{
			return false;
		}
		if (m_e != rhs.m_e)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L11));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(L11));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(L11), m_metaStruct, () => { return new L11(); } );
    }
}

[finalmq.MetaStruct("Message Type")]
public class MSG : finalmq.StructBase, IEquatable<MSG>
{
    public MSG()
	{
	}
	

    public MSG(string messageCode, string triggerEvent, string messageStructure)
	{
		m_messageCode = messageCode;
		m_triggerEvent = triggerEvent;
		m_messageStructure = messageStructure;
	}

	[finalmq.MetaField("")]
    public string messageCode
	{
		get { return m_messageCode; }
		set { m_messageCode = value; }
	}
	[finalmq.MetaField("")]
    public string triggerEvent
	{
		get { return m_triggerEvent; }
		set { m_triggerEvent = value; }
	}
	[finalmq.MetaField("")]
    public string messageStructure
	{
		get { return m_messageStructure; }
		set { m_messageStructure = value; }
	}

    string m_messageCode = string.Empty;
    string m_triggerEvent = string.Empty;
    string m_messageStructure = string.Empty;

	public bool Equals(MSG? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_messageCode != rhs.m_messageCode)
		{
			return false;
		}
		if (m_triggerEvent != rhs.m_triggerEvent)
		{
			return false;
		}
		if (m_messageStructure != rhs.m_messageStructure)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG), m_metaStruct, () => { return new MSG(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class MSH : finalmq.StructBase, IEquatable<MSH>
{
    public MSH()
	{
	}
	

    public MSH(string fieldSeparator, string encodingCharacters, string sendingApplication, string sendingFacility, string receivingApplication, string receivingFacility, string date_TimeofMessage, string security, testhl7.MSG messageType)
	{
		m_fieldSeparator = fieldSeparator;
		m_encodingCharacters = encodingCharacters;
		m_sendingApplication = sendingApplication;
		m_sendingFacility = sendingFacility;
		m_receivingApplication = receivingApplication;
		m_receivingFacility = receivingFacility;
		m_date_TimeofMessage = date_TimeofMessage;
		m_security = security;
		m_messageType = messageType;
	}

	[finalmq.MetaField("")]
    public string fieldSeparator
	{
		get { return m_fieldSeparator; }
		set { m_fieldSeparator = value; }
	}
	[finalmq.MetaField("")]
    public string encodingCharacters
	{
		get { return m_encodingCharacters; }
		set { m_encodingCharacters = value; }
	}
	[finalmq.MetaField("")]
    public string sendingApplication
	{
		get { return m_sendingApplication; }
		set { m_sendingApplication = value; }
	}
	[finalmq.MetaField("")]
    public string sendingFacility
	{
		get { return m_sendingFacility; }
		set { m_sendingFacility = value; }
	}
	[finalmq.MetaField("")]
    public string receivingApplication
	{
		get { return m_receivingApplication; }
		set { m_receivingApplication = value; }
	}
	[finalmq.MetaField("")]
    public string receivingFacility
	{
		get { return m_receivingFacility; }
		set { m_receivingFacility = value; }
	}
	[finalmq.MetaField("")]
    public string date_TimeofMessage
	{
		get { return m_date_TimeofMessage; }
		set { m_date_TimeofMessage = value; }
	}
	[finalmq.MetaField("")]
    public string security
	{
		get { return m_security; }
		set { m_security = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.MSG messageType
	{
		get { return m_messageType; }
		set { m_messageType = value; }
	}

    string m_fieldSeparator = string.Empty;
    string m_encodingCharacters = string.Empty;
    string m_sendingApplication = string.Empty;
    string m_sendingFacility = string.Empty;
    string m_receivingApplication = string.Empty;
    string m_receivingFacility = string.Empty;
    string m_date_TimeofMessage = string.Empty;
    string m_security = string.Empty;
    testhl7.MSG m_messageType = new testhl7.MSG();

	public bool Equals(MSH? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_fieldSeparator != rhs.m_fieldSeparator)
		{
			return false;
		}
		if (m_encodingCharacters != rhs.m_encodingCharacters)
		{
			return false;
		}
		if (m_sendingApplication != rhs.m_sendingApplication)
		{
			return false;
		}
		if (m_sendingFacility != rhs.m_sendingFacility)
		{
			return false;
		}
		if (m_receivingApplication != rhs.m_receivingApplication)
		{
			return false;
		}
		if (m_receivingFacility != rhs.m_receivingFacility)
		{
			return false;
		}
		if (m_date_TimeofMessage != rhs.m_date_TimeofMessage)
		{
			return false;
		}
		if (m_security != rhs.m_security)
		{
			return false;
		}
		if (!m_messageType.Equals(rhs.m_messageType))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSH));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSH));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSH), m_metaStruct, () => { return new MSH(); } );
    }
}

[finalmq.MetaStruct("")]
public class FAA : finalmq.StructBase, IEquatable<FAA>
{
    public FAA()
	{
	}
	

    public FAA(string a, string b)
	{
		m_a = a;
		m_b = b;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    string m_a = string.Empty;
    string m_b = string.Empty;

	public bool Equals(FAA? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
		{
			return false;
		}
		if (m_b != rhs.m_b)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(FAA));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(FAA));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(FAA), m_metaStruct, () => { return new FAA(); } );
    }
}

[finalmq.MetaStruct("")]
public class FBB : finalmq.StructBase, IEquatable<FBB>
{
    public FBB()
	{
	}
	

    public FBB(string a, string b, testhl7.FAA faa)
	{
		m_a = a;
		m_b = b;
		m_faa = faa;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.FAA faa
	{
		get { return m_faa; }
		set { m_faa = value; }
	}

    string m_a = string.Empty;
    string m_b = string.Empty;
    testhl7.FAA m_faa = new testhl7.FAA();

	public bool Equals(FBB? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
		{
			return false;
		}
		if (m_b != rhs.m_b)
		{
			return false;
		}
		if (!m_faa.Equals(rhs.m_faa))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(FBB));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(FBB));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(FBB), m_metaStruct, () => { return new FBB(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class AAA : finalmq.StructBase, IEquatable<AAA>
{
    public AAA()
	{
	}
	

    public AAA(string a)
	{
		m_a = a;
	}

	[finalmq.MetaField("")]
    public string a
	{
		get { return m_a; }
		set { m_a = value; }
	}

    string m_a = string.Empty;

	public bool Equals(AAA? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_a != rhs.m_a)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(AAA));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(AAA));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(AAA), m_metaStruct, () => { return new AAA(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class BBB : finalmq.StructBase, IEquatable<BBB>
{
    public BBB()
	{
	}
	

    public BBB(string b)
	{
		m_b = b;
	}

	[finalmq.MetaField("")]
    public string b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    string m_b = string.Empty;

	public bool Equals(BBB? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_b != rhs.m_b)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(BBB));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(BBB));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(BBB), m_metaStruct, () => { return new BBB(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class CCC : finalmq.StructBase, IEquatable<CCC>
{
    public CCC()
	{
	}
	

    public CCC(string c)
	{
		m_c = c;
	}

	[finalmq.MetaField("")]
    public string c
	{
		get { return m_c; }
		set { m_c = value; }
	}

    string m_c = string.Empty;

	public bool Equals(CCC? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_c != rhs.m_c)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(CCC));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(CCC));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(CCC), m_metaStruct, () => { return new CCC(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class DDD : finalmq.StructBase, IEquatable<DDD>
{
    public DDD()
	{
	}
	

    public DDD(string d)
	{
		m_d = d;
	}

	[finalmq.MetaField("")]
    public string d
	{
		get { return m_d; }
		set { m_d = value; }
	}

    string m_d = string.Empty;

	public bool Equals(DDD? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_d != rhs.m_d)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(DDD));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(DDD));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(DDD), m_metaStruct, () => { return new DDD(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class EEE : finalmq.StructBase, IEquatable<EEE>
{
    public EEE()
	{
	}
	

    public EEE(testhl7.FAA e)
	{
		m_e = e;
	}

	[finalmq.MetaField("")]
    public testhl7.FAA e
	{
		get { return m_e; }
		set { m_e = value; }
	}

    testhl7.FAA m_e = new testhl7.FAA();

	public bool Equals(EEE? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_e.Equals(rhs.m_e))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EEE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EEE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(EEE), m_metaStruct, () => { return new EEE(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class A01 : finalmq.StructBase, IEquatable<A01>
{
    public A01()
	{
	}
	

    public A01(IList<string> a)
	{
		m_a = a;
	}

	[finalmq.MetaField("")]
    public IList<string> a
	{
		get { return m_a; }
		set { m_a = value; }
	}

    IList<string> m_a = new List<string>();

	public bool Equals(A01? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_a.SequenceEqual(rhs.m_a))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(A01));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(A01));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(A01), m_metaStruct, () => { return new A01(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class A02 : finalmq.StructBase, IEquatable<A02>
{
    public A02()
	{
	}
	

    public A02(IList<testhl7.FAA> faa, IList<testhl7.FBB> fbb, testhl7.FAA sfaa, testhl7.FBB sfbb)
	{
		m_faa = faa;
		m_fbb = fbb;
		m_sfaa = sfaa;
		m_sfbb = sfbb;
	}

	[finalmq.MetaField("")]
    public IList<testhl7.FAA> faa
	{
		get { return m_faa; }
		set { m_faa = value; }
	}
	[finalmq.MetaField("")]
    public IList<testhl7.FBB> fbb
	{
		get { return m_fbb; }
		set { m_fbb = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.FAA sfaa
	{
		get { return m_sfaa; }
		set { m_sfaa = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.FBB sfbb
	{
		get { return m_sfbb; }
		set { m_sfbb = value; }
	}

    IList<testhl7.FAA> m_faa = new List<testhl7.FAA>();
    IList<testhl7.FBB> m_fbb = new List<testhl7.FBB>();
    testhl7.FAA m_sfaa = new testhl7.FAA();
    testhl7.FBB m_sfbb = new testhl7.FBB();

	public bool Equals(A02? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_faa.SequenceEqual(rhs.m_faa))
		{
			return false;
		}
		if (!m_fbb.SequenceEqual(rhs.m_fbb))
		{
			return false;
		}
		if (!m_sfaa.Equals(rhs.m_sfaa))
		{
			return false;
		}
		if (!m_sfbb.Equals(rhs.m_sfbb))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(A02));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(A02));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(A02), m_metaStruct, () => { return new A02(); } );
    }
}

[finalmq.MetaStruct("")]
public class GROUP_1 : finalmq.StructBase, IEquatable<GROUP_1>
{
    public GROUP_1()
	{
	}
	

    public GROUP_1(testhl7.AAA a, testhl7.CCC? c)
	{
		m_a = a;
		m_c = c;
	}

	[finalmq.MetaField("")]
    public testhl7.AAA a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.CCC? c
	{
		get { return m_c; }
		set { m_c = value; }
	}

    testhl7.AAA m_a = new testhl7.AAA();
    testhl7.CCC? m_c = null;

	public bool Equals(GROUP_1? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_a.Equals(rhs.m_a))
		{
			return false;
		}
		if (!((c == rhs.c) || ((c != null) && (c != null) && c.Equals(rhs.c))))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(GROUP_1));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(GROUP_1));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(GROUP_1), m_metaStruct, () => { return new GROUP_1(); } );
    }
}

[finalmq.MetaStruct("", finalmq.MetaStructFlags.METASTRUCTFLAG_CHOICE)]
public class GROUP_2 : finalmq.StructBase, IEquatable<GROUP_2>
{
    public GROUP_2()
	{
	}
	

    public GROUP_2(testhl7.AAA? a, testhl7.BBB? b, testhl7.CCC? c)
	{
		m_a = a;
		m_b = b;
		m_c = c;
	}

	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.AAA? a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.BBB? b
	{
		get { return m_b; }
		set { m_b = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.CCC? c
	{
		get { return m_c; }
		set { m_c = value; }
	}

    testhl7.AAA? m_a = null;
    testhl7.BBB? m_b = null;
    testhl7.CCC? m_c = null;

	public bool Equals(GROUP_2? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!((a == rhs.a) || ((a != null) && (a != null) && a.Equals(rhs.a))))
		{
			return false;
		}
		if (!((b == rhs.b) || ((b != null) && (b != null) && b.Equals(rhs.b))))
		{
			return false;
		}
		if (!((c == rhs.c) || ((c != null) && (c != null) && c.Equals(rhs.c))))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(GROUP_2));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(GROUP_2));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(GROUP_2), m_metaStruct, () => { return new GROUP_2(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_001 : finalmq.StructBase, IEquatable<MSG_001>
{
    public MSG_001()
	{
	}
	

    public MSG_001(testhl7.MSH msh, testhl7.AAA a)
	{
		m_msh = msh;
		m_a = a;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.AAA a
	{
		get { return m_a; }
		set { m_a = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.AAA m_a = new testhl7.AAA();

	public bool Equals(MSG_001? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_a.Equals(rhs.m_a))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_001));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_001));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_001), m_metaStruct, () => { return new MSG_001(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_002 : finalmq.StructBase, IEquatable<MSG_002>
{
    public MSG_002()
	{
	}
	

    public MSG_002(testhl7.MSH msh, testhl7.AAA? a, testhl7.BBB b)
	{
		m_msh = msh;
		m_a = a;
		m_b = b;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.AAA? a
	{
		get { return m_a; }
		set { m_a = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.BBB b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.AAA? m_a = null;
    testhl7.BBB m_b = new testhl7.BBB();

	public bool Equals(MSG_002? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!((a == rhs.a) || ((a != null) && (a != null) && a.Equals(rhs.a))))
		{
			return false;
		}
		if (!m_b.Equals(rhs.m_b))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_002));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_002));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_002), m_metaStruct, () => { return new MSG_002(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_003 : finalmq.StructBase, IEquatable<MSG_003>
{
    public MSG_003()
	{
	}
	

    public MSG_003(testhl7.MSH msh, testhl7.GROUP_1 c, testhl7.BBB b)
	{
		m_msh = msh;
		m_c = c;
		m_b = b;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.GROUP_1 c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.BBB b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.GROUP_1 m_c = new testhl7.GROUP_1();
    testhl7.BBB m_b = new testhl7.BBB();

	public bool Equals(MSG_003? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_c.Equals(rhs.m_c))
		{
			return false;
		}
		if (!m_b.Equals(rhs.m_b))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_003));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_003));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_003), m_metaStruct, () => { return new MSG_003(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_004 : finalmq.StructBase, IEquatable<MSG_004>
{
    public MSG_004()
	{
	}
	

    public MSG_004(testhl7.MSH msh, testhl7.GROUP_1? c, testhl7.BBB b)
	{
		m_msh = msh;
		m_c = c;
		m_b = b;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.GROUP_1? c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.BBB b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.GROUP_1? m_c = null;
    testhl7.BBB m_b = new testhl7.BBB();

	public bool Equals(MSG_004? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!((c == rhs.c) || ((c != null) && (c != null) && c.Equals(rhs.c))))
		{
			return false;
		}
		if (!m_b.Equals(rhs.m_b))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_004));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_004));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_004), m_metaStruct, () => { return new MSG_004(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_005 : finalmq.StructBase, IEquatable<MSG_005>
{
    public MSG_005()
	{
	}
	

    public MSG_005(testhl7.MSH msh, testhl7.GROUP_2? c, testhl7.BBB b)
	{
		m_msh = msh;
		m_c = c;
		m_b = b;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public testhl7.GROUP_2? c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.BBB b
	{
		get { return m_b; }
		set { m_b = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.GROUP_2? m_c = null;
    testhl7.BBB m_b = new testhl7.BBB();

	public bool Equals(MSG_005? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!((c == rhs.c) || ((c != null) && (c != null) && c.Equals(rhs.c))))
		{
			return false;
		}
		if (!m_b.Equals(rhs.m_b))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_005));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_005));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_005), m_metaStruct, () => { return new MSG_005(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_006 : finalmq.StructBase, IEquatable<MSG_006>
{
    public MSG_006()
	{
	}
	

    public MSG_006(testhl7.MSH msh, IList<testhl7.GROUP_2> c, testhl7.DDD d)
	{
		m_msh = msh;
		m_c = c;
		m_d = d;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public IList<testhl7.GROUP_2> c
	{
		get { return m_c; }
		set { m_c = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.DDD d
	{
		get { return m_d; }
		set { m_d = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    IList<testhl7.GROUP_2> m_c = new List<testhl7.GROUP_2>();
    testhl7.DDD m_d = new testhl7.DDD();

	public bool Equals(MSG_006? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_c.SequenceEqual(rhs.m_c))
		{
			return false;
		}
		if (!m_d.Equals(rhs.m_d))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_006));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_006));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_006), m_metaStruct, () => { return new MSG_006(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_007 : finalmq.StructBase, IEquatable<MSG_007>
{
    public MSG_007()
	{
	}
	

    public MSG_007(testhl7.MSH msh, testhl7.EEE e)
	{
		m_msh = msh;
		m_e = e;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.EEE e
	{
		get { return m_e; }
		set { m_e = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.EEE m_e = new testhl7.EEE();

	public bool Equals(MSG_007? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_e.Equals(rhs.m_e))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_007));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_007));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_007), m_metaStruct, () => { return new MSG_007(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_010 : finalmq.StructBase, IEquatable<MSG_010>
{
    public MSG_010()
	{
	}
	

    public MSG_010(testhl7.MSH msh, testhl7.A01 a01)
	{
		m_msh = msh;
		m_a01 = a01;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.A01 a01
	{
		get { return m_a01; }
		set { m_a01 = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.A01 m_a01 = new testhl7.A01();

	public bool Equals(MSG_010? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_a01.Equals(rhs.m_a01))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_010));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_010));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_010), m_metaStruct, () => { return new MSG_010(); } );
    }
}

[finalmq.MetaStruct("Address")]
public class MSG_011 : finalmq.StructBase, IEquatable<MSG_011>
{
    public MSG_011()
	{
	}
	

    public MSG_011(testhl7.MSH msh, testhl7.A02 a02)
	{
		m_msh = msh;
		m_a02 = a02;
	}

	[finalmq.MetaField("")]
    public testhl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public testhl7.A02 a02
	{
		get { return m_a02; }
		set { m_a02 = value; }
	}

    testhl7.MSH m_msh = new testhl7.MSH();
    testhl7.A02 m_a02 = new testhl7.A02();

	public bool Equals(MSG_011? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_msh.Equals(rhs.m_msh))
		{
			return false;
		}
		if (!m_a02.Equals(rhs.m_a02))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_011));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_011));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_011), m_metaStruct, () => { return new MSG_011(); } );
    }
}



} // close namespace testhl7     

#pragma warning restore IDE1006 // Benennungsstile

