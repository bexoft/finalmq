
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace hl7 {     


//////////////////////////////
// Enums
//////////////////////////////





//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("desc")]
public class HD : finalmq.StructBase, IEquatable<HD>
{
    public HD()
	{
	}
	

    public HD(string namespaceId, string universalId, string universalIdType)
	{
		m_namespaceId = namespaceId;
		m_universalId = universalId;
		m_universalIdType = universalIdType;
	}

	[finalmq.MetaField("")]
    public string namespaceId
	{
		get { return m_namespaceId; }
		set { m_namespaceId = value; }
	}
	[finalmq.MetaField("")]
    public string universalId
	{
		get { return m_universalId; }
		set { m_universalId = value; }
	}
	[finalmq.MetaField("")]
    public string universalIdType
	{
		get { return m_universalIdType; }
		set { m_universalIdType = value; }
	}

    string m_namespaceId = string.Empty;
    string m_universalId = string.Empty;
    string m_universalIdType = string.Empty;

	public bool Equals(HD? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_namespaceId != rhs.m_namespaceId)
		{
			return false;
		}
		if (m_universalId != rhs.m_universalId)
		{
			return false;
		}
		if (m_universalIdType != rhs.m_universalIdType)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HD));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HD));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(HD), m_metaStruct, () => { return new HD(); } );
    }
}

[finalmq.MetaStruct("desc")]
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

[finalmq.MetaStruct("desc")]
public class PT : finalmq.StructBase, IEquatable<PT>
{
    public PT()
	{
	}
	

    public PT(string processingId, string processingMode)
	{
		m_processingId = processingId;
		m_processingMode = processingMode;
	}

	[finalmq.MetaField("")]
    public string processingId
	{
		get { return m_processingId; }
		set { m_processingId = value; }
	}
	[finalmq.MetaField("")]
    public string processingMode
	{
		get { return m_processingMode; }
		set { m_processingMode = value; }
	}

    string m_processingId = string.Empty;
    string m_processingMode = string.Empty;

	public bool Equals(PT? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_processingId != rhs.m_processingId)
		{
			return false;
		}
		if (m_processingMode != rhs.m_processingMode)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(PT));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(PT));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(PT), m_metaStruct, () => { return new PT(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class CWE : finalmq.StructBase, IEquatable<CWE>
{
    public CWE()
	{
	}
	

    public CWE(string identifier, string text, string nameOfCodingSystem, string alternateIdentifier, string alternateText, string nameOfAlternateCodingSystem)
	{
		m_identifier = identifier;
		m_text = text;
		m_nameOfCodingSystem = nameOfCodingSystem;
		m_alternateIdentifier = alternateIdentifier;
		m_alternateText = alternateText;
		m_nameOfAlternateCodingSystem = nameOfAlternateCodingSystem;
	}

	[finalmq.MetaField("")]
    public string identifier
	{
		get { return m_identifier; }
		set { m_identifier = value; }
	}
	[finalmq.MetaField("")]
    public string text
	{
		get { return m_text; }
		set { m_text = value; }
	}
	[finalmq.MetaField("")]
    public string nameOfCodingSystem
	{
		get { return m_nameOfCodingSystem; }
		set { m_nameOfCodingSystem = value; }
	}
	[finalmq.MetaField("")]
    public string alternateIdentifier
	{
		get { return m_alternateIdentifier; }
		set { m_alternateIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string alternateText
	{
		get { return m_alternateText; }
		set { m_alternateText = value; }
	}
	[finalmq.MetaField("")]
    public string nameOfAlternateCodingSystem
	{
		get { return m_nameOfAlternateCodingSystem; }
		set { m_nameOfAlternateCodingSystem = value; }
	}

    string m_identifier = string.Empty;
    string m_text = string.Empty;
    string m_nameOfCodingSystem = string.Empty;
    string m_alternateIdentifier = string.Empty;
    string m_alternateText = string.Empty;
    string m_nameOfAlternateCodingSystem = string.Empty;

	public bool Equals(CWE? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_identifier != rhs.m_identifier)
		{
			return false;
		}
		if (m_text != rhs.m_text)
		{
			return false;
		}
		if (m_nameOfCodingSystem != rhs.m_nameOfCodingSystem)
		{
			return false;
		}
		if (m_alternateIdentifier != rhs.m_alternateIdentifier)
		{
			return false;
		}
		if (m_alternateText != rhs.m_alternateText)
		{
			return false;
		}
		if (m_nameOfAlternateCodingSystem != rhs.m_nameOfAlternateCodingSystem)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(CWE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(CWE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(CWE), m_metaStruct, () => { return new CWE(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class VID : finalmq.StructBase, IEquatable<VID>
{
    public VID()
	{
	}
	

    public VID(string versionId, hl7.CWE internationalizationCode, hl7.CWE internationalizationVersionId)
	{
		m_versionId = versionId;
		m_internationalizationCode = internationalizationCode;
		m_internationalizationVersionId = internationalizationVersionId;
	}

	[finalmq.MetaField("")]
    public string versionId
	{
		get { return m_versionId; }
		set { m_versionId = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE internationalizationCode
	{
		get { return m_internationalizationCode; }
		set { m_internationalizationCode = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE internationalizationVersionId
	{
		get { return m_internationalizationVersionId; }
		set { m_internationalizationVersionId = value; }
	}

    string m_versionId = string.Empty;
    hl7.CWE m_internationalizationCode = new hl7.CWE();
    hl7.CWE m_internationalizationVersionId = new hl7.CWE();

	public bool Equals(VID? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_versionId != rhs.m_versionId)
		{
			return false;
		}
		if (!m_internationalizationCode.Equals(rhs.m_internationalizationCode))
		{
			return false;
		}
		if (!m_internationalizationVersionId.Equals(rhs.m_internationalizationVersionId))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(VID));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(VID));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(VID), m_metaStruct, () => { return new VID(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class XON : finalmq.StructBase, IEquatable<XON>
{
    public XON()
	{
	}
	

    public XON(string organizationName, hl7.CWE organizationNameTypeCode, string idNumber, string identifierCheckDigit, string checkDigitScheme, hl7.HD assigningAuthority, string identifierTypeCode, hl7.HD assigningFacility, string nameRepresentationCode, string organizationIdentifier)
	{
		m_organizationName = organizationName;
		m_organizationNameTypeCode = organizationNameTypeCode;
		m_idNumber = idNumber;
		m_identifierCheckDigit = identifierCheckDigit;
		m_checkDigitScheme = checkDigitScheme;
		m_assigningAuthority = assigningAuthority;
		m_identifierTypeCode = identifierTypeCode;
		m_assigningFacility = assigningFacility;
		m_nameRepresentationCode = nameRepresentationCode;
		m_organizationIdentifier = organizationIdentifier;
	}

	[finalmq.MetaField("")]
    public string organizationName
	{
		get { return m_organizationName; }
		set { m_organizationName = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE organizationNameTypeCode
	{
		get { return m_organizationNameTypeCode; }
		set { m_organizationNameTypeCode = value; }
	}
	[finalmq.MetaField("")]
    public string idNumber
	{
		get { return m_idNumber; }
		set { m_idNumber = value; }
	}
	[finalmq.MetaField("")]
    public string identifierCheckDigit
	{
		get { return m_identifierCheckDigit; }
		set { m_identifierCheckDigit = value; }
	}
	[finalmq.MetaField("")]
    public string checkDigitScheme
	{
		get { return m_checkDigitScheme; }
		set { m_checkDigitScheme = value; }
	}
	[finalmq.MetaField("")]
    public hl7.HD assigningAuthority
	{
		get { return m_assigningAuthority; }
		set { m_assigningAuthority = value; }
	}
	[finalmq.MetaField("")]
    public string identifierTypeCode
	{
		get { return m_identifierTypeCode; }
		set { m_identifierTypeCode = value; }
	}
	[finalmq.MetaField("")]
    public hl7.HD assigningFacility
	{
		get { return m_assigningFacility; }
		set { m_assigningFacility = value; }
	}
	[finalmq.MetaField("")]
    public string nameRepresentationCode
	{
		get { return m_nameRepresentationCode; }
		set { m_nameRepresentationCode = value; }
	}
	[finalmq.MetaField("")]
    public string organizationIdentifier
	{
		get { return m_organizationIdentifier; }
		set { m_organizationIdentifier = value; }
	}

    string m_organizationName = string.Empty;
    hl7.CWE m_organizationNameTypeCode = new hl7.CWE();
    string m_idNumber = string.Empty;
    string m_identifierCheckDigit = string.Empty;
    string m_checkDigitScheme = string.Empty;
    hl7.HD m_assigningAuthority = new hl7.HD();
    string m_identifierTypeCode = string.Empty;
    hl7.HD m_assigningFacility = new hl7.HD();
    string m_nameRepresentationCode = string.Empty;
    string m_organizationIdentifier = string.Empty;

	public bool Equals(XON? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_organizationName != rhs.m_organizationName)
		{
			return false;
		}
		if (!m_organizationNameTypeCode.Equals(rhs.m_organizationNameTypeCode))
		{
			return false;
		}
		if (m_idNumber != rhs.m_idNumber)
		{
			return false;
		}
		if (m_identifierCheckDigit != rhs.m_identifierCheckDigit)
		{
			return false;
		}
		if (m_checkDigitScheme != rhs.m_checkDigitScheme)
		{
			return false;
		}
		if (!m_assigningAuthority.Equals(rhs.m_assigningAuthority))
		{
			return false;
		}
		if (m_identifierTypeCode != rhs.m_identifierTypeCode)
		{
			return false;
		}
		if (!m_assigningFacility.Equals(rhs.m_assigningFacility))
		{
			return false;
		}
		if (m_nameRepresentationCode != rhs.m_nameRepresentationCode)
		{
			return false;
		}
		if (m_organizationIdentifier != rhs.m_organizationIdentifier)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(XON));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(XON));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(XON), m_metaStruct, () => { return new XON(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class ED : finalmq.StructBase, IEquatable<ED>
{
    public ED()
	{
	}
	

    public ED(hl7.HD sourceApplication, string typeOfData, string dataSubtype, string encoding, string data)
	{
		m_sourceApplication = sourceApplication;
		m_typeOfData = typeOfData;
		m_dataSubtype = dataSubtype;
		m_encoding = encoding;
		m_data = data;
	}

	[finalmq.MetaField("")]
    public hl7.HD sourceApplication
	{
		get { return m_sourceApplication; }
		set { m_sourceApplication = value; }
	}
	[finalmq.MetaField("")]
    public string typeOfData
	{
		get { return m_typeOfData; }
		set { m_typeOfData = value; }
	}
	[finalmq.MetaField("")]
    public string dataSubtype
	{
		get { return m_dataSubtype; }
		set { m_dataSubtype = value; }
	}
	[finalmq.MetaField("")]
    public string encoding
	{
		get { return m_encoding; }
		set { m_encoding = value; }
	}
	[finalmq.MetaField("")]
    public string data
	{
		get { return m_data; }
		set { m_data = value; }
	}

    hl7.HD m_sourceApplication = new hl7.HD();
    string m_typeOfData = string.Empty;
    string m_dataSubtype = string.Empty;
    string m_encoding = string.Empty;
    string m_data = string.Empty;

	public bool Equals(ED? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_sourceApplication.Equals(rhs.m_sourceApplication))
		{
			return false;
		}
		if (m_typeOfData != rhs.m_typeOfData)
		{
			return false;
		}
		if (m_dataSubtype != rhs.m_dataSubtype)
		{
			return false;
		}
		if (m_encoding != rhs.m_encoding)
		{
			return false;
		}
		if (m_data != rhs.m_data)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ED));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ED));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ED), m_metaStruct, () => { return new ED(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class ERL : finalmq.StructBase, IEquatable<ERL>
{
    public ERL()
	{
	}
	

    public ERL(string segmentId, string segmentSequence, string fieldPosition, string fieldRepetition, string componentNumber, string subcomponentNumber)
	{
		m_segmentId = segmentId;
		m_segmentSequence = segmentSequence;
		m_fieldPosition = fieldPosition;
		m_fieldRepetition = fieldRepetition;
		m_componentNumber = componentNumber;
		m_subcomponentNumber = subcomponentNumber;
	}

	[finalmq.MetaField("")]
    public string segmentId
	{
		get { return m_segmentId; }
		set { m_segmentId = value; }
	}
	[finalmq.MetaField("")]
    public string segmentSequence
	{
		get { return m_segmentSequence; }
		set { m_segmentSequence = value; }
	}
	[finalmq.MetaField("")]
    public string fieldPosition
	{
		get { return m_fieldPosition; }
		set { m_fieldPosition = value; }
	}
	[finalmq.MetaField("")]
    public string fieldRepetition
	{
		get { return m_fieldRepetition; }
		set { m_fieldRepetition = value; }
	}
	[finalmq.MetaField("")]
    public string componentNumber
	{
		get { return m_componentNumber; }
		set { m_componentNumber = value; }
	}
	[finalmq.MetaField("")]
    public string subcomponentNumber
	{
		get { return m_subcomponentNumber; }
		set { m_subcomponentNumber = value; }
	}

    string m_segmentId = string.Empty;
    string m_segmentSequence = string.Empty;
    string m_fieldPosition = string.Empty;
    string m_fieldRepetition = string.Empty;
    string m_componentNumber = string.Empty;
    string m_subcomponentNumber = string.Empty;

	public bool Equals(ERL? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_segmentId != rhs.m_segmentId)
		{
			return false;
		}
		if (m_segmentSequence != rhs.m_segmentSequence)
		{
			return false;
		}
		if (m_fieldPosition != rhs.m_fieldPosition)
		{
			return false;
		}
		if (m_fieldRepetition != rhs.m_fieldRepetition)
		{
			return false;
		}
		if (m_componentNumber != rhs.m_componentNumber)
		{
			return false;
		}
		if (m_subcomponentNumber != rhs.m_subcomponentNumber)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ERL));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ERL));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ERL), m_metaStruct, () => { return new ERL(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class XCN : finalmq.StructBase, IEquatable<XCN>
{
    public XCN()
	{
	}
	

    public XCN(string personIdentifier, string familyName, string givenName, string secondOrFurtherGivenNames)
	{
		m_personIdentifier = personIdentifier;
		m_familyName = familyName;
		m_givenName = givenName;
		m_secondOrFurtherGivenNames = secondOrFurtherGivenNames;
	}

	[finalmq.MetaField("")]
    public string personIdentifier
	{
		get { return m_personIdentifier; }
		set { m_personIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string familyName
	{
		get { return m_familyName; }
		set { m_familyName = value; }
	}
	[finalmq.MetaField("")]
    public string givenName
	{
		get { return m_givenName; }
		set { m_givenName = value; }
	}
	[finalmq.MetaField("")]
    public string secondOrFurtherGivenNames
	{
		get { return m_secondOrFurtherGivenNames; }
		set { m_secondOrFurtherGivenNames = value; }
	}

    string m_personIdentifier = string.Empty;
    string m_familyName = string.Empty;
    string m_givenName = string.Empty;
    string m_secondOrFurtherGivenNames = string.Empty;

	public bool Equals(XCN? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_personIdentifier != rhs.m_personIdentifier)
		{
			return false;
		}
		if (m_familyName != rhs.m_familyName)
		{
			return false;
		}
		if (m_givenName != rhs.m_givenName)
		{
			return false;
		}
		if (m_secondOrFurtherGivenNames != rhs.m_secondOrFurtherGivenNames)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(XCN));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(XCN));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(XCN), m_metaStruct, () => { return new XCN(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class EI : finalmq.StructBase, IEquatable<EI>
{
    public EI()
	{
	}
	

    public EI(string entityIdentifier, string namespaceId, string universalId, string universalIdType)
	{
		m_entityIdentifier = entityIdentifier;
		m_namespaceId = namespaceId;
		m_universalId = universalId;
		m_universalIdType = universalIdType;
	}

	[finalmq.MetaField("")]
    public string entityIdentifier
	{
		get { return m_entityIdentifier; }
		set { m_entityIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string namespaceId
	{
		get { return m_namespaceId; }
		set { m_namespaceId = value; }
	}
	[finalmq.MetaField("")]
    public string universalId
	{
		get { return m_universalId; }
		set { m_universalId = value; }
	}
	[finalmq.MetaField("")]
    public string universalIdType
	{
		get { return m_universalIdType; }
		set { m_universalIdType = value; }
	}

    string m_entityIdentifier = string.Empty;
    string m_namespaceId = string.Empty;
    string m_universalId = string.Empty;
    string m_universalIdType = string.Empty;

	public bool Equals(EI? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_entityIdentifier != rhs.m_entityIdentifier)
		{
			return false;
		}
		if (m_namespaceId != rhs.m_namespaceId)
		{
			return false;
		}
		if (m_universalId != rhs.m_universalId)
		{
			return false;
		}
		if (m_universalIdType != rhs.m_universalIdType)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EI));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EI));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(EI), m_metaStruct, () => { return new EI(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class MSH : finalmq.StructBase, IEquatable<MSH>
{
    public MSH()
	{
	}
	

    public MSH(string fieldSeparator, string encodingCharacters, hl7.HD sendingApplication, hl7.HD sendingFacility, hl7.HD receivingApplication, hl7.HD receivingFacility, string dateTimeOfMessage, string security, hl7.MSG messageType, string messageControlId, hl7.PT processingId, hl7.VID versionId, string sequenceNumber, string continuationPointer, string acceptAcknowledgmentType, string applicationAcknowledgmentType, string countryCode, string characterSet, string principalLanguageOfMessage, string alternateCharacterSetHandlingScheme, string messageProfileIdentifier, string sendingResponsibleOrganization, string receivingResponsibleOrganization, string sendingNetworkAddress, string receivingNetworkAddress)
	{
		m_fieldSeparator = fieldSeparator;
		m_encodingCharacters = encodingCharacters;
		m_sendingApplication = sendingApplication;
		m_sendingFacility = sendingFacility;
		m_receivingApplication = receivingApplication;
		m_receivingFacility = receivingFacility;
		m_dateTimeOfMessage = dateTimeOfMessage;
		m_security = security;
		m_messageType = messageType;
		m_messageControlId = messageControlId;
		m_processingId = processingId;
		m_versionId = versionId;
		m_sequenceNumber = sequenceNumber;
		m_continuationPointer = continuationPointer;
		m_acceptAcknowledgmentType = acceptAcknowledgmentType;
		m_applicationAcknowledgmentType = applicationAcknowledgmentType;
		m_countryCode = countryCode;
		m_characterSet = characterSet;
		m_principalLanguageOfMessage = principalLanguageOfMessage;
		m_alternateCharacterSetHandlingScheme = alternateCharacterSetHandlingScheme;
		m_messageProfileIdentifier = messageProfileIdentifier;
		m_sendingResponsibleOrganization = sendingResponsibleOrganization;
		m_receivingResponsibleOrganization = receivingResponsibleOrganization;
		m_sendingNetworkAddress = sendingNetworkAddress;
		m_receivingNetworkAddress = receivingNetworkAddress;
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
    public hl7.HD sendingApplication
	{
		get { return m_sendingApplication; }
		set { m_sendingApplication = value; }
	}
	[finalmq.MetaField("")]
    public hl7.HD sendingFacility
	{
		get { return m_sendingFacility; }
		set { m_sendingFacility = value; }
	}
	[finalmq.MetaField("")]
    public hl7.HD receivingApplication
	{
		get { return m_receivingApplication; }
		set { m_receivingApplication = value; }
	}
	[finalmq.MetaField("")]
    public hl7.HD receivingFacility
	{
		get { return m_receivingFacility; }
		set { m_receivingFacility = value; }
	}
	[finalmq.MetaField("")]
    public string dateTimeOfMessage
	{
		get { return m_dateTimeOfMessage; }
		set { m_dateTimeOfMessage = value; }
	}
	[finalmq.MetaField("")]
    public string security
	{
		get { return m_security; }
		set { m_security = value; }
	}
	[finalmq.MetaField("")]
    public hl7.MSG messageType
	{
		get { return m_messageType; }
		set { m_messageType = value; }
	}
	[finalmq.MetaField("")]
    public string messageControlId
	{
		get { return m_messageControlId; }
		set { m_messageControlId = value; }
	}
	[finalmq.MetaField("")]
    public hl7.PT processingId
	{
		get { return m_processingId; }
		set { m_processingId = value; }
	}
	[finalmq.MetaField("")]
    public hl7.VID versionId
	{
		get { return m_versionId; }
		set { m_versionId = value; }
	}
	[finalmq.MetaField("")]
    public string sequenceNumber
	{
		get { return m_sequenceNumber; }
		set { m_sequenceNumber = value; }
	}
	[finalmq.MetaField("")]
    public string continuationPointer
	{
		get { return m_continuationPointer; }
		set { m_continuationPointer = value; }
	}
	[finalmq.MetaField("")]
    public string acceptAcknowledgmentType
	{
		get { return m_acceptAcknowledgmentType; }
		set { m_acceptAcknowledgmentType = value; }
	}
	[finalmq.MetaField("")]
    public string applicationAcknowledgmentType
	{
		get { return m_applicationAcknowledgmentType; }
		set { m_applicationAcknowledgmentType = value; }
	}
	[finalmq.MetaField("")]
    public string countryCode
	{
		get { return m_countryCode; }
		set { m_countryCode = value; }
	}
	[finalmq.MetaField("")]
    public string characterSet
	{
		get { return m_characterSet; }
		set { m_characterSet = value; }
	}
	[finalmq.MetaField("")]
    public string principalLanguageOfMessage
	{
		get { return m_principalLanguageOfMessage; }
		set { m_principalLanguageOfMessage = value; }
	}
	[finalmq.MetaField("")]
    public string alternateCharacterSetHandlingScheme
	{
		get { return m_alternateCharacterSetHandlingScheme; }
		set { m_alternateCharacterSetHandlingScheme = value; }
	}
	[finalmq.MetaField("")]
    public string messageProfileIdentifier
	{
		get { return m_messageProfileIdentifier; }
		set { m_messageProfileIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string sendingResponsibleOrganization
	{
		get { return m_sendingResponsibleOrganization; }
		set { m_sendingResponsibleOrganization = value; }
	}
	[finalmq.MetaField("")]
    public string receivingResponsibleOrganization
	{
		get { return m_receivingResponsibleOrganization; }
		set { m_receivingResponsibleOrganization = value; }
	}
	[finalmq.MetaField("")]
    public string sendingNetworkAddress
	{
		get { return m_sendingNetworkAddress; }
		set { m_sendingNetworkAddress = value; }
	}
	[finalmq.MetaField("")]
    public string receivingNetworkAddress
	{
		get { return m_receivingNetworkAddress; }
		set { m_receivingNetworkAddress = value; }
	}

    string m_fieldSeparator = string.Empty;
    string m_encodingCharacters = string.Empty;
    hl7.HD m_sendingApplication = new hl7.HD();
    hl7.HD m_sendingFacility = new hl7.HD();
    hl7.HD m_receivingApplication = new hl7.HD();
    hl7.HD m_receivingFacility = new hl7.HD();
    string m_dateTimeOfMessage = string.Empty;
    string m_security = string.Empty;
    hl7.MSG m_messageType = new hl7.MSG();
    string m_messageControlId = string.Empty;
    hl7.PT m_processingId = new hl7.PT();
    hl7.VID m_versionId = new hl7.VID();
    string m_sequenceNumber = string.Empty;
    string m_continuationPointer = string.Empty;
    string m_acceptAcknowledgmentType = string.Empty;
    string m_applicationAcknowledgmentType = string.Empty;
    string m_countryCode = string.Empty;
    string m_characterSet = string.Empty;
    string m_principalLanguageOfMessage = string.Empty;
    string m_alternateCharacterSetHandlingScheme = string.Empty;
    string m_messageProfileIdentifier = string.Empty;
    string m_sendingResponsibleOrganization = string.Empty;
    string m_receivingResponsibleOrganization = string.Empty;
    string m_sendingNetworkAddress = string.Empty;
    string m_receivingNetworkAddress = string.Empty;

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
		if (!m_sendingApplication.Equals(rhs.m_sendingApplication))
		{
			return false;
		}
		if (!m_sendingFacility.Equals(rhs.m_sendingFacility))
		{
			return false;
		}
		if (!m_receivingApplication.Equals(rhs.m_receivingApplication))
		{
			return false;
		}
		if (!m_receivingFacility.Equals(rhs.m_receivingFacility))
		{
			return false;
		}
		if (m_dateTimeOfMessage != rhs.m_dateTimeOfMessage)
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
		if (m_messageControlId != rhs.m_messageControlId)
		{
			return false;
		}
		if (!m_processingId.Equals(rhs.m_processingId))
		{
			return false;
		}
		if (!m_versionId.Equals(rhs.m_versionId))
		{
			return false;
		}
		if (m_sequenceNumber != rhs.m_sequenceNumber)
		{
			return false;
		}
		if (m_continuationPointer != rhs.m_continuationPointer)
		{
			return false;
		}
		if (m_acceptAcknowledgmentType != rhs.m_acceptAcknowledgmentType)
		{
			return false;
		}
		if (m_applicationAcknowledgmentType != rhs.m_applicationAcknowledgmentType)
		{
			return false;
		}
		if (m_countryCode != rhs.m_countryCode)
		{
			return false;
		}
		if (m_characterSet != rhs.m_characterSet)
		{
			return false;
		}
		if (m_principalLanguageOfMessage != rhs.m_principalLanguageOfMessage)
		{
			return false;
		}
		if (m_alternateCharacterSetHandlingScheme != rhs.m_alternateCharacterSetHandlingScheme)
		{
			return false;
		}
		if (m_messageProfileIdentifier != rhs.m_messageProfileIdentifier)
		{
			return false;
		}
		if (m_sendingResponsibleOrganization != rhs.m_sendingResponsibleOrganization)
		{
			return false;
		}
		if (m_receivingResponsibleOrganization != rhs.m_receivingResponsibleOrganization)
		{
			return false;
		}
		if (m_sendingNetworkAddress != rhs.m_sendingNetworkAddress)
		{
			return false;
		}
		if (m_receivingNetworkAddress != rhs.m_receivingNetworkAddress)
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

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class SFT : finalmq.StructBase, IEquatable<SFT>
{
    public SFT()
	{
	}
	

    public SFT(hl7.XON softwareVendorOrganization, string softwareCertifiedVersionOrReleaseNumber, string softwareBinaryID, string softwareProductInformation, string softwareInstallDate)
	{
		m_softwareVendorOrganization = softwareVendorOrganization;
		m_softwareCertifiedVersionOrReleaseNumber = softwareCertifiedVersionOrReleaseNumber;
		m_softwareBinaryID = softwareBinaryID;
		m_softwareProductInformation = softwareProductInformation;
		m_softwareInstallDate = softwareInstallDate;
	}

	[finalmq.MetaField("")]
    public hl7.XON softwareVendorOrganization
	{
		get { return m_softwareVendorOrganization; }
		set { m_softwareVendorOrganization = value; }
	}
	[finalmq.MetaField("")]
    public string softwareCertifiedVersionOrReleaseNumber
	{
		get { return m_softwareCertifiedVersionOrReleaseNumber; }
		set { m_softwareCertifiedVersionOrReleaseNumber = value; }
	}
	[finalmq.MetaField("")]
    public string softwareBinaryID
	{
		get { return m_softwareBinaryID; }
		set { m_softwareBinaryID = value; }
	}
	[finalmq.MetaField("")]
    public string softwareProductInformation
	{
		get { return m_softwareProductInformation; }
		set { m_softwareProductInformation = value; }
	}
	[finalmq.MetaField("")]
    public string softwareInstallDate
	{
		get { return m_softwareInstallDate; }
		set { m_softwareInstallDate = value; }
	}

    hl7.XON m_softwareVendorOrganization = new hl7.XON();
    string m_softwareCertifiedVersionOrReleaseNumber = string.Empty;
    string m_softwareBinaryID = string.Empty;
    string m_softwareProductInformation = string.Empty;
    string m_softwareInstallDate = string.Empty;

	public bool Equals(SFT? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_softwareVendorOrganization.Equals(rhs.m_softwareVendorOrganization))
		{
			return false;
		}
		if (m_softwareCertifiedVersionOrReleaseNumber != rhs.m_softwareCertifiedVersionOrReleaseNumber)
		{
			return false;
		}
		if (m_softwareBinaryID != rhs.m_softwareBinaryID)
		{
			return false;
		}
		if (m_softwareProductInformation != rhs.m_softwareProductInformation)
		{
			return false;
		}
		if (m_softwareInstallDate != rhs.m_softwareInstallDate)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SFT));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SFT));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SFT), m_metaStruct, () => { return new SFT(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class UAC : finalmq.StructBase, IEquatable<UAC>
{
    public UAC()
	{
	}
	

    public UAC(hl7.CWE userAuthenticationCredentialTypeCode, hl7.ED userAuthenticationCredential)
	{
		m_userAuthenticationCredentialTypeCode = userAuthenticationCredentialTypeCode;
		m_userAuthenticationCredential = userAuthenticationCredential;
	}

	[finalmq.MetaField("")]
    public hl7.CWE userAuthenticationCredentialTypeCode
	{
		get { return m_userAuthenticationCredentialTypeCode; }
		set { m_userAuthenticationCredentialTypeCode = value; }
	}
	[finalmq.MetaField("")]
    public hl7.ED userAuthenticationCredential
	{
		get { return m_userAuthenticationCredential; }
		set { m_userAuthenticationCredential = value; }
	}

    hl7.CWE m_userAuthenticationCredentialTypeCode = new hl7.CWE();
    hl7.ED m_userAuthenticationCredential = new hl7.ED();

	public bool Equals(UAC? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_userAuthenticationCredentialTypeCode.Equals(rhs.m_userAuthenticationCredentialTypeCode))
		{
			return false;
		}
		if (!m_userAuthenticationCredential.Equals(rhs.m_userAuthenticationCredential))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(UAC));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(UAC));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(UAC), m_metaStruct, () => { return new UAC(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class MSA : finalmq.StructBase, IEquatable<MSA>
{
    public MSA()
	{
	}
	

    public MSA(string acknowledgementCode, string messageControlId, string textMessage, string expectedSequenceNumber, string delayedAcknowledgementType, string errorCondition, string messageWaitingNumber, string messageWaitingPriority)
	{
		m_acknowledgementCode = acknowledgementCode;
		m_messageControlId = messageControlId;
		m_textMessage = textMessage;
		m_expectedSequenceNumber = expectedSequenceNumber;
		m_delayedAcknowledgementType = delayedAcknowledgementType;
		m_errorCondition = errorCondition;
		m_messageWaitingNumber = messageWaitingNumber;
		m_messageWaitingPriority = messageWaitingPriority;
	}

	[finalmq.MetaField("")]
    public string acknowledgementCode
	{
		get { return m_acknowledgementCode; }
		set { m_acknowledgementCode = value; }
	}
	[finalmq.MetaField("")]
    public string messageControlId
	{
		get { return m_messageControlId; }
		set { m_messageControlId = value; }
	}
	[finalmq.MetaField("")]
    public string textMessage
	{
		get { return m_textMessage; }
		set { m_textMessage = value; }
	}
	[finalmq.MetaField("")]
    public string expectedSequenceNumber
	{
		get { return m_expectedSequenceNumber; }
		set { m_expectedSequenceNumber = value; }
	}
	[finalmq.MetaField("")]
    public string delayedAcknowledgementType
	{
		get { return m_delayedAcknowledgementType; }
		set { m_delayedAcknowledgementType = value; }
	}
	[finalmq.MetaField("")]
    public string errorCondition
	{
		get { return m_errorCondition; }
		set { m_errorCondition = value; }
	}
	[finalmq.MetaField("")]
    public string messageWaitingNumber
	{
		get { return m_messageWaitingNumber; }
		set { m_messageWaitingNumber = value; }
	}
	[finalmq.MetaField("")]
    public string messageWaitingPriority
	{
		get { return m_messageWaitingPriority; }
		set { m_messageWaitingPriority = value; }
	}

    string m_acknowledgementCode = string.Empty;
    string m_messageControlId = string.Empty;
    string m_textMessage = string.Empty;
    string m_expectedSequenceNumber = string.Empty;
    string m_delayedAcknowledgementType = string.Empty;
    string m_errorCondition = string.Empty;
    string m_messageWaitingNumber = string.Empty;
    string m_messageWaitingPriority = string.Empty;

	public bool Equals(MSA? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_acknowledgementCode != rhs.m_acknowledgementCode)
		{
			return false;
		}
		if (m_messageControlId != rhs.m_messageControlId)
		{
			return false;
		}
		if (m_textMessage != rhs.m_textMessage)
		{
			return false;
		}
		if (m_expectedSequenceNumber != rhs.m_expectedSequenceNumber)
		{
			return false;
		}
		if (m_delayedAcknowledgementType != rhs.m_delayedAcknowledgementType)
		{
			return false;
		}
		if (m_errorCondition != rhs.m_errorCondition)
		{
			return false;
		}
		if (m_messageWaitingNumber != rhs.m_messageWaitingNumber)
		{
			return false;
		}
		if (m_messageWaitingPriority != rhs.m_messageWaitingPriority)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSA));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSA));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSA), m_metaStruct, () => { return new MSA(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class ERR : finalmq.StructBase, IEquatable<ERR>
{
    public ERR()
	{
	}
	

    public ERR(string errorCodeAndLocation, hl7.ERL errorLocation, hl7.CWE hl7ErrorCode, string severity, hl7.CWE applicationErrorCode, string applicationErrorParameter, string diagnosticInformation, string userMessage, hl7.CWE informPersonIndicator, hl7.CWE overrideType, hl7.CWE overrideReasonCode, string helpDeskContactPoint)
	{
		m_errorCodeAndLocation = errorCodeAndLocation;
		m_errorLocation = errorLocation;
		m_hl7ErrorCode = hl7ErrorCode;
		m_severity = severity;
		m_applicationErrorCode = applicationErrorCode;
		m_applicationErrorParameter = applicationErrorParameter;
		m_diagnosticInformation = diagnosticInformation;
		m_userMessage = userMessage;
		m_informPersonIndicator = informPersonIndicator;
		m_overrideType = overrideType;
		m_overrideReasonCode = overrideReasonCode;
		m_helpDeskContactPoint = helpDeskContactPoint;
	}

	[finalmq.MetaField("")]
    public string errorCodeAndLocation
	{
		get { return m_errorCodeAndLocation; }
		set { m_errorCodeAndLocation = value; }
	}
	[finalmq.MetaField("")]
    public hl7.ERL errorLocation
	{
		get { return m_errorLocation; }
		set { m_errorLocation = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE hl7ErrorCode
	{
		get { return m_hl7ErrorCode; }
		set { m_hl7ErrorCode = value; }
	}
	[finalmq.MetaField("")]
    public string severity
	{
		get { return m_severity; }
		set { m_severity = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE applicationErrorCode
	{
		get { return m_applicationErrorCode; }
		set { m_applicationErrorCode = value; }
	}
	[finalmq.MetaField("")]
    public string applicationErrorParameter
	{
		get { return m_applicationErrorParameter; }
		set { m_applicationErrorParameter = value; }
	}
	[finalmq.MetaField("")]
    public string diagnosticInformation
	{
		get { return m_diagnosticInformation; }
		set { m_diagnosticInformation = value; }
	}
	[finalmq.MetaField("")]
    public string userMessage
	{
		get { return m_userMessage; }
		set { m_userMessage = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE informPersonIndicator
	{
		get { return m_informPersonIndicator; }
		set { m_informPersonIndicator = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE overrideType
	{
		get { return m_overrideType; }
		set { m_overrideType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE overrideReasonCode
	{
		get { return m_overrideReasonCode; }
		set { m_overrideReasonCode = value; }
	}
	[finalmq.MetaField("")]
    public string helpDeskContactPoint
	{
		get { return m_helpDeskContactPoint; }
		set { m_helpDeskContactPoint = value; }
	}

    string m_errorCodeAndLocation = string.Empty;
    hl7.ERL m_errorLocation = new hl7.ERL();
    hl7.CWE m_hl7ErrorCode = new hl7.CWE();
    string m_severity = string.Empty;
    hl7.CWE m_applicationErrorCode = new hl7.CWE();
    string m_applicationErrorParameter = string.Empty;
    string m_diagnosticInformation = string.Empty;
    string m_userMessage = string.Empty;
    hl7.CWE m_informPersonIndicator = new hl7.CWE();
    hl7.CWE m_overrideType = new hl7.CWE();
    hl7.CWE m_overrideReasonCode = new hl7.CWE();
    string m_helpDeskContactPoint = string.Empty;

	public bool Equals(ERR? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_errorCodeAndLocation != rhs.m_errorCodeAndLocation)
		{
			return false;
		}
		if (!m_errorLocation.Equals(rhs.m_errorLocation))
		{
			return false;
		}
		if (!m_hl7ErrorCode.Equals(rhs.m_hl7ErrorCode))
		{
			return false;
		}
		if (m_severity != rhs.m_severity)
		{
			return false;
		}
		if (!m_applicationErrorCode.Equals(rhs.m_applicationErrorCode))
		{
			return false;
		}
		if (m_applicationErrorParameter != rhs.m_applicationErrorParameter)
		{
			return false;
		}
		if (m_diagnosticInformation != rhs.m_diagnosticInformation)
		{
			return false;
		}
		if (m_userMessage != rhs.m_userMessage)
		{
			return false;
		}
		if (!m_informPersonIndicator.Equals(rhs.m_informPersonIndicator))
		{
			return false;
		}
		if (!m_overrideType.Equals(rhs.m_overrideType))
		{
			return false;
		}
		if (!m_overrideReasonCode.Equals(rhs.m_overrideReasonCode))
		{
			return false;
		}
		if (m_helpDeskContactPoint != rhs.m_helpDeskContactPoint)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ERR));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ERR));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ERR), m_metaStruct, () => { return new ERR(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class SAC : finalmq.StructBase, IEquatable<SAC>
{
    public SAC()
	{
	}
	

    public SAC(hl7.EI externalAccessionIdentifier, hl7.EI accessionIdentifier, hl7.EI containerIdentifier, hl7.EI primaryParentContainerIdentifier, hl7.EI equipmentContainerIdentifier, string specimenSource, string registrationDateTime, hl7.CWE containerStatus, hl7.CWE carrierType, hl7.EI carrierId, string positionInCarrier, hl7.CWE trayTypeSAC, hl7.EI trayIdentifier, string positionInTray, hl7.CWE location, string containerHeight, string containerDiamenter, string barrierDelta, string bottomDelta, hl7.CWE containerHeightDiameterUnit, string containerVolume, string availableSpecimenVolume, string initialSpecimenVolume)
	{
		m_externalAccessionIdentifier = externalAccessionIdentifier;
		m_accessionIdentifier = accessionIdentifier;
		m_containerIdentifier = containerIdentifier;
		m_primaryParentContainerIdentifier = primaryParentContainerIdentifier;
		m_equipmentContainerIdentifier = equipmentContainerIdentifier;
		m_specimenSource = specimenSource;
		m_registrationDateTime = registrationDateTime;
		m_containerStatus = containerStatus;
		m_carrierType = carrierType;
		m_carrierId = carrierId;
		m_positionInCarrier = positionInCarrier;
		m_trayTypeSAC = trayTypeSAC;
		m_trayIdentifier = trayIdentifier;
		m_positionInTray = positionInTray;
		m_location = location;
		m_containerHeight = containerHeight;
		m_containerDiamenter = containerDiamenter;
		m_barrierDelta = barrierDelta;
		m_bottomDelta = bottomDelta;
		m_containerHeightDiameterUnit = containerHeightDiameterUnit;
		m_containerVolume = containerVolume;
		m_availableSpecimenVolume = availableSpecimenVolume;
		m_initialSpecimenVolume = initialSpecimenVolume;
	}

	[finalmq.MetaField("")]
    public hl7.EI externalAccessionIdentifier
	{
		get { return m_externalAccessionIdentifier; }
		set { m_externalAccessionIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI accessionIdentifier
	{
		get { return m_accessionIdentifier; }
		set { m_accessionIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI containerIdentifier
	{
		get { return m_containerIdentifier; }
		set { m_containerIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI primaryParentContainerIdentifier
	{
		get { return m_primaryParentContainerIdentifier; }
		set { m_primaryParentContainerIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI equipmentContainerIdentifier
	{
		get { return m_equipmentContainerIdentifier; }
		set { m_equipmentContainerIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string specimenSource
	{
		get { return m_specimenSource; }
		set { m_specimenSource = value; }
	}
	[finalmq.MetaField("")]
    public string registrationDateTime
	{
		get { return m_registrationDateTime; }
		set { m_registrationDateTime = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE containerStatus
	{
		get { return m_containerStatus; }
		set { m_containerStatus = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE carrierType
	{
		get { return m_carrierType; }
		set { m_carrierType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI carrierId
	{
		get { return m_carrierId; }
		set { m_carrierId = value; }
	}
	[finalmq.MetaField("")]
    public string positionInCarrier
	{
		get { return m_positionInCarrier; }
		set { m_positionInCarrier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE trayTypeSAC
	{
		get { return m_trayTypeSAC; }
		set { m_trayTypeSAC = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI trayIdentifier
	{
		get { return m_trayIdentifier; }
		set { m_trayIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string positionInTray
	{
		get { return m_positionInTray; }
		set { m_positionInTray = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE location
	{
		get { return m_location; }
		set { m_location = value; }
	}
	[finalmq.MetaField("")]
    public string containerHeight
	{
		get { return m_containerHeight; }
		set { m_containerHeight = value; }
	}
	[finalmq.MetaField("")]
    public string containerDiamenter
	{
		get { return m_containerDiamenter; }
		set { m_containerDiamenter = value; }
	}
	[finalmq.MetaField("")]
    public string barrierDelta
	{
		get { return m_barrierDelta; }
		set { m_barrierDelta = value; }
	}
	[finalmq.MetaField("")]
    public string bottomDelta
	{
		get { return m_bottomDelta; }
		set { m_bottomDelta = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE containerHeightDiameterUnit
	{
		get { return m_containerHeightDiameterUnit; }
		set { m_containerHeightDiameterUnit = value; }
	}
	[finalmq.MetaField("")]
    public string containerVolume
	{
		get { return m_containerVolume; }
		set { m_containerVolume = value; }
	}
	[finalmq.MetaField("")]
    public string availableSpecimenVolume
	{
		get { return m_availableSpecimenVolume; }
		set { m_availableSpecimenVolume = value; }
	}
	[finalmq.MetaField("")]
    public string initialSpecimenVolume
	{
		get { return m_initialSpecimenVolume; }
		set { m_initialSpecimenVolume = value; }
	}

    hl7.EI m_externalAccessionIdentifier = new hl7.EI();
    hl7.EI m_accessionIdentifier = new hl7.EI();
    hl7.EI m_containerIdentifier = new hl7.EI();
    hl7.EI m_primaryParentContainerIdentifier = new hl7.EI();
    hl7.EI m_equipmentContainerIdentifier = new hl7.EI();
    string m_specimenSource = string.Empty;
    string m_registrationDateTime = string.Empty;
    hl7.CWE m_containerStatus = new hl7.CWE();
    hl7.CWE m_carrierType = new hl7.CWE();
    hl7.EI m_carrierId = new hl7.EI();
    string m_positionInCarrier = string.Empty;
    hl7.CWE m_trayTypeSAC = new hl7.CWE();
    hl7.EI m_trayIdentifier = new hl7.EI();
    string m_positionInTray = string.Empty;
    hl7.CWE m_location = new hl7.CWE();
    string m_containerHeight = string.Empty;
    string m_containerDiamenter = string.Empty;
    string m_barrierDelta = string.Empty;
    string m_bottomDelta = string.Empty;
    hl7.CWE m_containerHeightDiameterUnit = new hl7.CWE();
    string m_containerVolume = string.Empty;
    string m_availableSpecimenVolume = string.Empty;
    string m_initialSpecimenVolume = string.Empty;

	public bool Equals(SAC? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_externalAccessionIdentifier.Equals(rhs.m_externalAccessionIdentifier))
		{
			return false;
		}
		if (!m_accessionIdentifier.Equals(rhs.m_accessionIdentifier))
		{
			return false;
		}
		if (!m_containerIdentifier.Equals(rhs.m_containerIdentifier))
		{
			return false;
		}
		if (!m_primaryParentContainerIdentifier.Equals(rhs.m_primaryParentContainerIdentifier))
		{
			return false;
		}
		if (!m_equipmentContainerIdentifier.Equals(rhs.m_equipmentContainerIdentifier))
		{
			return false;
		}
		if (m_specimenSource != rhs.m_specimenSource)
		{
			return false;
		}
		if (m_registrationDateTime != rhs.m_registrationDateTime)
		{
			return false;
		}
		if (!m_containerStatus.Equals(rhs.m_containerStatus))
		{
			return false;
		}
		if (!m_carrierType.Equals(rhs.m_carrierType))
		{
			return false;
		}
		if (!m_carrierId.Equals(rhs.m_carrierId))
		{
			return false;
		}
		if (m_positionInCarrier != rhs.m_positionInCarrier)
		{
			return false;
		}
		if (!m_trayTypeSAC.Equals(rhs.m_trayTypeSAC))
		{
			return false;
		}
		if (!m_trayIdentifier.Equals(rhs.m_trayIdentifier))
		{
			return false;
		}
		if (m_positionInTray != rhs.m_positionInTray)
		{
			return false;
		}
		if (!m_location.Equals(rhs.m_location))
		{
			return false;
		}
		if (m_containerHeight != rhs.m_containerHeight)
		{
			return false;
		}
		if (m_containerDiamenter != rhs.m_containerDiamenter)
		{
			return false;
		}
		if (m_barrierDelta != rhs.m_barrierDelta)
		{
			return false;
		}
		if (m_bottomDelta != rhs.m_bottomDelta)
		{
			return false;
		}
		if (!m_containerHeightDiameterUnit.Equals(rhs.m_containerHeightDiameterUnit))
		{
			return false;
		}
		if (m_containerVolume != rhs.m_containerVolume)
		{
			return false;
		}
		if (m_availableSpecimenVolume != rhs.m_availableSpecimenVolume)
		{
			return false;
		}
		if (m_initialSpecimenVolume != rhs.m_initialSpecimenVolume)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SAC));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SAC));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SAC), m_metaStruct, () => { return new SAC(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class OBX : finalmq.StructBase, IEquatable<OBX>
{
    public OBX()
	{
	}
	

    public OBX(string setId, string valueType, hl7.CWE observationIdentifier, string observationSubId, string observationValue, hl7.CWE units, string referencesRange, hl7.CWE interpretationCodes, string probability, string natureOfAbnormalTest, string observationResultStatus, string effectiveDateOfReferenceRange, string userDefinedAccessChecks, string dateTimeOfTheObservation, hl7.CWE producersId, hl7.XCN responsibleObserver, hl7.CWE observationMethod, hl7.EI equipmentInstanceIdentifier, string dateTimeOfTheAnalysis, hl7.CWE observationSite, hl7.EI observationInstanceIdentifier, string moodCode, hl7.XON performingOrganizationName, string performingOrganizationAddress, hl7.XCN performingOrganizationMedicalDirector, string patientsResultsReleaseCategory)
	{
		m_setId = setId;
		m_valueType = valueType;
		m_observationIdentifier = observationIdentifier;
		m_observationSubId = observationSubId;
		m_observationValue = observationValue;
		m_units = units;
		m_referencesRange = referencesRange;
		m_interpretationCodes = interpretationCodes;
		m_probability = probability;
		m_natureOfAbnormalTest = natureOfAbnormalTest;
		m_observationResultStatus = observationResultStatus;
		m_effectiveDateOfReferenceRange = effectiveDateOfReferenceRange;
		m_userDefinedAccessChecks = userDefinedAccessChecks;
		m_dateTimeOfTheObservation = dateTimeOfTheObservation;
		m_producersId = producersId;
		m_responsibleObserver = responsibleObserver;
		m_observationMethod = observationMethod;
		m_equipmentInstanceIdentifier = equipmentInstanceIdentifier;
		m_dateTimeOfTheAnalysis = dateTimeOfTheAnalysis;
		m_observationSite = observationSite;
		m_observationInstanceIdentifier = observationInstanceIdentifier;
		m_moodCode = moodCode;
		m_performingOrganizationName = performingOrganizationName;
		m_performingOrganizationAddress = performingOrganizationAddress;
		m_performingOrganizationMedicalDirector = performingOrganizationMedicalDirector;
		m_patientsResultsReleaseCategory = patientsResultsReleaseCategory;
	}

	[finalmq.MetaField("")]
    public string setId
	{
		get { return m_setId; }
		set { m_setId = value; }
	}
	[finalmq.MetaField("")]
    public string valueType
	{
		get { return m_valueType; }
		set { m_valueType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE observationIdentifier
	{
		get { return m_observationIdentifier; }
		set { m_observationIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string observationSubId
	{
		get { return m_observationSubId; }
		set { m_observationSubId = value; }
	}
	[finalmq.MetaField("")]
    public string observationValue
	{
		get { return m_observationValue; }
		set { m_observationValue = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE units
	{
		get { return m_units; }
		set { m_units = value; }
	}
	[finalmq.MetaField("")]
    public string referencesRange
	{
		get { return m_referencesRange; }
		set { m_referencesRange = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE interpretationCodes
	{
		get { return m_interpretationCodes; }
		set { m_interpretationCodes = value; }
	}
	[finalmq.MetaField("")]
    public string probability
	{
		get { return m_probability; }
		set { m_probability = value; }
	}
	[finalmq.MetaField("")]
    public string natureOfAbnormalTest
	{
		get { return m_natureOfAbnormalTest; }
		set { m_natureOfAbnormalTest = value; }
	}
	[finalmq.MetaField("")]
    public string observationResultStatus
	{
		get { return m_observationResultStatus; }
		set { m_observationResultStatus = value; }
	}
	[finalmq.MetaField("")]
    public string effectiveDateOfReferenceRange
	{
		get { return m_effectiveDateOfReferenceRange; }
		set { m_effectiveDateOfReferenceRange = value; }
	}
	[finalmq.MetaField("")]
    public string userDefinedAccessChecks
	{
		get { return m_userDefinedAccessChecks; }
		set { m_userDefinedAccessChecks = value; }
	}
	[finalmq.MetaField("")]
    public string dateTimeOfTheObservation
	{
		get { return m_dateTimeOfTheObservation; }
		set { m_dateTimeOfTheObservation = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE producersId
	{
		get { return m_producersId; }
		set { m_producersId = value; }
	}
	[finalmq.MetaField("")]
    public hl7.XCN responsibleObserver
	{
		get { return m_responsibleObserver; }
		set { m_responsibleObserver = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE observationMethod
	{
		get { return m_observationMethod; }
		set { m_observationMethod = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI equipmentInstanceIdentifier
	{
		get { return m_equipmentInstanceIdentifier; }
		set { m_equipmentInstanceIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string dateTimeOfTheAnalysis
	{
		get { return m_dateTimeOfTheAnalysis; }
		set { m_dateTimeOfTheAnalysis = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE observationSite
	{
		get { return m_observationSite; }
		set { m_observationSite = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI observationInstanceIdentifier
	{
		get { return m_observationInstanceIdentifier; }
		set { m_observationInstanceIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string moodCode
	{
		get { return m_moodCode; }
		set { m_moodCode = value; }
	}
	[finalmq.MetaField("")]
    public hl7.XON performingOrganizationName
	{
		get { return m_performingOrganizationName; }
		set { m_performingOrganizationName = value; }
	}
	[finalmq.MetaField("")]
    public string performingOrganizationAddress
	{
		get { return m_performingOrganizationAddress; }
		set { m_performingOrganizationAddress = value; }
	}
	[finalmq.MetaField("")]
    public hl7.XCN performingOrganizationMedicalDirector
	{
		get { return m_performingOrganizationMedicalDirector; }
		set { m_performingOrganizationMedicalDirector = value; }
	}
	[finalmq.MetaField("")]
    public string patientsResultsReleaseCategory
	{
		get { return m_patientsResultsReleaseCategory; }
		set { m_patientsResultsReleaseCategory = value; }
	}

    string m_setId = string.Empty;
    string m_valueType = string.Empty;
    hl7.CWE m_observationIdentifier = new hl7.CWE();
    string m_observationSubId = string.Empty;
    string m_observationValue = string.Empty;
    hl7.CWE m_units = new hl7.CWE();
    string m_referencesRange = string.Empty;
    hl7.CWE m_interpretationCodes = new hl7.CWE();
    string m_probability = string.Empty;
    string m_natureOfAbnormalTest = string.Empty;
    string m_observationResultStatus = string.Empty;
    string m_effectiveDateOfReferenceRange = string.Empty;
    string m_userDefinedAccessChecks = string.Empty;
    string m_dateTimeOfTheObservation = string.Empty;
    hl7.CWE m_producersId = new hl7.CWE();
    hl7.XCN m_responsibleObserver = new hl7.XCN();
    hl7.CWE m_observationMethod = new hl7.CWE();
    hl7.EI m_equipmentInstanceIdentifier = new hl7.EI();
    string m_dateTimeOfTheAnalysis = string.Empty;
    hl7.CWE m_observationSite = new hl7.CWE();
    hl7.EI m_observationInstanceIdentifier = new hl7.EI();
    string m_moodCode = string.Empty;
    hl7.XON m_performingOrganizationName = new hl7.XON();
    string m_performingOrganizationAddress = string.Empty;
    hl7.XCN m_performingOrganizationMedicalDirector = new hl7.XCN();
    string m_patientsResultsReleaseCategory = string.Empty;

	public bool Equals(OBX? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_setId != rhs.m_setId)
		{
			return false;
		}
		if (m_valueType != rhs.m_valueType)
		{
			return false;
		}
		if (!m_observationIdentifier.Equals(rhs.m_observationIdentifier))
		{
			return false;
		}
		if (m_observationSubId != rhs.m_observationSubId)
		{
			return false;
		}
		if (m_observationValue != rhs.m_observationValue)
		{
			return false;
		}
		if (!m_units.Equals(rhs.m_units))
		{
			return false;
		}
		if (m_referencesRange != rhs.m_referencesRange)
		{
			return false;
		}
		if (!m_interpretationCodes.Equals(rhs.m_interpretationCodes))
		{
			return false;
		}
		if (m_probability != rhs.m_probability)
		{
			return false;
		}
		if (m_natureOfAbnormalTest != rhs.m_natureOfAbnormalTest)
		{
			return false;
		}
		if (m_observationResultStatus != rhs.m_observationResultStatus)
		{
			return false;
		}
		if (m_effectiveDateOfReferenceRange != rhs.m_effectiveDateOfReferenceRange)
		{
			return false;
		}
		if (m_userDefinedAccessChecks != rhs.m_userDefinedAccessChecks)
		{
			return false;
		}
		if (m_dateTimeOfTheObservation != rhs.m_dateTimeOfTheObservation)
		{
			return false;
		}
		if (!m_producersId.Equals(rhs.m_producersId))
		{
			return false;
		}
		if (!m_responsibleObserver.Equals(rhs.m_responsibleObserver))
		{
			return false;
		}
		if (!m_observationMethod.Equals(rhs.m_observationMethod))
		{
			return false;
		}
		if (!m_equipmentInstanceIdentifier.Equals(rhs.m_equipmentInstanceIdentifier))
		{
			return false;
		}
		if (m_dateTimeOfTheAnalysis != rhs.m_dateTimeOfTheAnalysis)
		{
			return false;
		}
		if (!m_observationSite.Equals(rhs.m_observationSite))
		{
			return false;
		}
		if (!m_observationInstanceIdentifier.Equals(rhs.m_observationInstanceIdentifier))
		{
			return false;
		}
		if (m_moodCode != rhs.m_moodCode)
		{
			return false;
		}
		if (!m_performingOrganizationName.Equals(rhs.m_performingOrganizationName))
		{
			return false;
		}
		if (m_performingOrganizationAddress != rhs.m_performingOrganizationAddress)
		{
			return false;
		}
		if (!m_performingOrganizationMedicalDirector.Equals(rhs.m_performingOrganizationMedicalDirector))
		{
			return false;
		}
		if (m_patientsResultsReleaseCategory != rhs.m_patientsResultsReleaseCategory)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(OBX));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(OBX));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(OBX), m_metaStruct, () => { return new OBX(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class NTE : finalmq.StructBase, IEquatable<NTE>
{
    public NTE()
	{
	}
	

    public NTE(string setId, string sourceOfComment, string comment, hl7.CWE commentType, hl7.XCN enteredBy, string enteredDateTime, string effectiveStartDate, string expirationDate)
	{
		m_setId = setId;
		m_sourceOfComment = sourceOfComment;
		m_comment = comment;
		m_commentType = commentType;
		m_enteredBy = enteredBy;
		m_enteredDateTime = enteredDateTime;
		m_effectiveStartDate = effectiveStartDate;
		m_expirationDate = expirationDate;
	}

	[finalmq.MetaField("")]
    public string setId
	{
		get { return m_setId; }
		set { m_setId = value; }
	}
	[finalmq.MetaField("")]
    public string sourceOfComment
	{
		get { return m_sourceOfComment; }
		set { m_sourceOfComment = value; }
	}
	[finalmq.MetaField("")]
    public string comment
	{
		get { return m_comment; }
		set { m_comment = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE commentType
	{
		get { return m_commentType; }
		set { m_commentType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.XCN enteredBy
	{
		get { return m_enteredBy; }
		set { m_enteredBy = value; }
	}
	[finalmq.MetaField("")]
    public string enteredDateTime
	{
		get { return m_enteredDateTime; }
		set { m_enteredDateTime = value; }
	}
	[finalmq.MetaField("")]
    public string effectiveStartDate
	{
		get { return m_effectiveStartDate; }
		set { m_effectiveStartDate = value; }
	}
	[finalmq.MetaField("")]
    public string expirationDate
	{
		get { return m_expirationDate; }
		set { m_expirationDate = value; }
	}

    string m_setId = string.Empty;
    string m_sourceOfComment = string.Empty;
    string m_comment = string.Empty;
    hl7.CWE m_commentType = new hl7.CWE();
    hl7.XCN m_enteredBy = new hl7.XCN();
    string m_enteredDateTime = string.Empty;
    string m_effectiveStartDate = string.Empty;
    string m_expirationDate = string.Empty;

	public bool Equals(NTE? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_setId != rhs.m_setId)
		{
			return false;
		}
		if (m_sourceOfComment != rhs.m_sourceOfComment)
		{
			return false;
		}
		if (m_comment != rhs.m_comment)
		{
			return false;
		}
		if (!m_commentType.Equals(rhs.m_commentType))
		{
			return false;
		}
		if (!m_enteredBy.Equals(rhs.m_enteredBy))
		{
			return false;
		}
		if (m_enteredDateTime != rhs.m_enteredDateTime)
		{
			return false;
		}
		if (m_effectiveStartDate != rhs.m_effectiveStartDate)
		{
			return false;
		}
		if (m_expirationDate != rhs.m_expirationDate)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(NTE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(NTE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(NTE), m_metaStruct, () => { return new NTE(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class SPM : finalmq.StructBase, IEquatable<SPM>
{
    public SPM()
	{
	}
	

    public SPM(string setIdSpm, string specimenId, string specimenParentIds, hl7.CWE specimenType, hl7.CWE specimenTypeModifier, hl7.CWE specimenAdditives, hl7.CWE specimenCollectionMethod, hl7.CWE specimenSourceSite, hl7.CWE specimenSourceSiteModifier, hl7.CWE specimenCollectionSite, hl7.CWE specimenRole, string specimenCollectionAmount, string groupedSpecimenCount, string specimenDescription, hl7.CWE specimenHandlingCode, hl7.CWE specimenRiskCode, string specimenCollectionDateTime, string specimenReceivedDateTime, string specimenExpirationDateTime, string specimenAvailabiity, hl7.CWE specimenRejectReason, hl7.CWE specimenQuality, hl7.CWE specimenAppropriateness, hl7.CWE specimenCondition, string specimenCurrentQuantity, string numberOfSpecimenContainers, hl7.CWE containerType, hl7.CWE containerCondition, hl7.CWE specimenChildRole, string accessionId, string otherSpecimenId, hl7.EI shipmentId)
	{
		m_setIdSpm = setIdSpm;
		m_specimenId = specimenId;
		m_specimenParentIds = specimenParentIds;
		m_specimenType = specimenType;
		m_specimenTypeModifier = specimenTypeModifier;
		m_specimenAdditives = specimenAdditives;
		m_specimenCollectionMethod = specimenCollectionMethod;
		m_specimenSourceSite = specimenSourceSite;
		m_specimenSourceSiteModifier = specimenSourceSiteModifier;
		m_specimenCollectionSite = specimenCollectionSite;
		m_specimenRole = specimenRole;
		m_specimenCollectionAmount = specimenCollectionAmount;
		m_groupedSpecimenCount = groupedSpecimenCount;
		m_specimenDescription = specimenDescription;
		m_specimenHandlingCode = specimenHandlingCode;
		m_specimenRiskCode = specimenRiskCode;
		m_specimenCollectionDateTime = specimenCollectionDateTime;
		m_specimenReceivedDateTime = specimenReceivedDateTime;
		m_specimenExpirationDateTime = specimenExpirationDateTime;
		m_specimenAvailabiity = specimenAvailabiity;
		m_specimenRejectReason = specimenRejectReason;
		m_specimenQuality = specimenQuality;
		m_specimenAppropriateness = specimenAppropriateness;
		m_specimenCondition = specimenCondition;
		m_specimenCurrentQuantity = specimenCurrentQuantity;
		m_numberOfSpecimenContainers = numberOfSpecimenContainers;
		m_containerType = containerType;
		m_containerCondition = containerCondition;
		m_specimenChildRole = specimenChildRole;
		m_accessionId = accessionId;
		m_otherSpecimenId = otherSpecimenId;
		m_shipmentId = shipmentId;
	}

	[finalmq.MetaField("")]
    public string setIdSpm
	{
		get { return m_setIdSpm; }
		set { m_setIdSpm = value; }
	}
	[finalmq.MetaField("")]
    public string specimenId
	{
		get { return m_specimenId; }
		set { m_specimenId = value; }
	}
	[finalmq.MetaField("")]
    public string specimenParentIds
	{
		get { return m_specimenParentIds; }
		set { m_specimenParentIds = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenType
	{
		get { return m_specimenType; }
		set { m_specimenType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenTypeModifier
	{
		get { return m_specimenTypeModifier; }
		set { m_specimenTypeModifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenAdditives
	{
		get { return m_specimenAdditives; }
		set { m_specimenAdditives = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenCollectionMethod
	{
		get { return m_specimenCollectionMethod; }
		set { m_specimenCollectionMethod = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenSourceSite
	{
		get { return m_specimenSourceSite; }
		set { m_specimenSourceSite = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenSourceSiteModifier
	{
		get { return m_specimenSourceSiteModifier; }
		set { m_specimenSourceSiteModifier = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenCollectionSite
	{
		get { return m_specimenCollectionSite; }
		set { m_specimenCollectionSite = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenRole
	{
		get { return m_specimenRole; }
		set { m_specimenRole = value; }
	}
	[finalmq.MetaField("")]
    public string specimenCollectionAmount
	{
		get { return m_specimenCollectionAmount; }
		set { m_specimenCollectionAmount = value; }
	}
	[finalmq.MetaField("")]
    public string groupedSpecimenCount
	{
		get { return m_groupedSpecimenCount; }
		set { m_groupedSpecimenCount = value; }
	}
	[finalmq.MetaField("")]
    public string specimenDescription
	{
		get { return m_specimenDescription; }
		set { m_specimenDescription = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenHandlingCode
	{
		get { return m_specimenHandlingCode; }
		set { m_specimenHandlingCode = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenRiskCode
	{
		get { return m_specimenRiskCode; }
		set { m_specimenRiskCode = value; }
	}
	[finalmq.MetaField("")]
    public string specimenCollectionDateTime
	{
		get { return m_specimenCollectionDateTime; }
		set { m_specimenCollectionDateTime = value; }
	}
	[finalmq.MetaField("")]
    public string specimenReceivedDateTime
	{
		get { return m_specimenReceivedDateTime; }
		set { m_specimenReceivedDateTime = value; }
	}
	[finalmq.MetaField("")]
    public string specimenExpirationDateTime
	{
		get { return m_specimenExpirationDateTime; }
		set { m_specimenExpirationDateTime = value; }
	}
	[finalmq.MetaField("")]
    public string specimenAvailabiity
	{
		get { return m_specimenAvailabiity; }
		set { m_specimenAvailabiity = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenRejectReason
	{
		get { return m_specimenRejectReason; }
		set { m_specimenRejectReason = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenQuality
	{
		get { return m_specimenQuality; }
		set { m_specimenQuality = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenAppropriateness
	{
		get { return m_specimenAppropriateness; }
		set { m_specimenAppropriateness = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenCondition
	{
		get { return m_specimenCondition; }
		set { m_specimenCondition = value; }
	}
	[finalmq.MetaField("")]
    public string specimenCurrentQuantity
	{
		get { return m_specimenCurrentQuantity; }
		set { m_specimenCurrentQuantity = value; }
	}
	[finalmq.MetaField("")]
    public string numberOfSpecimenContainers
	{
		get { return m_numberOfSpecimenContainers; }
		set { m_numberOfSpecimenContainers = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE containerType
	{
		get { return m_containerType; }
		set { m_containerType = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE containerCondition
	{
		get { return m_containerCondition; }
		set { m_containerCondition = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE specimenChildRole
	{
		get { return m_specimenChildRole; }
		set { m_specimenChildRole = value; }
	}
	[finalmq.MetaField("")]
    public string accessionId
	{
		get { return m_accessionId; }
		set { m_accessionId = value; }
	}
	[finalmq.MetaField("")]
    public string otherSpecimenId
	{
		get { return m_otherSpecimenId; }
		set { m_otherSpecimenId = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EI shipmentId
	{
		get { return m_shipmentId; }
		set { m_shipmentId = value; }
	}

    string m_setIdSpm = string.Empty;
    string m_specimenId = string.Empty;
    string m_specimenParentIds = string.Empty;
    hl7.CWE m_specimenType = new hl7.CWE();
    hl7.CWE m_specimenTypeModifier = new hl7.CWE();
    hl7.CWE m_specimenAdditives = new hl7.CWE();
    hl7.CWE m_specimenCollectionMethod = new hl7.CWE();
    hl7.CWE m_specimenSourceSite = new hl7.CWE();
    hl7.CWE m_specimenSourceSiteModifier = new hl7.CWE();
    hl7.CWE m_specimenCollectionSite = new hl7.CWE();
    hl7.CWE m_specimenRole = new hl7.CWE();
    string m_specimenCollectionAmount = string.Empty;
    string m_groupedSpecimenCount = string.Empty;
    string m_specimenDescription = string.Empty;
    hl7.CWE m_specimenHandlingCode = new hl7.CWE();
    hl7.CWE m_specimenRiskCode = new hl7.CWE();
    string m_specimenCollectionDateTime = string.Empty;
    string m_specimenReceivedDateTime = string.Empty;
    string m_specimenExpirationDateTime = string.Empty;
    string m_specimenAvailabiity = string.Empty;
    hl7.CWE m_specimenRejectReason = new hl7.CWE();
    hl7.CWE m_specimenQuality = new hl7.CWE();
    hl7.CWE m_specimenAppropriateness = new hl7.CWE();
    hl7.CWE m_specimenCondition = new hl7.CWE();
    string m_specimenCurrentQuantity = string.Empty;
    string m_numberOfSpecimenContainers = string.Empty;
    hl7.CWE m_containerType = new hl7.CWE();
    hl7.CWE m_containerCondition = new hl7.CWE();
    hl7.CWE m_specimenChildRole = new hl7.CWE();
    string m_accessionId = string.Empty;
    string m_otherSpecimenId = string.Empty;
    hl7.EI m_shipmentId = new hl7.EI();

	public bool Equals(SPM? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_setIdSpm != rhs.m_setIdSpm)
		{
			return false;
		}
		if (m_specimenId != rhs.m_specimenId)
		{
			return false;
		}
		if (m_specimenParentIds != rhs.m_specimenParentIds)
		{
			return false;
		}
		if (!m_specimenType.Equals(rhs.m_specimenType))
		{
			return false;
		}
		if (!m_specimenTypeModifier.Equals(rhs.m_specimenTypeModifier))
		{
			return false;
		}
		if (!m_specimenAdditives.Equals(rhs.m_specimenAdditives))
		{
			return false;
		}
		if (!m_specimenCollectionMethod.Equals(rhs.m_specimenCollectionMethod))
		{
			return false;
		}
		if (!m_specimenSourceSite.Equals(rhs.m_specimenSourceSite))
		{
			return false;
		}
		if (!m_specimenSourceSiteModifier.Equals(rhs.m_specimenSourceSiteModifier))
		{
			return false;
		}
		if (!m_specimenCollectionSite.Equals(rhs.m_specimenCollectionSite))
		{
			return false;
		}
		if (!m_specimenRole.Equals(rhs.m_specimenRole))
		{
			return false;
		}
		if (m_specimenCollectionAmount != rhs.m_specimenCollectionAmount)
		{
			return false;
		}
		if (m_groupedSpecimenCount != rhs.m_groupedSpecimenCount)
		{
			return false;
		}
		if (m_specimenDescription != rhs.m_specimenDescription)
		{
			return false;
		}
		if (!m_specimenHandlingCode.Equals(rhs.m_specimenHandlingCode))
		{
			return false;
		}
		if (!m_specimenRiskCode.Equals(rhs.m_specimenRiskCode))
		{
			return false;
		}
		if (m_specimenCollectionDateTime != rhs.m_specimenCollectionDateTime)
		{
			return false;
		}
		if (m_specimenReceivedDateTime != rhs.m_specimenReceivedDateTime)
		{
			return false;
		}
		if (m_specimenExpirationDateTime != rhs.m_specimenExpirationDateTime)
		{
			return false;
		}
		if (m_specimenAvailabiity != rhs.m_specimenAvailabiity)
		{
			return false;
		}
		if (!m_specimenRejectReason.Equals(rhs.m_specimenRejectReason))
		{
			return false;
		}
		if (!m_specimenQuality.Equals(rhs.m_specimenQuality))
		{
			return false;
		}
		if (!m_specimenAppropriateness.Equals(rhs.m_specimenAppropriateness))
		{
			return false;
		}
		if (!m_specimenCondition.Equals(rhs.m_specimenCondition))
		{
			return false;
		}
		if (m_specimenCurrentQuantity != rhs.m_specimenCurrentQuantity)
		{
			return false;
		}
		if (m_numberOfSpecimenContainers != rhs.m_numberOfSpecimenContainers)
		{
			return false;
		}
		if (!m_containerType.Equals(rhs.m_containerType))
		{
			return false;
		}
		if (!m_containerCondition.Equals(rhs.m_containerCondition))
		{
			return false;
		}
		if (!m_specimenChildRole.Equals(rhs.m_specimenChildRole))
		{
			return false;
		}
		if (m_accessionId != rhs.m_accessionId)
		{
			return false;
		}
		if (m_otherSpecimenId != rhs.m_otherSpecimenId)
		{
			return false;
		}
		if (!m_shipmentId.Equals(rhs.m_shipmentId))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SPM));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SPM));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SPM), m_metaStruct, () => { return new SPM(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class EQU : finalmq.StructBase, IEquatable<EQU>
{
    public EQU()
	{
	}
	

    public EQU(hl7.EI equipmentInstanceIdentifier, string eventDateTime, hl7.CWE equipmentState, hl7.CWE localRemoteControlState, hl7.CWE alertLevel)
	{
		m_equipmentInstanceIdentifier = equipmentInstanceIdentifier;
		m_eventDateTime = eventDateTime;
		m_equipmentState = equipmentState;
		m_localRemoteControlState = localRemoteControlState;
		m_alertLevel = alertLevel;
	}

	[finalmq.MetaField("")]
    public hl7.EI equipmentInstanceIdentifier
	{
		get { return m_equipmentInstanceIdentifier; }
		set { m_equipmentInstanceIdentifier = value; }
	}
	[finalmq.MetaField("")]
    public string eventDateTime
	{
		get { return m_eventDateTime; }
		set { m_eventDateTime = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE equipmentState
	{
		get { return m_equipmentState; }
		set { m_equipmentState = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE localRemoteControlState
	{
		get { return m_localRemoteControlState; }
		set { m_localRemoteControlState = value; }
	}
	[finalmq.MetaField("")]
    public hl7.CWE alertLevel
	{
		get { return m_alertLevel; }
		set { m_alertLevel = value; }
	}

    hl7.EI m_equipmentInstanceIdentifier = new hl7.EI();
    string m_eventDateTime = string.Empty;
    hl7.CWE m_equipmentState = new hl7.CWE();
    hl7.CWE m_localRemoteControlState = new hl7.CWE();
    hl7.CWE m_alertLevel = new hl7.CWE();

	public bool Equals(EQU? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_equipmentInstanceIdentifier.Equals(rhs.m_equipmentInstanceIdentifier))
		{
			return false;
		}
		if (m_eventDateTime != rhs.m_eventDateTime)
		{
			return false;
		}
		if (!m_equipmentState.Equals(rhs.m_equipmentState))
		{
			return false;
		}
		if (!m_localRemoteControlState.Equals(rhs.m_localRemoteControlState))
		{
			return false;
		}
		if (!m_alertLevel.Equals(rhs.m_alertLevel))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EQU));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(EQU));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(EQU), m_metaStruct, () => { return new EQU(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SPM_GROUP : finalmq.StructBase, IEquatable<SPM_GROUP>
{
    public SPM_GROUP()
	{
	}
	

    public SPM_GROUP(hl7.SPM spm, IList<hl7.OBX> obx)
	{
		m_spm = spm;
		m_obx = obx;
	}

	[finalmq.MetaField("")]
    public hl7.SPM spm
	{
		get { return m_spm; }
		set { m_spm = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.OBX> obx
	{
		get { return m_obx; }
		set { m_obx = value; }
	}

    hl7.SPM m_spm = new hl7.SPM();
    IList<hl7.OBX> m_obx = new List<hl7.OBX>();

	public bool Equals(SPM_GROUP? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_spm.Equals(rhs.m_spm))
		{
			return false;
		}
		if (!m_obx.SequenceEqual(rhs.m_obx))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SPM_GROUP));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SPM_GROUP));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SPM_GROUP), m_metaStruct, () => { return new SPM_GROUP(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SAC_GROUP : finalmq.StructBase, IEquatable<SAC_GROUP>
{
    public SAC_GROUP()
	{
	}
	

    public SAC_GROUP(hl7.SAC sac, IList<hl7.OBX> obx, IList<hl7.NTE> nte, IList<hl7.SPM_GROUP> spm)
	{
		m_sac = sac;
		m_obx = obx;
		m_nte = nte;
		m_spm = spm;
	}

	[finalmq.MetaField("")]
    public hl7.SAC sac
	{
		get { return m_sac; }
		set { m_sac = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.OBX> obx
	{
		get { return m_obx; }
		set { m_obx = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.NTE> nte
	{
		get { return m_nte; }
		set { m_nte = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.SPM_GROUP> spm
	{
		get { return m_spm; }
		set { m_spm = value; }
	}

    hl7.SAC m_sac = new hl7.SAC();
    IList<hl7.OBX> m_obx = new List<hl7.OBX>();
    IList<hl7.NTE> m_nte = new List<hl7.NTE>();
    IList<hl7.SPM_GROUP> m_spm = new List<hl7.SPM_GROUP>();

	public bool Equals(SAC_GROUP? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_sac.Equals(rhs.m_sac))
		{
			return false;
		}
		if (!m_obx.SequenceEqual(rhs.m_obx))
		{
			return false;
		}
		if (!m_nte.SequenceEqual(rhs.m_nte))
		{
			return false;
		}
		if (!m_spm.SequenceEqual(rhs.m_spm))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SAC_GROUP));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SAC_GROUP));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SAC_GROUP), m_metaStruct, () => { return new SAC_GROUP(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class ACK : finalmq.StructBase, IEquatable<ACK>
{
    public ACK()
	{
	}
	

    public ACK(hl7.MSH msh, IList<hl7.SFT> sft, hl7.UAC? uac, hl7.MSA msa, IList<hl7.MSH> err)
	{
		m_msh = msh;
		m_sft = sft;
		m_uac = uac;
		m_msa = msa;
		m_err = err;
	}

	[finalmq.MetaField("")]
    public hl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.SFT> sft
	{
		get { return m_sft; }
		set { m_sft = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public hl7.UAC? uac
	{
		get { return m_uac; }
		set { m_uac = value; }
	}
	[finalmq.MetaField("")]
    public hl7.MSA msa
	{
		get { return m_msa; }
		set { m_msa = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.MSH> err
	{
		get { return m_err; }
		set { m_err = value; }
	}

    hl7.MSH m_msh = new hl7.MSH();
    IList<hl7.SFT> m_sft = new List<hl7.SFT>();
    hl7.UAC? m_uac = null;
    hl7.MSA m_msa = new hl7.MSA();
    IList<hl7.MSH> m_err = new List<hl7.MSH>();

	public bool Equals(ACK? rhs)
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
		if (!m_sft.SequenceEqual(rhs.m_sft))
		{
			return false;
		}
		if (!((uac == rhs.uac) || ((uac != null) && (uac != null) && uac.Equals(rhs.uac))))
		{
			return false;
		}
		if (!m_msa.Equals(rhs.m_msa))
		{
			return false;
		}
		if (!m_err.SequenceEqual(rhs.m_err))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ACK));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ACK));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ACK), m_metaStruct, () => { return new ACK(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class SSU_U03 : finalmq.StructBase, IEquatable<SSU_U03>
{
    public SSU_U03()
	{
	}
	

    public SSU_U03(hl7.MSH msh, IList<hl7.SFT> sft, hl7.UAC? uac, hl7.EQU equ, IList<hl7.SAC_GROUP> sac)
	{
		m_msh = msh;
		m_sft = sft;
		m_uac = uac;
		m_equ = equ;
		m_sac = sac;
	}

	[finalmq.MetaField("")]
    public hl7.MSH msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.SFT> sft
	{
		get { return m_sft; }
		set { m_sft = value; }
	}
	[finalmq.MetaField("", finalmq.MetaFieldFlags.METAFLAG_NULLABLE)]
    public hl7.UAC? uac
	{
		get { return m_uac; }
		set { m_uac = value; }
	}
	[finalmq.MetaField("")]
    public hl7.EQU equ
	{
		get { return m_equ; }
		set { m_equ = value; }
	}
	[finalmq.MetaField("")]
    public IList<hl7.SAC_GROUP> sac
	{
		get { return m_sac; }
		set { m_sac = value; }
	}

    hl7.MSH m_msh = new hl7.MSH();
    IList<hl7.SFT> m_sft = new List<hl7.SFT>();
    hl7.UAC? m_uac = null;
    hl7.EQU m_equ = new hl7.EQU();
    IList<hl7.SAC_GROUP> m_sac = new List<hl7.SAC_GROUP>();

	public bool Equals(SSU_U03? rhs)
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
		if (!m_sft.SequenceEqual(rhs.m_sft))
		{
			return false;
		}
		if (!((uac == rhs.uac) || ((uac != null) && (uac != null) && uac.Equals(rhs.uac))))
		{
			return false;
		}
		if (!m_equ.Equals(rhs.m_equ))
		{
			return false;
		}
		if (!m_sac.SequenceEqual(rhs.m_sac))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SSU_U03));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(SSU_U03));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(SSU_U03), m_metaStruct, () => { return new SSU_U03(); } );
    }
}



} // close namespace hl7     

#pragma warning restore IDE1006 // Benennungsstile

