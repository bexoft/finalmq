using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Linq;

namespace finalmq
{
    public abstract class VariantValueBase<T> : IVariantValue
    {
        public VariantValueBase(T value)
        {
            m_value = value;
        }
        public dynamic Data
        {
            get
            {
                return m_value!;
            }
            set
            {
                m_value = value;
            }
        }

        public Variant? GetVariant(string name)
        {
            return null;
        }
        public virtual bool Equals(IVariantValue? rhs)
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

            bool res = m_value == rhs.Data;
            return res;
        }
        public bool Add(string name, Variant variant)
        {
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
                return 1;
            }
        }

        public void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name)
        {
            visitor.EnterLeaf(variant, VarType, index, level, size, name);

        }

        public abstract int VarType { get; }
        public abstract IVariantValue Clone();

        protected T m_value;
    }

    public abstract class VariantValueReferenceBase<T> : VariantValueBase<T>
    {
        public VariantValueReferenceBase(T value)
            : base(value)
        {
        }
        public override bool Equals(IVariantValue? rhs)
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

            bool res = m_value!.Equals(rhs.Data);
            return res;
        }
    }

    public abstract class VariantValueArrayBase<T> : IVariantValue
    {
        public VariantValueArrayBase(T[] value)
        {
            m_value = value;
        }
        public dynamic Data
        {
            get
            {
                return m_value!;
            }
            set
            {
                m_value = value;
            }
        }

        public Variant? GetVariant(string name)
        {
            return null;
        }
        public virtual bool Equals(IVariantValue? rhs)
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

            bool res = m_value!.SequenceEqual((T[])rhs.Data);
            return res;
        }
        public bool Add(string name, Variant variant)
        {
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
                return 1;
            }
        }

        public void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name)
        {
            visitor.EnterLeaf(variant, VarType, index, level, size, name);

        }

        public abstract int VarType { get; }
        public abstract IVariantValue Clone();

        protected T[] m_value;
    }

    public abstract class VariantValueListBase<T> : IVariantValue
    {
        public VariantValueListBase(IList<T> value)
        {
            m_value = value;
        }
        public dynamic Data
        {
            get
            {
                return m_value!;
            }
            set
            {
                m_value = value;
            }
        }

        public Variant? GetVariant(string name)
        {
            return null;
        }
        public virtual bool Equals(IVariantValue? rhs)
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

            bool res = m_value!.SequenceEqual((IList<T>)rhs.Data);
            return res;
        }
        public bool Add(string name, Variant variant)
        {
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
                return 1;
            }
        }

        public void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name)
        {
            visitor.EnterLeaf(variant, VarType, index, level, size, name);

        }

        public abstract int VarType { get; }
        public abstract IVariantValue Clone();

        protected IList<T> m_value;
    }

    public abstract class VariantValueListOfArrayBase<T> : IVariantValue
    {
        public VariantValueListOfArrayBase(IList<T[]> value)
        {
            m_value = value;
        }
        public dynamic Data
        {
            get
            {
                return m_value!;
            }
            set
            {
                m_value = value;
            }
        }

        public Variant? GetVariant(string name)
        {
            return null;
        }
        public virtual bool Equals(IVariantValue? rhs)
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

            if (m_value.Count != ((IList<T[]>)rhs.Data).Count)
            {
                return false;
            }

            for (int i = 0; i < m_value.Count; i++)
            {
                T[] arr1 = m_value[i];
                T[] arr2 = rhs.Data[i];
                if (!arr1.SequenceEqual(arr2))
                {
                    return false;
                }
            }

            return true;
        }
        public bool Add(string name, Variant variant)
        {
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
                return 1;
            }
        }

        public void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name)
        {
            visitor.EnterLeaf(variant, VarType, index, level, size, name);

        }

        public abstract int VarType { get; }
        public abstract IVariantValue Clone();

        protected IList<T[]> m_value;
    }

    public class VariantValueBool : VariantValueBase<bool>
    {
        public VariantValueBool(bool data) : base(data) {}
        public override int VarType { get { return (int)MetaTypeId.TYPE_BOOL; } }
        public override IVariantValue Clone() { return new VariantValueBool(Data); }
    }
    public class VariantValueInt8 : VariantValueBase<sbyte>
    {
        public VariantValueInt8(sbyte data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT8; } }
        public override IVariantValue Clone() { return new VariantValueInt8(Data); }
    }
    public class VariantValueUInt8 : VariantValueBase<byte>
    {
        public VariantValueUInt8(byte data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT8; } }
        public override IVariantValue Clone() { return new VariantValueUInt8(Data); }
    }
    public class VariantValueInt16 : VariantValueBase<short>
    {
        public VariantValueInt16(short data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT16; } }
        public override IVariantValue Clone() { return new VariantValueInt16(Data); }
    }
    public class VariantValueUInt16 : VariantValueBase<ushort>
    {
        public VariantValueUInt16(ushort data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT16; } }
        public override IVariantValue Clone() { return new VariantValueUInt16(Data); }
    }
    public class VariantValueInt32 : VariantValueBase<int>
    {
        public VariantValueInt32(int data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT32; } }
        public override IVariantValue Clone() { return new VariantValueInt32(Data); }
    }
    public class VariantValueUInt32 : VariantValueBase<uint>
    {
        public VariantValueUInt32(uint data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT32; } }
        public override IVariantValue Clone() { return new VariantValueUInt32(Data); }
    }
    public class VariantValueInt64 : VariantValueBase<long>
    {
        public VariantValueInt64(long data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT64; } }
        public override IVariantValue Clone() { return new VariantValueInt64(Data); }
    }
    public class VariantValueUInt64 : VariantValueBase<ulong>
    {
        public VariantValueUInt64(ulong data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT64; } }
        public override IVariantValue Clone() { return new VariantValueUInt64(Data); }
    }
    public class VariantValueFloat : VariantValueBase<float>
    {
        public VariantValueFloat(float data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_FLOAT; } }
        public override IVariantValue Clone() { return new VariantValueFloat(Data); }
    }
    public class VariantValueDouble : VariantValueBase<double>
    {
        public VariantValueDouble(double data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_DOUBLE; } }
        public override IVariantValue Clone() { return new VariantValueDouble(Data); }
    }
    public class VariantValueString : VariantValueBase<string>
    {
        public VariantValueString(string data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_STRING; } }
        public override IVariantValue Clone() { return new VariantValueString(Data.Clone()); }
    }
    public class VariantValueBytes : VariantValueBase<byte[]>
    {
        public VariantValueBytes(byte[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_BYTES; } }
        public override IVariantValue Clone() { return new VariantValueBytes(Data.Clone()); }
    }
    public class VariantValueArrayBool : VariantValueArrayBase<bool>
    {
        public VariantValueArrayBool(bool[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_BOOL; } }
        public override IVariantValue Clone() { return new VariantValueArrayBool(Data.Clone()); }
    }
    public class VariantValueArrayInt8 : VariantValueArrayBase<sbyte>
    {
        public VariantValueArrayInt8(sbyte[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT8; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt8(Data.Clone()); }
    }
    public class VariantValueArrayInt16 : VariantValueArrayBase<short>
    {
        public VariantValueArrayInt16(short[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT16; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt16(Data.Clone()); }
    }
    public class VariantValueArrayUInt16 : VariantValueArrayBase<ushort>
    {
        public VariantValueArrayUInt16(ushort[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_UINT16; } }
        public override IVariantValue Clone() { return new VariantValueArrayUInt16(Data.Clone()); }
    }
    public class VariantValueArrayInt32 : VariantValueArrayBase<int>
    {
        public VariantValueArrayInt32(int[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT32; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt32(Data.Clone()); }
    }
    public class VariantValueArrayUInt32 : VariantValueArrayBase<uint>
    {
        public VariantValueArrayUInt32(uint[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_UINT32; } }
        public override IVariantValue Clone() { return new VariantValueArrayUInt32(Data.Clone()); }
    }
    public class VariantValueArrayInt64 : VariantValueArrayBase<long>
    {
        public VariantValueArrayInt64(long[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT64; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt64(Data.Clone()); }
    }
    public class VariantValueArrayUInt64 : VariantValueArrayBase<ulong>
    {
        public VariantValueArrayUInt64(ulong[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_UINT64; } }
        public override IVariantValue Clone() { return new VariantValueArrayUInt64(Data.Clone()); }
    }
    public class VariantValueArrayFloat : VariantValueArrayBase<float>
    {
        public VariantValueArrayFloat(float[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_FLOAT; } }
        public override IVariantValue Clone() { return new VariantValueArrayFloat(Data.Clone()); }
    }
    public class VariantValueArrayDouble : VariantValueArrayBase<double>
    {
        public VariantValueArrayDouble(double[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_DOUBLE; } }
        public override IVariantValue Clone() { return new VariantValueArrayDouble(Data.Clone()); }
    }
    public class VariantValueArrayString : VariantValueListBase<string>
    {
        public VariantValueArrayString(IList<string> data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_STRING; } }
        public override IVariantValue Clone() { return new VariantValueArrayString(Data.Clone()); }
    }
    public class VariantValueArrayBytes : VariantValueListOfArrayBase<byte>
    {
        public VariantValueArrayBytes(IList<byte[]> data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_BYTES; } }
        public override IVariantValue Clone() { return new VariantValueArrayBytes(Data.Clone()); }
    }

    class RegisterVariantValues
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register()
        {
            VariantValueFactory.Instance.Register<bool>((dynamic data) => { return new VariantValueBool(data); });
            VariantValueFactory.Instance.Register<sbyte>((dynamic data) => { return new VariantValueInt8(data); });
            VariantValueFactory.Instance.Register<byte>((dynamic data) => { return new VariantValueUInt8(data); });
            VariantValueFactory.Instance.Register<short>((dynamic data) => { return new VariantValueInt16(data); });
            VariantValueFactory.Instance.Register<ushort>((dynamic data) => { return new VariantValueUInt16(data); });
            VariantValueFactory.Instance.Register<int>((dynamic data) => { return new VariantValueInt32(data); });
            VariantValueFactory.Instance.Register<uint>((dynamic data) => { return new VariantValueUInt32(data); });
            VariantValueFactory.Instance.Register<long>((dynamic data) => { return new VariantValueInt64(data); });
            VariantValueFactory.Instance.Register<ulong>((dynamic data) => { return new VariantValueUInt64(data); });
            VariantValueFactory.Instance.Register<float>((dynamic data) => { return new VariantValueFloat(data); });
            VariantValueFactory.Instance.Register<double>((dynamic data) => { return new VariantValueDouble(data); });
            VariantValueFactory.Instance.Register<string>((dynamic data) => { return new VariantValueString(data); });
            VariantValueFactory.Instance.Register<byte[]>((dynamic data) => { return new VariantValueBytes(data); });
            VariantValueFactory.Instance.Register<bool[]>((dynamic data) => { return new VariantValueArrayBool(data); });
            VariantValueFactory.Instance.Register<sbyte[]>((dynamic data) => { return new VariantValueArrayInt8(data); });
            VariantValueFactory.Instance.Register<short[]>((dynamic data) => { return new VariantValueArrayInt16(data); });
            VariantValueFactory.Instance.Register<ushort[]>((dynamic data) => { return new VariantValueArrayUInt16(data); });
            VariantValueFactory.Instance.Register<int[]>((dynamic data) => { return new VariantValueArrayInt32(data); });
            VariantValueFactory.Instance.Register<uint[]>((dynamic data) => { return new VariantValueArrayUInt32(data); });
            VariantValueFactory.Instance.Register<long[]>((dynamic data) => { return new VariantValueArrayInt64(data); });
            VariantValueFactory.Instance.Register<ulong[]>((dynamic data) => { return new VariantValueArrayUInt64(data); });
            VariantValueFactory.Instance.Register<float[]>((dynamic data) => { return new VariantValueArrayFloat(data); });
            VariantValueFactory.Instance.Register<double[]>((dynamic data) => { return new VariantValueArrayDouble(data); });
            VariantValueFactory.Instance.Register<IList<string>>((dynamic data) => { return new VariantValueArrayString(data); });
            VariantValueFactory.Instance.Register<IList<byte[]>>((dynamic data) => { return new VariantValueArrayBytes(data); });
        }
    }
}
