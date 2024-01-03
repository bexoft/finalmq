
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace timer {     


//////////////////////////////
// Enums
//////////////////////////////





//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("Call StartRequest to start the timer event.", finalmq.MetaStructFlags.METASTRUCTFLAG_NONE, new string[] {})]
public class StartRequest : finalmq.StructBase, IEquatable<StartRequest>
{
    public StartRequest()
	{
	}
	



	public bool Equals(StartRequest? rhs)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(StartRequest));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(StartRequest));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(StartRequest), m_metaStruct, () => { return new StartRequest(); } );
    }
}

[finalmq.MetaStruct("Call StopRequest to stop the timer event.", finalmq.MetaStructFlags.METASTRUCTFLAG_NONE, new string[] {})]
public class StopRequest : finalmq.StructBase, IEquatable<StopRequest>
{
    public StopRequest()
	{
	}
	



	public bool Equals(StopRequest? rhs)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(StopRequest));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(StopRequest));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(StopRequest), m_metaStruct, () => { return new StopRequest(); } );
    }
}

[finalmq.MetaStruct("The server triggers this event to all connected clients.", finalmq.MetaStructFlags.METASTRUCTFLAG_NONE, new string[] {})]
public class TimerEvent : finalmq.StructBase, IEquatable<TimerEvent>
{
    public TimerEvent()
	{
	}
	

    public TimerEvent(string time)
	{
		m_time = time;
	}

	[finalmq.MetaField("The current time.", finalmq.MetaFieldFlags.METAFLAG_NONE, new string[] {})]
    public string time
	{
		get { return m_time; }
		set { m_time = value; }
	}

    string m_time = string.Empty;

	public bool Equals(TimerEvent? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_time != rhs.m_time)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TimerEvent));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(TimerEvent));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(TimerEvent), m_metaStruct, () => { return new TimerEvent(); } );
    }
}



} // close namespace timer     

#pragma warning restore IDE1006 // Benennungsstile

