using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace finalmq
{
    using VariantList = IList<Variant>;

    public class VariantValueList : IVariantValue
    {
        private readonly int VARTYPE_LIST = (int)MetaTypeId.TYPE_ARRAY_STRUCT;

        public VariantValueList(VariantList value)
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
            int index = 0;
            if (Int32.TryParse(name, out index))
            {
                if (index >= 0 && index < m_value.Count)
                {
                    return m_value[index];
                }
            }
            return null;
        }


        public Variant? GetVariant(string name)
        {
            if (name.Length == 0)
            {
                return null;
            }

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
        public bool Equals(IVariantValue rhs)
        {
            if (this == rhs)
            {
                return true;
            }

            if (VarType != rhs.VarType)
            {
                return false;
            }

            return m_value.Equals(rhs.Data);
        }
        public bool Add(string name, Variant variant)
        {
            return false;
        }
        public bool Add(Variant variant)
        {
            m_value.Add(variant);
            return true;
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
            visitor.EnterList(variant, VARTYPE_LIST, index, level, size, name);
            ++level;
            int i = 0;
            int subsize = m_value.Count;
            foreach (var subVariant in m_value)
            {
                subVariant.Accept(visitor, i, level, subsize, "");
                i++;
            }
            --level;
            visitor.ExitList(variant, VARTYPE_LIST, index, level, size, name);
        }

        public int VarType { get { return VARTYPE_LIST; } }
        public IVariantValue Clone() { return new VariantValueList(Data.Clone()); }

        private VariantList m_value;
    }

    class RegisterVariantList
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register()
        {
            VariantValueFactory.Instance.Register<VariantList>((dynamic data) => { return new VariantValueList(data); });
        }
    }
}
