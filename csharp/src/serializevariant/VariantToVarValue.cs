using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{
    public class VariantToVarValue : IVariantVisitor
    {
        public VariantToVarValue(Variant variant, IParserVisitor visitor)
        {
            m_variant = variant;
            m_visitor = visitor;
        }

        public void Convert()
        {
            if (m_structVarValue == null)
            {
                m_structVarValue = MetaDataGlobal.Instance.GetStruct("finalmq.variant.VarValue");
            }
            Debug.Assert(m_structVarValue != null);
            m_variant.Accept(this);
        }

        // IVariantVisitor
        public void EnterLeaf(Variant var, int type, int index, int level, int size, string name)
        {
            Variant variant = var;

            Debug.Assert(m_structVarValue != null);
            if (m_fieldStruct == null)
            {
                m_fieldStruct = m_structVarValue.GetFieldByName("vallist");
            }
            MetaField? fieldStruct = m_fieldStruct;
            Debug.Assert(fieldStruct != null);

            if (m_fieldStructWithoutArray == null)
            {
                m_fieldStructWithoutArray = fieldStruct.FieldWithoutArray;
            }
            MetaField? fieldStructWithoutArray = m_fieldStructWithoutArray;
            Debug.Assert(fieldStructWithoutArray != null);

            if (level > 0)
            {
                m_visitor.EnterStruct(fieldStructWithoutArray);
            }

            if (name.Length != 0)
            {
                if (m_fieldName == null)
                {
                    m_fieldName = m_structVarValue.GetFieldByName("name");
                }
                MetaField? fieldName = m_fieldName;
                Debug.Assert(fieldName != null);
                m_visitor.EnterString(fieldName, name);
            }

            if (m_fieldType == null)
            {
                m_fieldType = m_structVarValue.GetFieldByName("type");
            }
            MetaField? fieldType = m_fieldType;
            Debug.Assert(fieldType != null);
            m_visitor.EnterEnum(fieldType, type);

            switch (type)
            {
                case (int)MetaTypeId.TYPE_NONE:
                    break;
                case (int)MetaTypeId.TYPE_BOOL:
                    {
                        if (m_fieldBool == null)
                        {
                            m_fieldBool = m_structVarValue.GetFieldByName("valbool");
                        }
                        MetaField? fieldBool = m_fieldBool;
                        Debug.Assert(fieldBool != null);
                        bool data = variant;
                        m_visitor.EnterBool(fieldBool, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_INT8:
                    {
                        if (m_fieldInt8 == null)
                        {
                            m_fieldInt8 = m_structVarValue.GetFieldByName("valint8");
                        }
                        MetaField? fieldInt8 = m_fieldInt8;
                        Debug.Assert(fieldInt8 != null);
                        sbyte data = variant;
                        m_visitor.EnterInt8(fieldInt8, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_UINT8:
                    {
                        if (m_fieldUInt8 == null)
                        {
                            m_fieldUInt8 = m_structVarValue.GetFieldByName("valuint8");
                        }
                        MetaField? fieldUInt8 = m_fieldUInt8;
                        Debug.Assert(fieldUInt8 != null);
                        byte data = variant;
                        m_visitor.EnterUInt8(fieldUInt8, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_INT16:
                    {
                        if (m_fieldInt16 == null)
                        {
                            m_fieldInt16 = m_structVarValue.GetFieldByName("valint16");
                        }
                        MetaField? fieldInt16 = m_fieldInt16;
                        Debug.Assert(fieldInt16 != null);
                        short data = variant;
                        m_visitor.EnterInt16(fieldInt16, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_UINT16:
                    {
                        if (m_fieldUInt16 == null)
                        {
                            m_fieldUInt16 = m_structVarValue.GetFieldByName("valuint16");
                        }
                        MetaField? fieldUInt16 = m_fieldUInt16;
                        Debug.Assert(fieldUInt16 != null);
                        ushort data = variant;
                        m_visitor.EnterUInt16(fieldUInt16, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_INT32:
                    {
                        if (m_fieldInt32 == null)
                        {
                            m_fieldInt32 = m_structVarValue.GetFieldByName("valint32");
                        }
                        MetaField? fieldInt32 = m_fieldInt32;
                        Debug.Assert(fieldInt32 != null);
                        int data = variant;
                        m_visitor.EnterInt32(fieldInt32, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_UINT32:
                    {
                        if (m_fieldUInt32 == null)
                        {
                            m_fieldUInt32 = m_structVarValue.GetFieldByName("valuint32");
                        }
                        MetaField? fieldUInt32 = m_fieldUInt32;
                        Debug.Assert(fieldUInt32 != null);
                        uint data = variant;
                        m_visitor.EnterUInt32(fieldUInt32, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_INT64:
                    {
                        if (m_fieldInt64 == null)
                        {
                            m_fieldInt64 = m_structVarValue.GetFieldByName("valint64");
                        }
                        MetaField? fieldInt64 = m_fieldInt64;
                        Debug.Assert(fieldInt64 != null);
                        long data = variant;
                        m_visitor.EnterInt64(fieldInt64, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_UINT64:
                    {
                        if (m_fieldUInt64 == null)
                        {
                            m_fieldUInt64 = m_structVarValue.GetFieldByName("valuint64");
                        }
                        MetaField? fieldUInt64 = m_fieldUInt64;
                        Debug.Assert(fieldUInt64 != null);
                        ulong data = variant;
                        m_visitor.EnterUInt64(fieldUInt64, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_FLOAT:
                    {
                        if (m_fieldFloat == null)
                        {
                            m_fieldFloat = m_structVarValue.GetFieldByName("valfloat");
                        }
                        MetaField? fieldFloat = m_fieldFloat;
                        Debug.Assert(fieldFloat != null);
                        float data = variant;
                        m_visitor.EnterFloat(fieldFloat, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_DOUBLE:
                    {
                        if (m_fieldDouble == null)
                        {
                            m_fieldDouble = m_structVarValue.GetFieldByName("valdouble");
                        }
                        MetaField? fieldDouble = m_fieldDouble;
                        Debug.Assert(fieldDouble != null);
                        double data = variant;
                        m_visitor.EnterDouble(fieldDouble, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_STRING:
                    {
                        if (m_fieldString == null)
                        {
                            m_fieldString = m_structVarValue.GetFieldByName("valstring");
                        }
                        MetaField? fieldString = m_fieldString;
                        Debug.Assert(fieldString != null);
                        string data = variant;
                        m_visitor.EnterString(fieldString, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_BYTES:
                    {
                        if (m_fieldBytes == null)
                        {
                            m_fieldBytes = m_structVarValue.GetFieldByName("valbytes");
                        }
                        MetaField? fieldBytes = m_fieldBytes;
                        Debug.Assert(fieldBytes != null);
                        byte[] data = variant;
                        m_visitor.EnterBytes(fieldBytes, data, 0, data.Length);
                    }
                    break;

                case (int)MetaTypeId.TYPE_ARRAY_BOOL:
                    {
                        if (m_fieldArrayBool == null)
                        {
                            m_fieldArrayBool = m_structVarValue.GetFieldByName("valarrbool");
                        }
                        MetaField? fieldArrayBool = m_fieldArrayBool;
                        Debug.Assert(fieldArrayBool != null);
                        bool[] data = variant;
                        m_visitor.EnterArrayBool(fieldArrayBool, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_INT8:
                    {
                        if (m_fieldArrayInt8 == null)
                        {
                            m_fieldArrayInt8 = m_structVarValue.GetFieldByName("valarrint8");
                        }
                        MetaField? fieldArrayInt8 = m_fieldArrayInt8;
                        Debug.Assert(fieldArrayInt8 != null);
                        sbyte[] data = variant;
                        m_visitor.EnterArrayInt8(fieldArrayInt8, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_INT16:
                    {
                        if (m_fieldArrayInt16 == null)
                        {
                            m_fieldArrayInt16 = m_structVarValue.GetFieldByName("valarrint16");
                        }
                        MetaField? fieldArrayInt16 = m_fieldArrayInt16;
                        Debug.Assert(fieldArrayInt16 != null);
                        short[] data = variant;
                        m_visitor.EnterArrayInt16(fieldArrayInt16, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_UINT16:
                    {
                        if (m_fieldArrayUInt16 == null)
                        {
                            m_fieldArrayUInt16 = m_structVarValue.GetFieldByName("valarruint16");
                        }
                        MetaField? fieldArrayUInt16 = m_fieldArrayUInt16;
                        Debug.Assert(fieldArrayUInt16 != null);
                        ushort[] data = variant;
                        m_visitor.EnterArrayUInt16(fieldArrayUInt16, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_INT32:
                    {
                        if (m_fieldArrayInt32 == null)
                        {
                            m_fieldArrayInt32 = m_structVarValue.GetFieldByName("valarrint32");
                        }
                        MetaField? fieldArrayInt32 = m_fieldArrayInt32;
                        Debug.Assert(fieldArrayInt32 != null);
                        int[] data = variant;
                        m_visitor.EnterArrayInt32(fieldArrayInt32, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_UINT32:
                    {
                        if (m_fieldArrayUInt32 == null)
                        {
                            m_fieldArrayUInt32 = m_structVarValue.GetFieldByName("valarruint32");
                        }
                        MetaField? fieldArrayUInt32 = m_fieldArrayUInt32;
                        Debug.Assert(fieldArrayUInt32 != null);
                        uint[] data = variant;
                        m_visitor.EnterArrayUInt32(fieldArrayUInt32, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_INT64:
                    {
                        if (m_fieldArrayInt64 == null)
                        {
                            m_fieldArrayInt64 = m_structVarValue.GetFieldByName("valarrint64");
                        }
                        MetaField? fieldArrayInt64 = m_fieldArrayInt64;
                        Debug.Assert(fieldArrayInt64 != null);
                        long[] data = variant;
                        m_visitor.EnterArrayInt64(fieldArrayInt64, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_UINT64:
                    {
                        if (m_fieldArrayUInt64 == null)
                        {
                            m_fieldArrayUInt64 = m_structVarValue.GetFieldByName("valarruint64");
                        }
                        MetaField? fieldArrayUInt64 = m_fieldArrayUInt64;
                        Debug.Assert(fieldArrayUInt64 != null);
                        ulong[] data = variant;
                        m_visitor.EnterArrayUInt64(fieldArrayUInt64, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_FLOAT:
                    {
                        if (m_fieldArrayFloat == null)
                        {
                            m_fieldArrayFloat = m_structVarValue.GetFieldByName("valarrfloat");
                        }
                        MetaField? fieldArrayFloat = m_fieldArrayFloat;
                        Debug.Assert(fieldArrayFloat != null);
                        float[] data = variant;
                        m_visitor.EnterArrayFloat(fieldArrayFloat, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_DOUBLE:
                    {
                        if (m_fieldArrayDouble == null)
                        {
                            m_fieldArrayDouble = m_structVarValue.GetFieldByName("valarrdouble");
                        }
                        MetaField? fieldArrayDouble = m_fieldArrayDouble;
                        Debug.Assert(fieldArrayDouble != null);
                        double[] data = variant;
                        m_visitor.EnterArrayDouble(fieldArrayDouble, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_STRING:
                    {
                        if (m_fieldArrayString == null)
                        {
                            m_fieldArrayString = m_structVarValue.GetFieldByName("valarrstring");
                        }
                        MetaField? fieldArrayString = m_fieldArrayString;
                        Debug.Assert(fieldArrayString != null);
                        IList<string> data = variant.GetData<IList<string>>();
                        m_visitor.EnterArrayString(fieldArrayString, data);
                    }
                    break;
                case (int)MetaTypeId.TYPE_ARRAY_BYTES:
                    {
                        if (m_fieldArrayBytes == null)
                        {
                            m_fieldArrayBytes = m_structVarValue.GetFieldByName("valarrbytes");
                        }
                        MetaField? fieldArrayBytes = m_fieldArrayBytes;
                        Debug.Assert(fieldArrayBytes != null);
                        IList<byte[]> data = variant.GetData<IList<byte[]>>();
                        m_visitor.EnterArrayBytes(fieldArrayBytes, data);
                    }
                    break;
            }

            if (level > 0)
            {
                m_visitor.ExitStruct(fieldStructWithoutArray);
            }
        }
        public void EnterStruct(Variant variant, int type, int index, int level, int size, string name)
        {
            if (m_fieldList == null)
            {
                Debug.Assert(m_structVarValue != null);
                m_fieldList = m_structVarValue.GetFieldByName("vallist");
            }
            MetaField? fieldList = m_fieldList;
            Debug.Assert(fieldList != null);

            if (m_fieldListWithoutArray == null)
            {
                m_fieldListWithoutArray = fieldList.FieldWithoutArray;
            }
            MetaField? fieldListWithoutArray = m_fieldListWithoutArray;
            Debug.Assert(fieldListWithoutArray != null);

            if (level > 0)
            {
                m_visitor.EnterStruct(fieldListWithoutArray);
            }

            if (name.Length != 0)
            {
                if (m_fieldName == null)
                {
                    Debug.Assert(m_structVarValue != null);
                    m_fieldName = m_structVarValue.GetFieldByName("name");
                }
                MetaField? fieldName = m_fieldName;
                Debug.Assert(fieldName != null);
                m_visitor.EnterString(fieldName, name);
            }

            if (m_fieldType == null)
            {
                Debug.Assert(m_structVarValue != null);
                m_fieldType = m_structVarValue.GetFieldByName("type");
            }
            MetaField? fieldType = m_fieldType;
            Debug.Assert(fieldType != null);
            m_visitor.EnterEnum(fieldType, type);

            m_visitor.EnterArrayStruct(fieldList);
        }
        public void ExitStruct(Variant variant, int type, int index, int level, int size, string name)
        {
            if (m_fieldList == null)
            {
                Debug.Assert(m_structVarValue != null);
                m_fieldList = m_structVarValue.GetFieldByName("vallist");
            }
            MetaField? fieldList = m_fieldList;
            Debug.Assert(fieldList != null);

            m_visitor.ExitArrayStruct(fieldList);

            if (m_fieldListWithoutArray == null)
            {
                m_fieldListWithoutArray = fieldList.FieldWithoutArray;
            }
            MetaField? fieldListWithoutArray = m_fieldListWithoutArray;
            Debug.Assert(fieldListWithoutArray != null);

            if (level > 0)
            {
                m_visitor.ExitStruct(fieldListWithoutArray);
            }
        }
        public void EnterList(Variant variant, int type, int index, int level, int size, string name)
        {
            EnterStruct(variant, type, index, level, size, name);
        }
        public void ExitList(Variant variant, int type, int index, int level, int size, string name)
        {
            ExitStruct(variant, type, index, level, size, name);
        }

        Variant m_variant;
        IParserVisitor m_visitor;
        static MetaStruct? m_structVarValue = null;

        static MetaField? m_fieldStruct = null;
        static MetaField? m_fieldStructWithoutArray = null;
        static MetaField? m_fieldName = null;
        static MetaField? m_fieldType = null;
        static MetaField? m_fieldBool = null;
        static MetaField? m_fieldInt8 = null;
        static MetaField? m_fieldUInt8 = null;
        static MetaField? m_fieldInt16 = null;
        static MetaField? m_fieldUInt16 = null;
        static MetaField? m_fieldInt32 = null;
        static MetaField? m_fieldUInt32 = null;
        static MetaField? m_fieldInt64 = null;
        static MetaField? m_fieldUInt64 = null;
        static MetaField? m_fieldFloat = null;
        static MetaField? m_fieldDouble = null;
        static MetaField? m_fieldString = null;
        static MetaField? m_fieldBytes = null;
        static MetaField? m_fieldArrayBool = null;
        static MetaField? m_fieldArrayInt8 = null;
        static MetaField? m_fieldArrayUInt8 = null;
        static MetaField? m_fieldArrayInt16 = null;
        static MetaField? m_fieldArrayUInt16 = null;
        static MetaField? m_fieldArrayInt32 = null;
        static MetaField? m_fieldArrayUInt32 = null;
        static MetaField? m_fieldArrayInt64 = null;
        static MetaField? m_fieldArrayUInt64 = null;
        static MetaField? m_fieldArrayFloat = null;
        static MetaField? m_fieldArrayDouble = null;
        static MetaField? m_fieldArrayString = null;
        static MetaField? m_fieldArrayBytes = null;

        static MetaField? m_fieldList = null;
        static MetaField? m_fieldListWithoutArray = null;
    }

}
