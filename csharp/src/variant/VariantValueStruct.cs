using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace finalmq
{
    using VariantStruct = IList<NameValue>;

    public class NameValue : IEquatable<NameValue>
    {
        public NameValue(string name, Variant value)
        {
            m_name = name;
            m_value = value;
        }
        public string Name
        {
            get { return m_name; }
        }
        public Variant Value
        { 
            get { return m_value; } 
            set { m_value = value; }
        }

        public bool Equals(NameValue? rhs)
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

            return m_value.Equals(rhs.m_value);
        }

        readonly string m_name;
        Variant m_value;
    }

    public class VariantValueStruct : IVariantValue
    {
        public static readonly int VARTYPE_STRUCT = (int)MetaTypeId.TYPE_STRUCT;

        public VariantValueStruct()
        {
            m_value = new List<NameValue>();
        }
        public VariantValueStruct(VariantStruct value)
        {
            m_value = value;
        }
        public dynamic Data
        {
            get
            {
                return m_value;
            }
            set
            {
                if (m_value.GetType() != value.GetType())
                {
                    throw new ArgumentException("Type mismatch: old type: " + m_value.GetType().ToString() + ", new type: " + value.GetType());
                }
                m_value = value;
            }
        }

        private Variant? Find(string name)
        {
            foreach (var entry in m_value)
            {
                if (entry.Name == name)
                {
                    return entry.Value;
                }
            }
            return null;
        }


        public Variant? GetVariant(string name)
        {
            string partname = "";
            string restname;

            //sperate first key ansd second key
            int cntp = name.IndexOf('.');
            if (cntp != -1)
            {
                partname = name.Substring(0, cntp);
                cntp++;
                restname = name.Substring(cntp);
            }
            else
            {
                partname = name;
                restname = "";
            }

            // remove "", if available in partname
            if ((partname.Length >= 2) && (partname[0] == '\"') && (partname.Last() == '\"'))
            {
                partname = partname.Substring(1, partname.Length - 2);
            }

            Debug.Assert(partname != null);
            // check if next key is in map (if not -> null)
            Variant? variant = Find(partname);
            if (variant == null)
            {
                return null;
            }

            // m_value[name].getValue( with remaining name )
            return variant.GetVariant(restname);
        }
        public bool Equals(IVariantValue? rhs)
        {
            if (rhs == null)
            {
                return false;
            }

            if (this == rhs)
            {
                return true;
            }

            if (VarType != rhs.VarType)
            {
                return false;
            }

            VariantStruct? rhsData = rhs.Data as VariantStruct;
            if (rhsData == null)
            {
                return false;
            }

            return m_value.SequenceEqual(rhsData);
        }
        public bool Add(string name, Variant variant)
        {
            Variant? entry = Find(name);
            if (entry == null)
            {
                m_value.Add(new NameValue(name, variant));
                return true;
            }
            return false;
        }
        public bool Add(Variant variant)
        {
            return false;
        }
        public int Length
        {
            get
            {
                return m_value.Count;
            }
        }

        public void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name)
        {
            visitor.EnterList(variant, VARTYPE_STRUCT, index, level, size, name);
            ++level;
            int i = 0;
            int subsize = m_value.Count;
            foreach (NameValue entry in m_value)
            {
                entry.Value.Accept(visitor, i, level, subsize, entry.Name);
                i++;
            }
            --level;
            visitor.ExitList(variant, VARTYPE_STRUCT, index, level, size, name);
        }

        public int VarType { get { return VARTYPE_STRUCT; } }
        public IVariantValue Clone() { return new VariantValueList(Data.Clone()); }

        private VariantStruct m_value;
    }

    class RegisterVariantStruct
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register() => VariantValueFactory.Instance.Register<VariantStruct>((dynamic data) => { return new VariantValueStruct(data); });
    }
}
