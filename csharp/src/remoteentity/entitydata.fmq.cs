
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace finalmq {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("")]
public enum MsgMode
{
	[finalmq.MetaEnumEntry("The message is a request", "")]
    MSG_REQUEST = 0,
	[finalmq.MetaEnumEntry("The message is a reply", "")]
    MSG_REPLY = 1,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED2 = 2,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED3 = 3,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED4 = 4,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED5 = 5,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED6 = 6,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED7 = 7,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED8 = 8,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED9 = 9,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED10 = 10,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED11 = 11,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED12 = 12,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED13 = 13,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED14 = 14,
	[finalmq.MetaEnumEntry("", "")]
    MSG_RESERVED15 = 15,
}
class RegisterMsgMode
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(MsgMode));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(MsgMode), metaEnum);
    }
}

[finalmq.MetaEnum("")]
public enum Status
{
	[finalmq.MetaEnumEntry("OK", "")]
    STATUS_OK = 0,
	[finalmq.MetaEnumEntry("Peer endity not found", "")]
    STATUS_ENTITY_NOT_FOUND = 1,
	[finalmq.MetaEnumEntry("Not expected reply type", "")]
    STATUS_WRONG_REPLY_TYPE = 2,
	[finalmq.MetaEnumEntry("Syntax error at parsing the message", "")]
    STATUS_SYNTAX_ERROR = 3,
	[finalmq.MetaEnumEntry("The request/command is not registered at the request executor (server)", "")]
    STATUS_REQUEST_NOT_FOUND = 4,
	[finalmq.MetaEnumEntry("The request type is not known", "")]
    STATUS_REQUESTTYPE_NOT_KNOWN = 5,
	[finalmq.MetaEnumEntry("the reply type is not known", "")]
    STATUS_REPLYTYPE_NOT_KNOWN = 6,
	[finalmq.MetaEnumEntry("the (socket) session is disconnected", "")]
    STATUS_SESSION_DISCONNECTED = 7,
	[finalmq.MetaEnumEntry("The peer is disconnected", "")]
    STATUS_PEER_DISCONNECTED = 8,
	[finalmq.MetaEnumEntry("No reply was sent by the request executor (server)", "")]
    STATUS_NO_REPLY = 9,
	[finalmq.MetaEnumEntry("Wrong content type", "")]
    STATUS_WRONG_CONTENTTYPE = 10,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED11 = 11,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED12 = 12,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED13 = 13,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED14 = 14,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED15 = 15,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED16 = 16,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED17 = 17,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED18 = 18,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED19 = 19,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED20 = 20,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED21 = 21,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED22 = 22,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED23 = 23,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED24 = 24,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED25 = 25,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED26 = 26,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED27 = 27,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED28 = 28,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED29 = 29,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED30 = 30,
	[finalmq.MetaEnumEntry("", "")]
    STATUS_RESERVED31 = 31,
}
class RegisterStatus
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(Status));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(Status), metaEnum);
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("")]
public class Header : finalmq.StructBase, IEquatable<Header>
{
    public Header()
	{
	}
	

    public Header(ulong destid, string destname, ulong srcid, finalmq.MsgMode mode, finalmq.Status status, string path, string type, ulong corrid, IList<string> meta)
	{
		m_destid = destid;
		m_destname = destname;
		m_srcid = srcid;
		m_mode = mode;
		m_status = status;
		m_path = path;
		m_type = type;
		m_corrid = corrid;
		m_meta = meta;
	}

	[finalmq.MetaField("ID of destination/receiver endity", finalmq.MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public ulong destid
	{
		get { return m_destid; }
		set { m_destid = value; }
	}
	[finalmq.MetaField("Name of destination/receiver endity")]
    public string destname
	{
		get { return m_destname; }
		set { m_destname = value; }
	}
	[finalmq.MetaField("ID of source/sender endity", finalmq.MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public ulong srcid
	{
		get { return m_srcid; }
		set { m_srcid = value; }
	}
	[finalmq.MetaField("mode: request or reply")]
    public finalmq.MsgMode mode
	{
		get { return m_mode; }
		set { m_mode = value; }
	}
	[finalmq.MetaField("Reply status")]
    public finalmq.Status status
	{
		get { return m_status; }
		set { m_status = value; }
	}
	[finalmq.MetaField("path in the context of the entity, if empty than type is also the path")]
    public string path
	{
		get { return m_path; }
		set { m_path = value; }
	}
	[finalmq.MetaField("Message type in payload")]
    public string type
	{
		get { return m_type; }
		set { m_type = value; }
	}
	[finalmq.MetaField("It is set by the sender of the request. The receiver of the request will reply with the same correlation ID", finalmq.MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public ulong corrid
	{
		get { return m_corrid; }
		set { m_corrid = value; }
	}
	[finalmq.MetaField("Additional data for the message")]
    public IList<string> meta
	{
		get { return m_meta; }
		set { m_meta = value; }
	}

    ulong m_destid = 0;
    string m_destname = string.Empty;
    ulong m_srcid = 0;
    finalmq.MsgMode m_mode = new finalmq.MsgMode();
    finalmq.Status m_status = new finalmq.Status();
    string m_path = string.Empty;
    string m_type = string.Empty;
    ulong m_corrid = 0;
    IList<string> m_meta = new List<string>();

	public bool Equals(Header? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_destid != rhs.m_destid)
		{
			return false;
		}
		if (m_destname != rhs.m_destname)
		{
			return false;
		}
		if (m_srcid != rhs.m_srcid)
		{
			return false;
		}
		if (m_mode != rhs.m_mode)
		{
			return false;
		}
		if (m_status != rhs.m_status)
		{
			return false;
		}
		if (m_path != rhs.m_path)
		{
			return false;
		}
		if (m_type != rhs.m_type)
		{
			return false;
		}
		if (m_corrid != rhs.m_corrid)
		{
			return false;
		}
		if (!m_meta.SequenceEqual(rhs.m_meta))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Header));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Header));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(Header), m_metaStruct, () => { return new Header(); } );
    }
}

[finalmq.MetaStruct("Contains only plain message data")]
public class RawDataMessage : finalmq.StructBase, IEquatable<RawDataMessage>
{
    public RawDataMessage()
	{
	}
	



	public bool Equals(RawDataMessage? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}


		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(RawDataMessage));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(RawDataMessage));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(RawDataMessage), m_metaStruct, () => { return new RawDataMessage(); } );
    }
}

[finalmq.MetaStruct("If an entity connects to a remote entity it shall send ConnectEntity")]
public class ConnectEntity : finalmq.StructBase, IEquatable<ConnectEntity>
{
    public ConnectEntity()
	{
	}
	

    public ConnectEntity(string entityName)
	{
		m_entityName = entityName;
	}

	[finalmq.MetaField("The entity name of the sender of ConnectEntity request")]
    public string entityName
	{
		get { return m_entityName; }
		set { m_entityName = value; }
	}

    string m_entityName = string.Empty;

	public bool Equals(ConnectEntity? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_entityName != rhs.m_entityName)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ConnectEntity));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ConnectEntity));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ConnectEntity), m_metaStruct, () => { return new ConnectEntity(); } );
    }
}

[finalmq.MetaStruct("If an entity receives a ConnectEntity request it shall reply with ConnectEntityReply")]
public class ConnectEntityReply : finalmq.StructBase, IEquatable<ConnectEntityReply>
{
    public ConnectEntityReply()
	{
	}
	

    public ConnectEntityReply(ulong entityId, string entityName)
	{
		m_entityId = entityId;
		m_entityName = entityName;
	}

	[finalmq.MetaField("The entity ID of the sender of ConnectivityReply", finalmq.MetaFieldFlags.METAFLAG_PROTO_VARINT)]
    public ulong entityId
	{
		get { return m_entityId; }
		set { m_entityId = value; }
	}
	[finalmq.MetaField("The entity name of the sender of ConnectivityReply")]
    public string entityName
	{
		get { return m_entityName; }
		set { m_entityName = value; }
	}

    ulong m_entityId = 0;
    string m_entityName = string.Empty;

	public bool Equals(ConnectEntityReply? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_entityId != rhs.m_entityId)
		{
			return false;
		}
		if (m_entityName != rhs.m_entityName)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ConnectEntityReply));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(ConnectEntityReply));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(ConnectEntityReply), m_metaStruct, () => { return new ConnectEntityReply(); } );
    }
}

[finalmq.MetaStruct("The entity that will be removed will send DisconnectEntity to all its peers.")]
public class DisconnectEntity : finalmq.StructBase, IEquatable<DisconnectEntity>
{
    public DisconnectEntity()
	{
	}
	



	public bool Equals(DisconnectEntity? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}


		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(DisconnectEntity));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(DisconnectEntity));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(DisconnectEntity), m_metaStruct, () => { return new DisconnectEntity(); } );
    }
}

[finalmq.MetaStruct("Send pure data for protocols with no meta info.")]
public class RawBytes : finalmq.StructBase, IEquatable<RawBytes>
{
    public RawBytes()
	{
	}
	

    public RawBytes(byte[] data)
	{
		m_data = data;
	}

	[finalmq.MetaField("The data")]
    public byte[] data
	{
		get { return m_data; }
		set { m_data = value; }
	}

    byte[] m_data = Array.Empty<byte>();

	public bool Equals(RawBytes? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_data.SequenceEqual(rhs.m_data))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(RawBytes));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(RawBytes));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(RawBytes), m_metaStruct, () => { return new RawBytes(); } );
    }
}

[finalmq.MetaStruct("Use this type for the request if you register a command with a path and the rquest does not have any data")]
public class NoData : finalmq.StructBase, IEquatable<NoData>
{
    public NoData()
	{
	}
	



	public bool Equals(NoData? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}


		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(NoData));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(NoData));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(NoData), m_metaStruct, () => { return new NoData(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class HD_RE : finalmq.StructBase, IEquatable<HD_RE>
{
    public HD_RE()
	{
	}
	

    public HD_RE(string namespaceId, string universalId, string universalIdType)
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

	public bool Equals(HD_RE? rhs)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HD_RE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HD_RE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(HD_RE), m_metaStruct, () => { return new HD_RE(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class MSG_RE : finalmq.StructBase, IEquatable<MSG_RE>
{
    public MSG_RE()
	{
	}
	

    public MSG_RE(string messageCode, string triggerEvent, string messageStructure)
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

	public bool Equals(MSG_RE? rhs)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_RE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSG_RE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSG_RE), m_metaStruct, () => { return new MSG_RE(); } );
    }
}

[finalmq.MetaStruct("desc", finalmq.MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT)]
public class MSH_RE : finalmq.StructBase, IEquatable<MSH_RE>
{
    public MSH_RE()
	{
	}
	

    public MSH_RE(string fieldSeparator, string encodingCharacters, finalmq.HD_RE sendingApplication, finalmq.HD_RE sendingFacility, finalmq.HD_RE receivingApplication, finalmq.HD_RE receivingFacility, string dateTimeOfMessage, string security, finalmq.MSG_RE messageType)
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
    public finalmq.HD_RE sendingApplication
	{
		get { return m_sendingApplication; }
		set { m_sendingApplication = value; }
	}
	[finalmq.MetaField("")]
    public finalmq.HD_RE sendingFacility
	{
		get { return m_sendingFacility; }
		set { m_sendingFacility = value; }
	}
	[finalmq.MetaField("")]
    public finalmq.HD_RE receivingApplication
	{
		get { return m_receivingApplication; }
		set { m_receivingApplication = value; }
	}
	[finalmq.MetaField("")]
    public finalmq.HD_RE receivingFacility
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
    public finalmq.MSG_RE messageType
	{
		get { return m_messageType; }
		set { m_messageType = value; }
	}

    string m_fieldSeparator = string.Empty;
    string m_encodingCharacters = string.Empty;
    finalmq.HD_RE m_sendingApplication = new finalmq.HD_RE();
    finalmq.HD_RE m_sendingFacility = new finalmq.HD_RE();
    finalmq.HD_RE m_receivingApplication = new finalmq.HD_RE();
    finalmq.HD_RE m_receivingFacility = new finalmq.HD_RE();
    string m_dateTimeOfMessage = string.Empty;
    string m_security = string.Empty;
    finalmq.MSG_RE m_messageType = new finalmq.MSG_RE();

	public bool Equals(MSH_RE? rhs)
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

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSH_RE));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(MSH_RE));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(MSH_RE), m_metaStruct, () => { return new MSH_RE(); } );
    }
}

[finalmq.MetaStruct("desc")]
public class Hl7Header : finalmq.StructBase, IEquatable<Hl7Header>
{
    public Hl7Header()
	{
	}
	

    public Hl7Header(finalmq.MSH_RE msh)
	{
		m_msh = msh;
	}

	[finalmq.MetaField("")]
    public finalmq.MSH_RE msh
	{
		get { return m_msh; }
		set { m_msh = value; }
	}

    finalmq.MSH_RE m_msh = new finalmq.MSH_RE();

	public bool Equals(Hl7Header? rhs)
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

		return true;
	}

	public override finalmq.MetaStruct MetaStruct
	{
		get
		{
			if (m_metaStruct == null)
			{
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Hl7Header));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Hl7Header));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(Hl7Header), m_metaStruct, () => { return new Hl7Header(); } );
    }
}



} // close namespace finalmq     

#pragma warning restore IDE1006 // Benennungsstile

