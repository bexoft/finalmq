using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{
    public class Variant
    {
        private readonly int VARTYPE_NONE = 0;

        Variant()
        {
        }

        public Variant(IVariantValue? value)
        {
            m_value = value;
        }

        public static Variant Create<T>(T data)
        {
            IVariantValue? value = VariantValueFactory.Instance.CreateVariantValue(data);
            if (value == null)
            {
                throw new InvalidDataException("Type " + typeof(T) + " not supported");
            }
            return new Variant(value);
        }

        public T GetData<T>()
        {
            if (m_value != null)
            {
                if (m_value.Data.GetType() == typeof(T))
                {
                    return m_value.Data;
                }
            }
            return default(T);
        }

        public void SetData<T>(T data)
        {
            if (data == null)
            {
                m_value = null;
            }
            else if ((m_value != null) && (m_value.Data.GetType() == typeof(T)))
            {
                Debug.Assert(m_value != null);
                m_value.Data = data;
            }
            else
            {
                IVariantValue? value = VariantValueFactory.Instance.CreateVariantValue(data);
                if (value == null)
                {
                    throw new InvalidDataException("Type " + typeof(T) + " not supported");
                }
                m_value = value;
            }
        }

        public Variant GetVariant(string name)
        {
            if (name.Length == 0)
            {
                return this;
            }
            if (m_value == null)
            {
                return null;
            }

            return m_value.GetVariant(name);
        }

        public bool Equals(Variant rhs)
        {
            if (this == rhs)
            {
                return true;
            }

            if (m_value == rhs.m_value)
            {
                return true;
            }

            if (m_value == null || rhs.m_value == null)
            {
                return false;
            }

            return m_value.Equals(rhs.m_value);
        }

        public void Accept(IVariantVisitor visitor, int index, int level, int size, string name)
        {
            if (m_value != null)
            {
                m_value.Accept(visitor, this, index, level, size, name);
            }
            else
            {
               visitor.EnterLeaf(this, VARTYPE_NONE, index, level, size, name);
            }
        }

        public int VarType
        {
            get 
            {
                if (m_value != null)
                {
                    return m_value.VarType;
                }
                else
                {
                    return VARTYPE_NONE;
                }
            }
        }

        public bool Add(string name, Variant variant)
        {   
            if (m_value != null)
            {
                return m_value.Add(name, variant);
            }
            return false;
        }

        public bool Add(Variant variant)
        {
            if (m_value != null)
            {
                return m_value.Add(variant);
            }
            return false;
        }

        public int Length
        {
            get
            {
                if (m_value != null)
                {
                    return m_value.Length;
                }
                return 0;
            }
        }

        Variant Clone()
        {
            if (m_value == null)
            {
                return new Variant(null);
            }

            return new Variant(m_value.Clone());
        }


        IVariantValue? m_value = null;
    }
}
