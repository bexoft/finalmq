
using System.Runtime.CompilerServices;


#pragma warning disable IDE1006 // Benennungsstile


namespace helloworld {     


//////////////////////////////
// Enums
//////////////////////////////


[finalmq.MetaEnum("The gender of a person")]
public enum Gender
{
	[finalmq.MetaEnumEntry("invalid gender", "invalid")]
    INVALID = 0,
	[finalmq.MetaEnumEntry("male", "male")]
    MALE = 1,
	[finalmq.MetaEnumEntry("female", "female")]
    FEMALE = 2,
	[finalmq.MetaEnumEntry("diverse", "diverse")]
    DIVERSE = 3,
}
class RegisterGender
{
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
    internal static void RegisterEnum()
    {
		finalmq.MetaEnum metaEnum = finalmq.StructBase.CreateMetaEnum(typeof(Gender));
        finalmq.TypeRegistry.Instance.RegisterEnum(typeof(Gender), metaEnum);
    }
}




//////////////////////////////
// Structs
//////////////////////////////



[finalmq.MetaStruct("An address")]
public class Address : finalmq.StructBase, IEquatable<Address>
{
    public Address()
	{
	}
	

    public Address(string street, uint number, uint postalcode, string city, string country)
	{
		m_street = street;
		m_number = number;
		m_postalcode = postalcode;
		m_city = city;
		m_country = country;
	}

	[finalmq.MetaField("The first name of a person.")]
    public string street
	{
		get { return m_street; }
		set { m_street = value; }
	}
	[finalmq.MetaField("The house number")]
    public uint number
	{
		get { return m_number; }
		set { m_number = value; }
	}
	[finalmq.MetaField("The postal code.")]
    public uint postalcode
	{
		get { return m_postalcode; }
		set { m_postalcode = value; }
	}
	[finalmq.MetaField("The city name.")]
    public string city
	{
		get { return m_city; }
		set { m_city = value; }
	}
	[finalmq.MetaField("The country name.")]
    public string country
	{
		get { return m_country; }
		set { m_country = value; }
	}

    string m_street = string.Empty;
    uint m_number = 0;
    uint m_postalcode = 0;
    string m_city = string.Empty;
    string m_country = string.Empty;

	public bool Equals(Address? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (m_street != rhs.m_street)
		{
			return false;
		}
		if (m_number != rhs.m_number)
		{
			return false;
		}
		if (m_postalcode != rhs.m_postalcode)
		{
			return false;
		}
		if (m_city != rhs.m_city)
		{
			return false;
		}
		if (m_country != rhs.m_country)
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Address));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Address));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(Address), m_metaStruct, () => { return new Address(); } );
    }
}

[finalmq.MetaStruct("Models a person")]
public class Person : finalmq.StructBase, IEquatable<Person>
{
    public Person()
	{
	}
	

    public Person(string name, string surname, helloworld.Gender gender, uint yearOfBirth, helloworld.Address address)
	{
		m_name = name;
		m_surname = surname;
		m_gender = gender;
		m_yearOfBirth = yearOfBirth;
		m_address = address;
	}

	[finalmq.MetaField("The first name of a person.")]
    public string name
	{
		get { return m_name; }
		set { m_name = value; }
	}
	[finalmq.MetaField("The last name of a person.")]
    public string surname
	{
		get { return m_surname; }
		set { m_surname = value; }
	}
	[finalmq.MetaField("The gender of a person.")]
    public helloworld.Gender gender
	{
		get { return m_gender; }
		set { m_gender = value; }
	}
	[finalmq.MetaField("Year of birth.")]
    public uint yearOfBirth
	{
		get { return m_yearOfBirth; }
		set { m_yearOfBirth = value; }
	}
	[finalmq.MetaField("The address of a person.")]
    public helloworld.Address address
	{
		get { return m_address; }
		set { m_address = value; }
	}

    string m_name = string.Empty;
    string m_surname = string.Empty;
    helloworld.Gender m_gender = new helloworld.Gender();
    uint m_yearOfBirth = 0;
    helloworld.Address m_address = new helloworld.Address();

	public bool Equals(Person? rhs)
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
		if (m_surname != rhs.m_surname)
		{
			return false;
		}
		if (m_gender != rhs.m_gender)
		{
			return false;
		}
		if (m_yearOfBirth != rhs.m_yearOfBirth)
		{
			return false;
		}
		if (!m_address.Equals(rhs.m_address))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Person));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(Person));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(Person), m_metaStruct, () => { return new Person(); } );
    }
}

[finalmq.MetaStruct("")]
public class HelloRequest : finalmq.StructBase, IEquatable<HelloRequest>
{
    public HelloRequest()
	{
	}
	

    public HelloRequest(IList<helloworld.Person> persons)
	{
		m_persons = persons;
	}

	[finalmq.MetaField("Parameter of HelloRequest")]
    public IList<helloworld.Person> persons
	{
		get { return m_persons; }
		set { m_persons = value; }
	}

    IList<helloworld.Person> m_persons = new List<helloworld.Person>();

	public bool Equals(HelloRequest? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_persons.SequenceEqual(rhs.m_persons))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HelloRequest));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HelloRequest));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(HelloRequest), m_metaStruct, () => { return new HelloRequest(); } );
    }
}

[finalmq.MetaStruct("")]
public class HelloReply : finalmq.StructBase, IEquatable<HelloReply>
{
    public HelloReply()
	{
	}
	

    public HelloReply(IList<string> greetings)
	{
		m_greetings = greetings;
	}

	[finalmq.MetaField("The greetings for all persons.")]
    public IList<string> greetings
	{
		get { return m_greetings; }
		set { m_greetings = value; }
	}

    IList<string> m_greetings = new List<string>();

	public bool Equals(HelloReply? rhs)
	{
		if (rhs == null)
		{
			return false;
		}

		if (this == rhs)
		{
			return true;
		}

		if (!m_greetings.SequenceEqual(rhs.m_greetings))
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
				m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HelloReply));
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
		m_metaStruct = finalmq.StructBase.CreateMetaStruct(typeof(HelloReply));
        finalmq.TypeRegistry.Instance.RegisterStruct(typeof(HelloReply), m_metaStruct, () => { return new HelloReply(); } );
    }
}



} // close namespace helloworld     

#pragma warning restore IDE1006 // Benennungsstile

