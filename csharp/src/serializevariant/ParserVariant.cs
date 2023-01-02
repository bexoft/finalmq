using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{
    using VariantList = IList<Variant>;

    public class ParserVariant
    {
        private static string STR_VARVALUE = "finalmq.variant.VarValue";

        public ParserVariant(IParserVisitor visitor, Variant variant)
        {
            m_visitor = visitor;
            m_root = variant;
        }

        public bool ParseStruct(string typeName)
        {
            bool ok = false;
            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(typeName);
            if (stru != null)
            {
                m_visitor.StartStruct(stru);
                ParseStruct(stru, m_root);
                ok = true;
            }
            else
            {
                m_visitor.NotifyError(EmptyString, "typename not found");
            }

            m_visitor.Finished();
            return ok;
        }

        private void ParseStruct(MetaStruct stru, Variant variant)
        {
            int size = stru.FieldsSize;
            for (int i = 0; i < size; ++i)
            {
                MetaField? field = stru.GetFieldByIndex(i);
                Debug.Assert(field != null);
                Variant? sub = variant.GetVariant(field.Name);
                if (sub != null)
                {
                    ProcessField(sub, field);
                }
                else
                {
                    ProcessEmptyField(field);
                }
            }
        }

        private void ParseStruct(MetaStruct stru)
        {
            int size = stru.FieldsSize;
            for (int i = 0; i < size; ++i)
            {
                MetaField? field = stru.GetFieldByIndex(i);
                Debug.Assert(field != null);
                ProcessEmptyField(field);
            }
        }

        private void ProcessField(Variant sub, MetaField field)
        {
            switch (field.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    m_visitor.EnterBool(field, sub);
                    break;
                case MetaTypeId.TYPE_INT32:
                    m_visitor.EnterInt32(field, sub);
                    break;
                case MetaTypeId.TYPE_UINT32:
                    m_visitor.EnterUInt32(field, sub);
                    break;
                case MetaTypeId.TYPE_INT64:
                    m_visitor.EnterInt64(field, sub);
                    break;
                case MetaTypeId.TYPE_UINT64:
                    m_visitor.EnterUInt64(field, sub);
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    m_visitor.EnterFloat(field, sub);
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    m_visitor.EnterDouble(field, sub);
                    break;
                case MetaTypeId.TYPE_STRING:
                    m_visitor.EnterString(field, sub);
                    break;
                case MetaTypeId.TYPE_BYTES:
                    m_visitor.EnterBytes(field, sub);
                    break;
                case MetaTypeId.TYPE_STRUCT:
                    {
                        m_visitor.EnterStruct(field);
                        if (field.TypeName == STR_VARVALUE)
                        {
                            VariantToVarValue variantToVarValue = new VariantToVarValue(sub, m_visitor);
                            variantToVarValue.Convert();
                        }
                        else
                        {
                            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
                            if (stru != null)
                            {
                                ParseStruct(stru, sub);
                            }
                            else
                            {
                                m_visitor.NotifyError(EmptyString, "typename not found");
                            }
                        }
                        m_visitor.ExitStruct(field);
                    }
                    break;
                case MetaTypeId.TYPE_ENUM:
                    {
                        if (sub.VarType == (int)MetaTypeId.TYPE_STRING)
                        {
                            string str = sub;
                            m_visitor.EnterEnum(field, str);
                        }
                        else
                        {
                            int value = sub;
                            m_visitor.EnterEnum(field, value);
                        }
                    }
                    break;
                    
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    m_visitor.EnterArrayBool(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    m_visitor.EnterArrayInt32(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    m_visitor.EnterArrayUInt32(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    m_visitor.EnterArrayInt64(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    m_visitor.EnterArrayUInt64(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    m_visitor.EnterArrayFloat(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    m_visitor.EnterArrayDouble(field, sub);
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    m_visitor.EnterArrayString(field, sub.GetData<IList<string>>());
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    m_visitor.EnterArrayBytes(field, sub.GetData<IList<byte[]>>());
                    break;
                case MetaTypeId.TYPE_ARRAY_STRUCT:
                    {
                        m_visitor.EnterArrayStruct(field);
                        VariantList list = sub.GetData<VariantList>();
                        MetaField? fieldWithoutArray = MetaDataGlobal.Instance.GetArrayField(field);
                        Debug.Assert(fieldWithoutArray != null);
                        MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(fieldWithoutArray);
                        if (stru != null)
                        {
                            foreach (Variant element in list)
                            {
                                m_visitor.EnterStruct(fieldWithoutArray);
                                ParseStruct(stru, element);
                                m_visitor.ExitStruct(fieldWithoutArray);

                            }
                        }
                        m_visitor.ExitArrayStruct(field);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    {
                        if (sub.VarType == (int)MetaTypeId.TYPE_ARRAY_STRING)
                        {
                            IList<string> value = sub.GetData<IList<string>>();
                            m_visitor.EnterArrayEnum(field, value);
                        }
                        else
                        {
                            int[] value = sub;
                            m_visitor.EnterArrayEnum(field, value);
                        }
                    }
                    break;
                default:
                    Debug.Assert(false);
                    break;
            }
        }

        private void ProcessEmptyField(MetaField field)
        {
            switch (field.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    m_visitor.EnterBool(field, false);
                    break;
                case MetaTypeId.TYPE_INT32:
                    m_visitor.EnterInt32(field, 0);
                    break;
                case MetaTypeId.TYPE_UINT32:
                    m_visitor.EnterUInt32(field, 0);
                    break;
                case MetaTypeId.TYPE_INT64:
                    m_visitor.EnterInt64(field, 0);
                    break;
                case MetaTypeId.TYPE_UINT64:
                    m_visitor.EnterUInt64(field, 0);
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    m_visitor.EnterFloat(field, 0.0f);
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    m_visitor.EnterDouble(field, 0.0);
                    break;
                case MetaTypeId.TYPE_STRING:
                    m_visitor.EnterString(field, EmptyString);
                    break;
                case MetaTypeId.TYPE_BYTES:
                    m_visitor.EnterBytes(field, EmptyBytes);
                    break;
                case MetaTypeId.TYPE_STRUCT:
                    {
                        m_visitor.EnterStruct(field);
                        if (field.TypeName == STR_VARVALUE)
                        {
                            VariantToVarValue variantToVarValue = new VariantToVarValue(new Variant(), m_visitor);
                            variantToVarValue.Convert();
                        }
                        else
                        {
                            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
                            if (stru != null)
                            {
                                ParseStruct(stru);
                            }
                            else
                            {
                                m_visitor.NotifyError(EmptyString, "typename not found");
                            }
                        }
                        m_visitor.ExitStruct(field);
                    }
                    break;
                case MetaTypeId.TYPE_ENUM:
                    m_visitor.EnterEnum(field, 0);
                    break;
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    m_visitor.EnterArrayBool(field, EmptyArrayBool);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    m_visitor.EnterArrayInt32(field, EmptyArrayInt32);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    m_visitor.EnterArrayUInt32(field, EmptyArrayUInt32);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    m_visitor.EnterArrayInt64(field, EmptyArrayInt64);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    m_visitor.EnterArrayUInt64(field, EmptyArrayUInt64);
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    m_visitor.EnterArrayFloat(field, EmptyArrayFloat);
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    m_visitor.EnterArrayDouble(field, EmptyArrayDouble);
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    m_visitor.EnterArrayString(field, EmptyArrayString);
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    m_visitor.EnterArrayBytes(field, EmptyArrayBytes);
                    break;
                case MetaTypeId.TYPE_ARRAY_STRUCT:
                    m_visitor.EnterArrayStruct(field);
                    m_visitor.ExitArrayStruct(field);
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    m_visitor.EnterArrayEnum(field, EmptyArrayInt32);
                    break;
                default:
                    Debug.Assert(false);
                    break;
            }
        }

        IParserVisitor  m_visitor;
        Variant         m_root;

        static readonly string EmptyString = "";
        static readonly byte[] EmptyBytes = new byte[0];
        static readonly bool[] EmptyArrayBool = new bool[0];
        static readonly int[] EmptyArrayInt32 = new int[0];
        static readonly uint[] EmptyArrayUInt32 = new uint[0];
        static readonly long[] EmptyArrayInt64 = new long[0];
        static readonly ulong[] EmptyArrayUInt64 = new ulong[0];
        static readonly float[] EmptyArrayFloat = new float[0];
        static readonly double[] EmptyArrayDouble = new double[0];
        static readonly string[] EmptyArrayString = new string[0];
        static readonly byte[][] EmptyArrayBytes = new byte[0][];
    }
}
