using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;

namespace finalmq
{
    using VariantList = List<Variant>;
    using VariantStruct = List<NameValue>;

    public class Variant
    {
        private readonly int VARTYPE_NONE = 0;

        public Variant()
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
                else
                {
                    return Convertion.Convert<T>(m_value.Data);
                }
            }
            return Default<T>();
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

        public T GetData<T>(string name)
        {
            Variant? v = GetVariant(name);
            if (v != null)
            {
                return v.GetData<T>();
            }
            return Default<T>();
        }

        public void SetData<T>(string name, T data)
        {
            Variant? v = GetVariant(name);
            if (v != null)
            {
                v.SetData<T>(data);
            }
        }
        public Variant? GetVariant(string name)
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

        public bool Add<T>(string name, T data)
        {
            if (m_value != null)
            {
                return m_value.Add(name, Create(data));
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
        public bool Add<T>(T data)
        {
            if (m_value != null)
            {
                return m_value.Add(Create(data));
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

        private T Default<T>()
        {
            T? v = default(T);
            if (v == null)
            {
                if (typeof(T) == typeof(string))
                {
                    return (dynamic)EmptyString;
                }
                if (typeof(T) == typeof(byte[]))
                {
                    return (dynamic)EmptyBytes;
                }
                if (typeof(T) == typeof(bool[]))
                {
                    return (dynamic)EmptyArrayBool;
                }
                if (typeof(T) == typeof(int[]))
                {
                    return (dynamic)EmptyArrayInt32;
                }
                if (typeof(T) == typeof(uint[]))
                {
                    return (dynamic)EmptyArrayUInt32;
                }
                if (typeof(T) == typeof(long[]))
                {
                    return (dynamic)EmptyArrayInt64;
                }
                if (typeof(T) == typeof(ulong[]))
                {
                    return (dynamic)EmptyArrayUInt64;
                }
                if (typeof(T) == typeof(float[]))
                {
                    return (dynamic)EmptyArrayFloat;
                }
                if (typeof(T) == typeof(double[]))
                {
                    return (dynamic)EmptyArrayDouble;
                }
                if (typeof(T) == typeof(IList<string>))
                {
                    return (dynamic)EmptyListString;
                }
                if (typeof(T) == typeof(IList<byte[]>))
                {
                    return (dynamic)EmptyListBytes;
                }
                if (typeof(T) == typeof(VariantList))
                {
                    return (dynamic)new VariantList();
                }
                if (typeof(T) == typeof(VariantStruct))
                {
                    return (dynamic)new VariantStruct();
                }
                throw new InvalidOperationException("Type " + typeof(T).Name + " not supported");
            }
            return v;
        }

        public static implicit operator bool(Variant v) { return v.GetData<bool>(); }
        public static implicit operator int(Variant v) { return v.GetData<int>(); }
        public static implicit operator uint(Variant v) { return v.GetData<uint>(); }
        public static implicit operator long(Variant v) { return v.GetData<long>(); }
        public static implicit operator ulong(Variant v) { return v.GetData<ulong>(); }
        public static implicit operator float(Variant v) { return v.GetData<float>(); }
        public static implicit operator double(Variant v) { return v.GetData<double>(); }
        public static implicit operator string(Variant v) { return v.GetData<string>(); }
        public static implicit operator byte[](Variant v) { return v.GetData<byte[]>(); }
        public static implicit operator bool[](Variant v) { return v.GetData<bool[]>(); }
        public static implicit operator int[](Variant v) { return v.GetData<int[]>(); }
        public static implicit operator uint[](Variant v) { return v.GetData<uint[]>(); }
        public static implicit operator long[](Variant v) { return v.GetData<long[]>(); }
        public static implicit operator ulong[](Variant v) { return v.GetData<ulong[]>(); }
        public static implicit operator float[](Variant v) { return v.GetData<float[]>(); }
        public static implicit operator double[](Variant v) { return v.GetData<double[]>(); }
        public static implicit operator List<string>(Variant v) 
        {
            List<string>? list = null;
            IList<string>? ilist = v.GetData<IList<string>>();
            list = ilist as List<string>;
            if (list == null)
            {
                list = new List<string>(ilist);
            }
            return list;
        }
        public static implicit operator List<byte[]>(Variant v)
        {
            List<byte[]>? list = null;
            IList<byte[]>? ilist = v.GetData<IList<byte[]>>();
            list = ilist as List<byte[]>;
            if (list == null)
            {
                list = new List<byte[]>(ilist);
            }
            return list;
        }

        IVariantValue? m_value = null;

        static readonly string EmptyString = "";
        static readonly byte[] EmptyBytes = new byte[0];
        static readonly bool[] EmptyArrayBool = new bool[0];
        static readonly int[] EmptyArrayInt32 = new int[0];
        static readonly uint[] EmptyArrayUInt32 = new uint[0];
        static readonly long[] EmptyArrayInt64 = new long[0];
        static readonly ulong[] EmptyArrayUInt64 = new ulong[0];
        static readonly float[] EmptyArrayFloat = new float[0];
        static readonly double[] EmptyArrayDouble = new double[0];
        static readonly IList<string> EmptyListString = new List<string>();
        static readonly IList<byte[]> EmptyListBytes = new List<byte[]>();
    }
}
