using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;

namespace finalmq
{
    using VariantList = List<Variant>;
    using VariantStruct = List<NameValue>;

    public class Variant : IEquatable<Variant>
    {
        public static readonly int VARTYPE_NONE = 0;

        public Variant()
        {
        }

        public Variant(IVariantValue? value)
        {
            m_value = value;
        }

        public static Variant Create<T>(T data) where T : notnull
        {
            IVariantValue? value = VariantValueFactory.Instance.CreateVariantValue(data);
            if (value == null)
            {
                throw new InvalidDataException("Type " + typeof(T) + " not supported");
            }
            return new Variant(value);
        }

        public T GetData<T>() where T : notnull
        {
            if (m_value != null)
            {
                if (m_value.Data.GetType() == typeof(T))
                {
                    return m_value.Data;
                }
                else
                {
                    T? data = Convertion.Convert<T>(m_value.Data);
                    if (data != null)
                    {
                        return data;
                    }
                }
            }
            return Default<T>();
        }

        public void SetData<T>(T data) where T : notnull
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

        public T GetData<T>(string name) where T : notnull
        {
            Variant? v = GetVariant(name);
            if (v != null)
            {
                return v.GetData<T>();
            }
            return Default<T>();
        }

        public void SetData<T>(string name, T data) where T : notnull
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

        public bool Equals(Variant? rhs)
        {
            if (rhs == null)
            {
                return false;
            }

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

        public void Accept(IVariantVisitor visitor, int index = 0, int level = 0, int size = 0, string name = "")
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

        public bool Add<T>(string name, T data) where T : notnull
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
        public bool Add<T>(T data) where T : notnull
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

        public Variant Clone()
        {
            if (m_value == null)
            {
                return new Variant(null);
            }

            return new Variant(m_value.Clone());
        }

        public Variant GetOrCreate(string name)
        {
            if (name.Length == 0)
            {
                return this;
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

            // check if number
            if (partname.Length != 0 && partname[0] >= '0' && partname[0] <= '9')
            {
                int index = 0;
                try
                {
                    index = Int32.Parse(partname);
                }
                catch (Exception)
                {
                }
                Debug.Assert(index >= 0);
                int sizeNew = index + 1;
                Debug.Assert(sizeNew > 0);
                if (m_value == null || m_value.VarType != VariantValueList.VARTYPE_LIST)
                {
                    m_value = new VariantValueList();
                }
                while (m_value.Length < sizeNew)
                {
                    m_value.Add(new Variant());
                }
                Variant? varSub = m_value.GetVariant(partname);
                Debug.Assert(varSub != null);
                return varSub.GetOrCreate(restname);
            }
            else
            {
                // remove "", if available in partname
                if ((partname.Length >= 2) && (partname[0] == '\"') && (partname.Last() == '\"'))
                {
                    partname = partname.Substring(1, partname.Length - 2);
                }
                if (m_value == null|| m_value.VarType != VariantValueStruct.VARTYPE_STRUCT)
                {
                    m_value = new VariantValueStruct();
                }
                Variant? varSub = m_value.GetVariant(partname);
                if (varSub == null)
                {
                    m_value.Add(partname, new Variant());
                    varSub = m_value.GetVariant(partname);
                }
                Debug.Assert(varSub != null);
                return varSub.GetOrCreate(restname);
            }
        }

        private T Default<T>()
        {
            T? v = default(T);
            if (v == null)
            {
                if (typeof(T) == typeof(string))
                {
                    return (dynamic)string.Empty;
                }
                if (typeof(T) == typeof(byte[]))
                {
                    return (dynamic)Array.Empty<byte>();
                }
                if (typeof(T) == typeof(bool[]))
                {
                    return (dynamic)Array.Empty<bool>();
                }
                if (typeof(T) == typeof(sbyte[]))
                {
                    return (dynamic)Array.Empty<sbyte>();
                }
                if (typeof(T) == typeof(byte[]))
                {
                    return (dynamic)Array.Empty<byte>();
                }
                if (typeof(T) == typeof(short[]))
                {
                    return (dynamic)Array.Empty<short>();
                }
                if (typeof(T) == typeof(ushort[]))
                {
                    return (dynamic)Array.Empty<ushort>();
                }
                if (typeof(T) == typeof(int[]))
                {
                    return (dynamic)Array.Empty<int>();
                }
                if (typeof(T) == typeof(uint[]))
                {
                    return (dynamic)Array.Empty<uint>();
                }
                if (typeof(T) == typeof(long[]))
                {
                    return (dynamic)Array.Empty<long>();
                }
                if (typeof(T) == typeof(ulong[]))
                {
                    return (dynamic)Array.Empty<ulong>();
                }
                if (typeof(T) == typeof(float[]))
                {
                    return (dynamic)Array.Empty<float>();
                }
                if (typeof(T) == typeof(double[]))
                {
                    return (dynamic)Array.Empty<double>();
                }
                if (typeof(T) == typeof(IList<string>))
                {
                    return (dynamic)Array.Empty<string>();
                }
                if (typeof(T) == typeof(IList<byte[]>))
                {
                    return (dynamic)Array.Empty<byte[]>();
                }
                if (typeof(T) == typeof(IList<Variant>))
                {
                    return (dynamic)Array.Empty<Variant>();
                }
                if (typeof(T) == typeof(IList<NameValue>))
                {
                    return (dynamic)Array.Empty<NameValue>();
                }
                throw new InvalidOperationException("Type " + typeof(T).Name + " not supported");
            }
            return v;
        }

        public static implicit operator bool(Variant v) { return v.GetData<bool>(); }
        public static implicit operator sbyte(Variant v) { return v.GetData<sbyte>(); }
        public static implicit operator byte(Variant v) { return v.GetData<byte>(); }
        public static implicit operator short(Variant v) { return v.GetData<short>(); }
        public static implicit operator ushort(Variant v) { return v.GetData<ushort>(); }
        public static implicit operator int(Variant v) { return v.GetData<int>(); }
        public static implicit operator uint(Variant v) { return v.GetData<uint>(); }
        public static implicit operator long(Variant v) { return v.GetData<long>(); }
        public static implicit operator ulong(Variant v) { return v.GetData<ulong>(); }
        public static implicit operator float(Variant v) { return v.GetData<float>(); }
        public static implicit operator double(Variant v) { return v.GetData<double>(); }
        public static implicit operator string(Variant v) { return v.GetData<string>(); }
        public static implicit operator byte[](Variant v) { return v.GetData<byte[]>(); }
        public static implicit operator bool[](Variant v) { return v.GetData<bool[]>(); }
        public static implicit operator sbyte[](Variant v) { return v.GetData<sbyte[]>(); }
//        public static implicit operator byte[](Variant v) { return v.GetData<byte[]>(); }
        public static implicit operator short[](Variant v) { return v.GetData<short[]>(); }
        public static implicit operator ushort[](Variant v) { return v.GetData<ushort[]>(); }
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
    }
}
