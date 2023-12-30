using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;

namespace finalmq
{
    public class ParserStruct
    {
        static readonly string STR_VARVALUE = "finalmq.variant.VarValue";

        public ParserStruct(IParserVisitor visitor, StructBase structBase)
        {
            m_visitor = visitor;
            m_root = structBase;
        }

        public bool ParseStruct()
        {
            MetaStruct stru = m_root.MetaStruct;

            m_visitor.StartStruct(stru);
            ParseStruct(m_root, stru);
            m_visitor.Finished();
            return true;
        }

        void ParseStruct(StructBase structBase, MetaStruct stru)
        {
            Type type = structBase.GetType();
            int size = stru.FieldsSize;
            for (int i = 0; i < size; ++i)
            {
                MetaField field = stru.GetFieldByIndex(i)!;
                PropertyInfo? property = type.GetProperty(field.Name);
                if (property != null)
                {
                    ProcessField(structBase, field, property);
                }
            }
        }

        private bool IsNullable(PropertyInfo property)
        {
            object[]? attributes = property.GetCustomAttributes(false);
            if (attributes != null)
            {
                foreach (var attribute in attributes)
                {
                    MetaFieldAttribute? attr = attribute as MetaFieldAttribute;
                    if (attr != null)
                    {
                        bool nullable = (attr.Flags & MetaFieldFlags.METAFLAG_NULLABLE) != 0;
                        return nullable;
                    }
                }
            }
            return false;
        }

        void ProcessField(StructBase structBase, MetaField field, PropertyInfo property)
        {
            object? value = property.GetValue(structBase);
            switch (field.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    if (value != null && value.GetType() == typeof(bool))
                    {
                        m_visitor.EnterBool(field, (bool)value);
                    }
                    break;
                case MetaTypeId.TYPE_INT8:
                    if (value != null && value.GetType() == typeof(sbyte))
                    {
                        m_visitor.EnterInt8(field, (sbyte)value);
                    }
                    break;
                case MetaTypeId.TYPE_UINT8:
                    if (value != null && value.GetType() == typeof(byte))
                    {
                        m_visitor.EnterUInt8(field, (byte)value);
                    }
                    break;
                case MetaTypeId.TYPE_INT16:
                    if (value != null && value.GetType() == typeof(short))
                    {
                        m_visitor.EnterInt32(field, (short)value);
                    }
                    break;
                case MetaTypeId.TYPE_UINT16:
                    if (value != null && value.GetType() == typeof(ushort))
                    {
                        m_visitor.EnterUInt32(field, (ushort)value);
                    }
                    break;
                case MetaTypeId.TYPE_INT32:
                    if (value != null && value.GetType() == typeof(int))
                    {
                        m_visitor.EnterInt32(field, (int)value);
                    }
                    break;
                case MetaTypeId.TYPE_UINT32:
                    if (value != null && value.GetType() == typeof(uint))
                    {
                        m_visitor.EnterUInt32(field, (uint)value);
                    }
                    break;
                case MetaTypeId.TYPE_INT64:
                    if (value != null && value.GetType() == typeof(long))
                    {
                        m_visitor.EnterInt64(field, (long)value);
                    }
                    break;
                case MetaTypeId.TYPE_UINT64:
                    if (value != null && value.GetType() == typeof(ulong))
                    {
                        m_visitor.EnterUInt64(field, (ulong)value);
                    }
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    if (value != null && value.GetType() == typeof(float))
                    {
                        m_visitor.EnterFloat(field, (float)value);
                    }
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    if (value != null && value.GetType() == typeof(double))
                    {
                        m_visitor.EnterDouble(field, (double)value);
                    }
                    break;
                case MetaTypeId.TYPE_STRING:
                    if (value != null && value.GetType() == typeof(string))
                    {
                        m_visitor.EnterString(field, (string)value);
                    }
                    break;
                case MetaTypeId.TYPE_BYTES:
                    var vBytes = value as byte[];
                    if (vBytes != null)
                    {
                        m_visitor.EnterBytes(field, vBytes, 0, vBytes.Length);
                    }
                    break;
                case MetaTypeId.TYPE_STRUCT:
                    if (field.TypeName == STR_VARVALUE)
                    {
                        var v = value as Variant;
                        if (v != null)
                        {
                            m_visitor.EnterStruct(field);
                            VariantToVarValue variantToVarValue = new VariantToVarValue(v, m_visitor);
                            variantToVarValue.Convert();
                            m_visitor.ExitStruct(field);
                        }
                    }
                    else
                    {
                        StructBase? sub = value as StructBase;
                        if ((sub != null) || !IsNullable(property))
                        {
                            if (sub != null)
                            {
                                m_visitor.EnterStruct(field);
                                ParseStruct(sub, sub.MetaStruct);
                                m_visitor.ExitStruct(field);
                            }
                            else
                            {
                                try
                                {
                                    sub = Activator.CreateInstance(property.PropertyType) as StructBase;
                                }
                                catch (Exception)
                                {
                                }
                                if (sub != null)
                                {
                                    m_visitor.EnterStruct(field);
                                    ParseStruct(sub, sub.MetaStruct);
                                    m_visitor.ExitStruct(field);
                                }
                            }
                        }
                        else
                        {
                            m_visitor.EnterStructNull(field);
                        }
                    }
                    break;
                case MetaTypeId.TYPE_ENUM:
                    {
                        System.Enum? en = value as System.Enum;
                        if (en != null)
                        {
                            m_visitor.EnterEnum(field, System.Convert.ToInt32(en));
                        }
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    var vArrayBool = value as bool[];
                    if (vArrayBool != null)
                    {
                        m_visitor.EnterArrayBool(field, vArrayBool);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT8:
                    var vArrayInt8 = value as sbyte[];
                    if (vArrayInt8 != null)
                    {
                        m_visitor.EnterArrayInt8(field, vArrayInt8);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT8:
                    var vArrayUInt8 = value as byte[];
                    if (vArrayUInt8 != null)
                    {
                        m_visitor.EnterArrayUInt8(field, vArrayUInt8);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT16:
                    var vArrayInt16 = value as short[];
                    if (vArrayInt16 != null)
                    {
                        m_visitor.EnterArrayInt16(field, vArrayInt16);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT16:
                    var vArrayUInt16 = value as ushort[];
                    if (vArrayUInt16 != null)
                    {
                        m_visitor.EnterArrayUInt16(field, vArrayUInt16);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    var vArrayInt32 = value as int[];
                    if (vArrayInt32 != null)
                    {
                        m_visitor.EnterArrayInt32(field, vArrayInt32);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    var vArrayUInt32 = value as uint[];
                    if (vArrayUInt32 != null)
                    {
                        m_visitor.EnterArrayUInt32(field, vArrayUInt32);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    var vArrayInt64 = value as long[];
                    if (vArrayInt64 != null)
                    {
                        m_visitor.EnterArrayInt64(field, vArrayInt64);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    var vArrayUInt64 = value as ulong[];
                    if (vArrayUInt64 != null)
                    {
                        m_visitor.EnterArrayUInt64(field, vArrayUInt64);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    var vArrayFloat = value as float[];
                    if (vArrayFloat != null)
                    {
                        m_visitor.EnterArrayFloat(field, vArrayFloat);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    var vArrayDouble = value as double[];
                    if (vArrayDouble != null)
                    {
                        m_visitor.EnterArrayDouble(field, vArrayDouble);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    var vArrayString = value as IList<string>;
                    if (vArrayString != null)
                    {
                        m_visitor.EnterArrayString(field, vArrayString);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    var vArrayBytes = value as IList<byte[]>;
                    if (vArrayBytes != null)
                    {
                        m_visitor.EnterArrayBytes(field, vArrayBytes);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_STRUCT:
                    {
                        m_visitor.EnterArrayStruct(field);
                        MetaField? fieldWithoutArray = field.FieldWithoutArray;
                        System.Collections.IEnumerable? enumerable = value as System.Collections.IEnumerable;
                        if (enumerable != null && fieldWithoutArray != null)
                        {
                            foreach (StructBase sub in enumerable)
                            {
                                if (sub != null)
                                {
                                    m_visitor.EnterStruct(fieldWithoutArray);
                                    ParseStruct(sub, sub.MetaStruct);
                                    m_visitor.ExitStruct(fieldWithoutArray);
                                }
                            }
                        }
                        m_visitor.ExitArrayStruct(field);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    {
                        System.Collections.IEnumerable? enumerable = value as System.Collections.IEnumerable;
                        if (enumerable != null)
                        {
                            int size = 0;
                            foreach (var sub in enumerable)
                            {
                                ++size;
                            }
                            int[] arrayInt32 = new int[size];
                            int i = 0;
                            foreach (var entry in enumerable)
                            {
                                System.Enum? en = entry as System.Enum;
                                if (en != null)
                                {
                                    arrayInt32[i] = System.Convert.ToInt32(en);
                                }
                                else
                                {
                                    arrayInt32[i] = 0;
                                }
                                ++i;
                            }
                            m_visitor.EnterArrayEnum(field, arrayInt32);
                        }
                    }
                    break;
                default:
                    Debug.Assert(false);
                    break;
            }
        }

        IParserVisitor  m_visitor;
        StructBase      m_root;
    }
}
