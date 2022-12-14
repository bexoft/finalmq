using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace finalmq
{
    public abstract class VariantValueBase : IVariantValue
    {
        public VariantValueBase(dynamic value)
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

        public Variant? GetVariant(string name)
        {
            return null;
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

        private dynamic m_value;
    }

    public class VariantValueBool : VariantValueBase
    {
        public VariantValueBool(bool data) : base(data) {}
        public override int VarType { get { return (int)MetaTypeId.TYPE_BOOL; } }
        public override IVariantValue Clone() { return new VariantValueBool(Data); }
    }
    public class VariantValueInt32 : VariantValueBase
    {
        public VariantValueInt32(int data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT32; } }
        public override IVariantValue Clone() { return new VariantValueInt32(Data); }
    }
    public class VariantValueUInt32 : VariantValueBase
    {
        public VariantValueUInt32(uint data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT32; } }
        public override IVariantValue Clone() { return new VariantValueUInt32(Data); }
    }
    public class VariantValueInt64 : VariantValueBase
    {
        public VariantValueInt64(long data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_INT64; } }
        public override IVariantValue Clone() { return new VariantValueInt64(Data); }
    }
    public class VariantValueUInt64 : VariantValueBase
    {
        public VariantValueUInt64(ulong data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_UINT64; } }
        public override IVariantValue Clone() { return new VariantValueUInt64(Data); }
    }
    public class VariantValueFloat : VariantValueBase
    {
        public VariantValueFloat(float data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_FLOAT; } }
        public override IVariantValue Clone() { return new VariantValueFloat(Data); }
    }
    public class VariantValueDouble : VariantValueBase
    {
        public VariantValueDouble(double data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_DOUBLE; } }
        public override IVariantValue Clone() { return new VariantValueDouble(Data); }
    }
    public class VariantValueString : VariantValueBase
    {
        public VariantValueString(string data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_STRING; } }
        public override IVariantValue Clone() { return new VariantValueString(Data.Clone()); }
    }
    public class VariantValueBytes : VariantValueBase
    {
        public VariantValueBytes(byte[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_BYTES; } }
        public override IVariantValue Clone() { return new VariantValueBytes(Data.Clone()); }
    }
    public class VariantValueArrayBool : VariantValueBase
    {
        public VariantValueArrayBool(bool[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_BOOL; } }
        public override IVariantValue Clone() { return new VariantValueArrayBool(Data.Clone()); }
    }
    public class VariantValueArrayInt32 : VariantValueBase
    {
        public VariantValueArrayInt32(int[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT32; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt32(Data.Clone()); }
    }
    public class VariantValueArrayUInt32 : VariantValueBase
    {
        public VariantValueArrayUInt32(uint[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_UINT32; } }
        public override IVariantValue Clone() { return new VariantValueArrayUInt32(Data.Clone()); }
    }
    public class VariantValueArrayInt64 : VariantValueBase
    {
        public VariantValueArrayInt64(long[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_INT64; } }
        public override IVariantValue Clone() { return new VariantValueArrayInt64(Data.Clone()); }
    }
    public class VariantValueArrayUInt64 : VariantValueBase
    {
        public VariantValueArrayUInt64(ulong[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_UINT64; } }
        public override IVariantValue Clone() { return new VariantValueArrayUInt64(Data.Clone()); }
    }
    public class VariantValueArrayFloat : VariantValueBase
    {
        public VariantValueArrayFloat(float[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_FLOAT; } }
        public override IVariantValue Clone() { return new VariantValueArrayFloat(Data.Clone()); }
    }
    public class VariantValueArrayDouble : VariantValueBase
    {
        public VariantValueArrayDouble(double[] data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_DOUBLE; } }
        public override IVariantValue Clone() { return new VariantValueArrayDouble(Data.Clone()); }
    }
    public class VariantValueArrayString : VariantValueBase
    {
        public VariantValueArrayString(IList<string> data) : base(data) { }
        public override int VarType { get { return (int)MetaTypeId.TYPE_ARRAY_STRING; } }
        public override IVariantValue Clone() { return new VariantValueArrayString(Data.Clone()); }
    }
    public class VariantValueArrayBytes : VariantValueBase
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
            VariantValueFactory.Instance.Register<int>((dynamic data) => { return new VariantValueInt32(data); });
            VariantValueFactory.Instance.Register<uint>((dynamic data) => { return new VariantValueUInt32(data); });
            VariantValueFactory.Instance.Register<long>((dynamic data) => { return new VariantValueInt64(data); });
            VariantValueFactory.Instance.Register<ulong>((dynamic data) => { return new VariantValueUInt64(data); });
            VariantValueFactory.Instance.Register<float>((dynamic data) => { return new VariantValueFloat(data); });
            VariantValueFactory.Instance.Register<double>((dynamic data) => { return new VariantValueDouble(data); });
            VariantValueFactory.Instance.Register<string>((dynamic data) => { return new VariantValueString(data); });
            VariantValueFactory.Instance.Register<byte[]>((dynamic data) => { return new VariantValueBytes(data); });
            VariantValueFactory.Instance.Register<bool[]>((dynamic data) => { return new VariantValueArrayBool(data); });
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
