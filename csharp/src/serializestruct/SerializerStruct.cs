using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace finalmq
{

    public class SerializerStruct : ParserRedirect
    {
        static readonly string STR_VARVALUE = "finalmq.variant.VarValue";

        public SerializerStruct(StructBase root)
        {
            m_internal = new Internal(this, root);
            SetVisitor(m_internal);
        }
        public void SetExitNotification(VarValueToVariant.FuncExit? funcExit)
        {
            m_internal.SetExitNotification(funcExit);
        }
        class Internal : IParserVisitor
        {
            public Internal(SerializerStruct outer, StructBase root)
            {
                m_root = root;
                m_outer = outer;
                ClearStruct(m_root);
                m_stack.Add(new StackEntry(m_root, null));
                m_current = m_stack.Last();
            }
            public void SetExitNotification(VarValueToVariant.FuncExit? funcExit)
            {
                m_funcExit = funcExit;
            }

            void ClearStruct(StructBase stru)
            {
                Type type = stru.GetType();
                StructBase? struEmpty = Activator.CreateInstance(type) as StructBase;
                if (struEmpty != null)
                {
                    PropertyInfo[] properties = type.GetProperties();
                    foreach (var property in properties)
                    {
                        if (property.CanWrite)
                        {
                            property.SetValue(stru, property.GetValue(struEmpty));
                        }
                    }
                }
            }

            public void NotifyError(string str, string message)
            {

            }
            public void StartStruct(MetaStruct stru)
            {
                m_wasStartStructCalled = true;
            }
            public void Finished()
            {

            }

            public void EnterStruct(MetaField field)
            {
                if (field.TypeId != MetaTypeId.TYPE_STRUCT && field.TypeId != MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    return;
                }
                Debug.Assert(m_stack.Count != 0);
                Debug.Assert(m_current != null);

                if (m_wasStartStructCalled && field.TypeName == STR_VARVALUE)
                {
                    m_varValueToVariant = null;
                    Variant? variant = null;
                    StructBase? structBase = m_current.StructBase;
                    if (structBase != null)
                    {
                        PropertyInfo? property = GetProperty(structBase, field.Name);
                        if (property != null && property.PropertyType == typeof(Variant))
                        {
                            variant = (Variant?)property.GetValue(structBase);
                        }
                    }
                    if (variant != null)
                    {
                        m_varValueToVariant = new VarValueToVariant(variant);
                        m_outer.SetVisitor(m_varValueToVariant.GetVisitor());
                        m_varValueToVariant.SetExitNotification(() => {
                            Debug.Assert(m_varValueToVariant != null);
                            m_outer.SetVisitor(this);
                            m_varValueToVariant.Convert();
                            m_varValueToVariant.SetExitNotification(null);
                        });
                    }
                    else
                    {
                        m_current.StructBase = null;
                    }
                }
                else
                {
                    StructBase? sub = null;
                    StructBase? structBase = m_current.StructBase;
                    if (structBase != null)
                    {
                        if (m_current.StructArrayName == null)
                        {
                            PropertyInfo? property = GetProperty(structBase, field.Name);
                            if (property != null)
                            {
                                sub = property.GetValue(structBase) as StructBase;
                                if (sub == null)
                                {
                                    try
                                    {
                                        sub = Activator.CreateInstance(property.PropertyType) as StructBase;
                                    }
                                    catch (Exception)
                                    {
                                    }
                                    property.SetValue(structBase, sub);
                                }
                            }
                        }
                        else
                        {
                            PropertyInfo? property = GetProperty(structBase, m_current.StructArrayName);
                            if (property != null)
                            {
                                System.Collections.IList? list = property.GetValue(structBase) as System.Collections.IList;
                                if (list != null)
                                {
                                    Type[]? genericTypes = property.PropertyType.GenericTypeArguments;
                                    if (genericTypes != null && genericTypes.Length > 0)
                                    {
                                        Type genericType = genericTypes[0];
                                        try
                                        {
                                            sub = Activator.CreateInstance(genericType) as StructBase;
                                        }
                                        catch (Exception)
                                        {
                                        }
                                        if (sub != null)
                                        {
                                            list.Add(sub);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    m_stack.Add(new StackEntry( sub, null ));
                    m_current = m_stack.Last();
                }
            }
            public void ExitStruct(MetaField field)
            {
                if (field.TypeId != MetaTypeId.TYPE_STRUCT && field.TypeId != MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    return;
                }
                if (m_stack.Count != 0)
                {
                    m_stack.RemoveAt(m_stack.Count - 1);
                    if (m_stack.Count != 0)
                    {
                        m_current = m_stack.Last();
                    }
                    else
                    {
                        m_current = null;
                        if (m_funcExit != null)
                        {
                            m_funcExit();
                        }
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

            public void EnterStructNull(MetaField field)
            {
                if (field.TypeId != MetaTypeId.TYPE_STRUCT && field.TypeId != MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    return;
                }
                Debug.Assert(m_stack.Count != 0);
                Debug.Assert(m_current != null);

                StructBase? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = GetProperty(structBase, field.Name);
                    if (property != null && IsNullable(property))
                    {
                        property.SetValue(structBase, null);
                    }
                }
            }

            public void EnterArrayStruct(MetaField field)
            {
                if (field.TypeId != MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    return;
                }
                Debug.Assert(m_current != null);
                m_current.StructArrayName = field.Name;
            }
            public void ExitArrayStruct(MetaField field)
            {
                if (field.TypeId != MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    return;
                }
                Debug.Assert(m_current != null);
                m_current.StructArrayName = null;
            }

            void SetValue<T>(PropertyInfo property, T value)
            {
                Debug.Assert(m_current != null);
                StructBase? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    property.SetValue(structBase, value);
                }
            }

            object? GetValue(PropertyInfo property)
            {
                Debug.Assert(m_current != null);
                StructBase? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    return property.GetValue(structBase);
                }
                return null;
            }

            PropertyInfo? GetProperty(string fieldName)
            {
                Debug.Assert(m_current != null);
                StructBase? structBase = m_current.StructBase;
                return GetProperty(structBase, fieldName);
            }

            PropertyInfo? GetProperty(StructBase? structBase, string fieldName)
            {
                if (structBase != null)
                {
                    Type type = structBase.GetType();
                    PropertyInfo? property = type.GetProperty(fieldName);
                    if (property != null)
                    {
                        return property;
                    }
                    return type.GetProperty(fieldName + '_');   // if c# keyword try with '_'
                }
                return null;
            }

            public dynamic? ConvertNumber(PropertyInfo property, dynamic value)
            {
                Type propertyType = property.PropertyType;
                try
                {
                    if (propertyType == typeof(bool))
                    {
                        return (bool)value;
                    }
                    else if (propertyType == typeof(sbyte))
                    {
                        return (sbyte)value;
                    }
                    else if (propertyType == typeof(byte))
                    {
                        return (byte)value;
                    }
                    else if (propertyType == typeof(short))
                    {
                        return (short)value;
                    }
                    else if (propertyType == typeof(ushort))
                    {
                        return (ushort)value;
                    }
                    else if (propertyType == typeof(int))
                    {
                        return (int)value;
                    }
                    else if (propertyType == typeof(uint))
                    {
                        return (uint)value;
                    }
                    else if (propertyType == typeof(long))
                    {
                        return (long)value;
                    }
                    else if (propertyType == typeof(ulong))
                    {
                        return (ulong)value;
                    }
                    else if (propertyType == typeof(float))
                    {
                        return (float)value;
                    }
                    else if (propertyType == typeof(double))
                    {
                        return (double)value;
                    }
                    else if (propertyType == typeof(string))
                    {
                        return Convertion.Convert<string>(value);
                    }
                    else if (propertyType.IsEnum)
                    {
                        int v = (int)value;
                        return ConvertIntToEnum(propertyType, v);
                    }
                    else if (IsTypeOf(propertyType, typeof(bool[])))
                    {
                        return new bool[] { (bool)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(sbyte[])))
                    {
                        return new sbyte[] { (sbyte)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(byte[])))
                    {
                        return new byte[] { (byte)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(short[])))
                    {
                        return new short[] { (short)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(ushort[])))
                    {
                        return new ushort[] { (ushort)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(int[])))
                    {
                        return new int[] { (int)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(uint[])))
                    {
                        return new uint[] { (uint)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(long[])))
                    {
                        return new long[] { (long)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(ulong[])))
                    {
                        return new ulong[] { (ulong)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(float[])))
                    {
                        return new float[] { (float)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(double[])))
                    {
                        return new double[] { (double)value };
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<string>)))
                    {
                        return new List<string> { Convertion.Convert<string>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<byte[]>)))
                    {
                        return new List<byte[]> { Encoding.UTF8.GetBytes(Convertion.Convert<string>(value)) };
                    }
                    else if (propertyType.IsGenericType && (propertyType.GenericTypeArguments.Length == 1) && propertyType.GenericTypeArguments[0].IsEnum)
                    {
                        Type typeEnum = propertyType.GenericTypeArguments[0];
                        int v = (int)value;
                        dynamic? valueEnum = ConvertIntToEnum(typeEnum, v);
                        if (valueEnum != null)
                        {
                            object? propertyValue = GetValue(property);
                            System.Collections.IList? list = propertyValue as System.Collections.IList;
                            if (list != null)
                            {
                                list.Add(valueEnum);
                            }
                            return list;
                        }
                    }
                }
                catch (Exception)
                {
                }
                return null;
            }

            T[] ConvertArrayToType<T, D>(D[] value)
            {
                Debug.Assert(value != null);
                T[] arr = new T[value.Length];
                int i = 0;
                if (typeof(T) == typeof(bool))
                {
                    foreach (var v in value)
                    {
                        Debug.Assert(v != null);
                        T? val = Convertion.Convert<T>(v);
                        arr[i] = val ?? default(T)!;
                        ++i;
                    }
                }
                else
                {
                    foreach (var v in value)
                    {
                        Debug.Assert(v != null);
                        arr[i] = (T)(dynamic)v;
                        ++i;
                    }
                }
                return arr;
            }
            T[] ConvertArrayToType<T>(IList<string> value)
            {
                T[] arr = new T[value.Count];
                int i = 0;
                foreach (var v in value)
                {
                    T? valueT = Convertion.Convert<T>(v ?? "");
                    Debug.Assert(valueT != null);
                    arr[i] = valueT;
                }
                return arr;
            }
            public dynamic? ConvertArrayNumber<T>(PropertyInfo property, T[] value)
            {
                Type propertyType = property.PropertyType;
                Debug.Assert(value != null);
                dynamic? valueOne = null;
                if (value.Length != 0)
                {
                    valueOne = value[0];
                }
                try
                {
                    if (propertyType == typeof(bool))
                    {
                        if (valueOne != null)
                        {
                            return (bool)valueOne;
                        }
                    }
                    else if (propertyType == typeof(sbyte))
                    {
                        if (valueOne != null)
                        {
                            return (sbyte)valueOne;
                        }
                    }
                    else if (propertyType == typeof(byte))
                    {
                        if (valueOne != null)
                        {
                            return (byte)valueOne;
                        }
                    }
                    else if (propertyType == typeof(short))
                    {
                        if (valueOne != null)
                        {
                            return (short)valueOne;
                        }
                    }
                    else if (propertyType == typeof(ushort))
                    {
                        if (valueOne != null)
                        {
                            return (ushort)valueOne;
                        }
                    }
                    else if (propertyType == typeof(int))
                    {
                        if (valueOne != null)
                        {
                            return (int)valueOne;
                        }
                    }
                    else if (propertyType == typeof(uint))
                    {
                        if (valueOne != null)
                        {
                            return (uint)valueOne;
                        }
                    }
                    else if (propertyType == typeof(long))
                    {
                        if (valueOne != null)
                        {
                            return (long)valueOne;
                        }
                    }
                    else if (propertyType == typeof(ulong))
                    {
                        if (valueOne != null)
                        {
                            return (ulong)valueOne;
                        }
                    }
                    else if (propertyType == typeof(float))
                    {
                        if (valueOne != null)
                        {
                            return (float)valueOne;
                        }
                    }
                    else if (propertyType == typeof(double))
                    {
                        if (valueOne != null)
                        {
                            return (double)valueOne;
                        }
                    }
                    else if (propertyType == typeof(string))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<string>(valueOne);
                        }
                    }
                    else if (propertyType.IsEnum)
                    {
                        if (valueOne != null)
                        {
                            int v = (int)valueOne;
                            return ConvertIntToEnum(propertyType, v);
                        }
                    }
                    else if (IsTypeOf(propertyType, typeof(bool[])))
                    {
                        return ConvertArrayToType<bool, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(sbyte[])))
                    {
                        return ConvertArrayToType<sbyte, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(byte[])))
                    {
                        return ConvertArrayToType<byte, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(short[])))
                    {
                        return ConvertArrayToType<short, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(ushort[])))
                    {
                        return ConvertArrayToType<ushort, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(int[])))
                    {
                        return ConvertArrayToType<int, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(uint[])))
                    {
                        return ConvertArrayToType<uint, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(long[])))
                    {
                        return ConvertArrayToType<long, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(ulong[])))
                    {
                        return ConvertArrayToType<ulong, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(float[])))
                    {
                        return ConvertArrayToType<float, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(double[])))
                    {
                        return ConvertArrayToType<double, T>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<string>)))
                    {
                        IList<string> list = new List<string>();
                        foreach (var v in value)
                        {
                            Debug.Assert(v != null);
                            list.Add(Convertion.Convert<string>(v) ?? "");
                        }
                        return list;
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<byte[]>)))
                    {
                        IList<byte[]> list = new List<byte[]>();
                        foreach (var v in value)
                        {
                            Debug.Assert(v != null);
                            list.Add(Encoding.UTF8.GetBytes(Convertion.Convert<string>(v) ?? ""));
                        }
                        return list;
                    }
                    else if (propertyType.IsGenericType && (propertyType.GenericTypeArguments.Length == 1) && propertyType.GenericTypeArguments[0].IsEnum)
                    {
                        Type typeEnum = propertyType.GenericTypeArguments[0];
                        object? propertyValue = GetValue(property);
                        System.Collections.IList? list = propertyValue as System.Collections.IList;
                        if (list != null)
                        {
                            foreach (var v in value)
                            {
                                Debug.Assert(v != null);
                                dynamic? valueEnum = ConvertIntToEnum(typeEnum, Convertion.Convert<int>(v));
                                list.Add(valueEnum);
                            }
                            return null;
                        }
                    }
                }
                catch (Exception)
                {
                }
                return null;
            }

            dynamic? ConvertString(PropertyInfo property, string value)
            {
                Type propertyType = property.PropertyType;
                try
                {
                    if (propertyType == typeof(bool))
                    {
                        return Convertion.Convert<bool>(value);
                    }
                    else if (propertyType == typeof(sbyte))
                    {
                        return Convertion.Convert<sbyte>(value);
                    }
                    else if (propertyType == typeof(byte))
                    {
                        return Convertion.Convert<byte>(value);
                    }
                    else if (propertyType == typeof(short))
                    {
                        return Convertion.Convert<short>(value);
                    }
                    else if (propertyType == typeof(ushort))
                    {
                        return Convertion.Convert<ushort>(value);
                    }
                    else if (propertyType == typeof(int))
                    {
                        return Convertion.Convert<int>(value);
                    }
                    else if (propertyType == typeof(uint))
                    {
                        return Convertion.Convert<uint>(value);
                    }
                    else if (propertyType == typeof(long))
                    {
                        return Convertion.Convert<long>(value);
                    }
                    else if (propertyType == typeof(ulong))
                    {
                        return Convertion.Convert<ulong>(value);
                    }
                    else if (propertyType == typeof(float))
                    {
                        return Convertion.Convert<float>(value);
                    }
                    else if (propertyType == typeof(double))
                    {
                        return Convertion.Convert<double>(value);
                    }
                    else if (propertyType == typeof(string))
                    {
                        return value;
                    }
                    else if (propertyType.IsEnum)
                    {
                        return ConvertStringToEnum(propertyType, value);
                    }
                    else if (IsTypeOf(propertyType, typeof(bool[])))
                    {
                        return new bool[] { Convertion.Convert<bool>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(sbyte[])))
                    {
                        return new sbyte[] { Convertion.Convert<sbyte>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(byte[])))
                    {
                        return new byte[] { Convertion.Convert<byte>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(short[])))
                    {
                        return new short[] { Convertion.Convert<short>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(ushort[])))
                    {
                        return new ushort[] { Convertion.Convert<ushort>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(int[])))
                    {
                        return new int[] { Convertion.Convert<int>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(uint[])))
                    {
                        return new uint[] { Convertion.Convert<uint>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(long[])))
                    {
                        return new long[] { Convertion.Convert<long>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(ulong[])))
                    {
                        return new ulong[] { Convertion.Convert<ulong>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(float[])))
                    {
                        return new float[] { Convertion.Convert<float>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(double[])))
                    {
                        return new double[] { Convertion.Convert<double>(value) };
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<string>)))
                    {
                        return new List<string> { value };
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<byte[]>)))
                    {
                        return new List<byte[]> { Encoding.UTF8.GetBytes(value) };
                    }
                    else if (propertyType.IsGenericType && (propertyType.GenericTypeArguments.Length == 1) && propertyType.GenericTypeArguments[0].IsEnum)
                    {
                        Type typeEnum = propertyType.GenericTypeArguments[0];
                        dynamic? valueEnum = ConvertStringToEnum(typeEnum, value);
                        if (valueEnum != null)
                        {
                            object? propertyValue = GetValue(property);
                            System.Collections.IList? list = propertyValue as System.Collections.IList;
                            if (list != null)
                            {
                                list.Add(valueEnum);
                            }
                            return null;
                        }
                    }
                }
                catch (Exception)
                {
                }
                return null;
            }

            public dynamic? ConvertArrayString(PropertyInfo property, IList<string> value)
            {
                Type propertyType = property.PropertyType;
                Debug.Assert(value != null);
                string? valueOne = null;
                if (value.Count != 0)
                {
                    valueOne = value[0];
                }
                try
                {
                    if (propertyType == typeof(bool))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<bool>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(sbyte))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<sbyte>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(byte))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<byte>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(short))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<short>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(ushort))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<ushort>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(int))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<int>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(uint))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<uint>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(long))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<long>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(ulong))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<ulong>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(float))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<float>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(double))
                    {
                        if (valueOne != null)
                        {
                            return Convertion.Convert<double>(valueOne);
                        }
                    }
                    else if (propertyType == typeof(string))
                    {
                        if (valueOne != null)
                        {
                            return valueOne;
                        }
                    }
                    else if (propertyType.IsEnum)
                    {
                        if (valueOne != null)
                        {
                            return ConvertStringToEnum(propertyType, valueOne);
                        }
                    }
                    else if (IsTypeOf(propertyType, typeof(bool[])))
                    {
                        return ConvertArrayToType<bool>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(sbyte[])))
                    {
                        return ConvertArrayToType<sbyte>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(byte[])))
                    {
                        return ConvertArrayToType<byte>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(short[])))
                    {
                        return ConvertArrayToType<short>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(ushort[])))
                    {
                        return ConvertArrayToType<ushort>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(int[])))
                    {
                        return ConvertArrayToType<int>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(uint[])))
                    {
                        return ConvertArrayToType<uint>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(long[])))
                    {
                        return ConvertArrayToType<long>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(ulong[])))
                    {
                        return ConvertArrayToType<ulong>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(float[])))
                    {
                        return ConvertArrayToType<float>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(double[])))
                    {
                        return ConvertArrayToType<double>(value);
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<string>)))
                    {
                        return value;
                    }
                    else if (IsTypeOf(propertyType, typeof(IList<byte[]>)))
                    {
                        IList<byte[]> list = new List<byte[]>();
                        foreach (var v in value)
                        {
                            Debug.Assert(v != null);
                            list.Add(Encoding.UTF8.GetBytes(Convertion.Convert<string>(v) ?? ""));
                        }
                        return list;
                    }
                    else if (propertyType.IsGenericType && (propertyType.GenericTypeArguments.Length == 1) && propertyType.GenericTypeArguments[0].IsEnum)
                    {
                        Type typeEnum = propertyType.GenericTypeArguments[0];
                        object? propertyValue = GetValue(property);
                        System.Collections.IList? list = propertyValue as System.Collections.IList;
                        if (list != null)
                        {
                            foreach (var v in value)
                            {
                                Debug.Assert(v != null);
                                dynamic? valueEnum = ConvertStringToEnum(typeEnum, v);
                                list.Add(valueEnum);
                            }
                            return null;
                        }
                    }
                }
                catch (Exception)
                {
                }
                return null;
            }


            void SetValueNumber<T>(string fieldName, T value)
            {
                Debug.Assert(value != null);
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (propertyType == typeof(T))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        dynamic? v = ConvertNumber(property, value);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }

            static bool IsTypeOf(Type type, Type typeOf)
            {
                return type == typeOf || type.IsSubclassOf(typeOf);
            }

            void SetValueArrayNumber<T>(string fieldName, T[] value)
            {
                Debug.Assert(value != null);
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (IsTypeOf(propertyType, typeof(T[])))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        dynamic? v = ConvertArrayNumber(property, value);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }

            void SetValueString(string fieldName, string value)
            {
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (propertyType == typeof(string))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        dynamic? v = ConvertString(property, value);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }

            void SetValueArrayString(string fieldName, IList<string> value)
            {
                Debug.Assert(value != null);
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (IsTypeOf(propertyType, typeof(IList<string>)))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        dynamic? v = ConvertArrayString(property, value);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }

            public void EnterBool(MetaField field, bool value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterInt8(MetaField field, sbyte value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterUInt8(MetaField field, byte value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterInt16(MetaField field, short value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterUInt16(MetaField field, ushort value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterInt32(MetaField field, int value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterUInt32(MetaField field, uint value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterInt64(MetaField field, long value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterUInt64(MetaField field, ulong value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterFloat(MetaField field, float value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterDouble(MetaField field, double value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterString(MetaField field, string value)
            {
                //todo
                SetValueString(field.Name, value);
            }
            public void EnterString(MetaField field, byte[] buffer, int offset, int size)
            {
                SetValueString(field.Name, Encoding.UTF8.GetString(buffer, offset, size));
            }
            public void EnterBytes(MetaField field, byte[] value, int offset, int size)
            {
                Debug.Assert(offset >= 0);
                Debug.Assert(offset + size <= value.Length);
                if (offset != 0 || size != value.Length)
                {
                    byte[] newValue = new byte[size];
                    Array.Copy(value, offset, newValue, 0, size);
                    value = newValue;
                }
                string fieldName = field.Name;
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (propertyType == typeof(byte[]))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        string valueString = Encoding.UTF8.GetString(value);
                        dynamic? v = ConvertString(property, valueString);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }

            private System.Enum? ConvertIntToEnum(Type enumType, int value)
            {
                return TypeRegistry.Instance.IntToEnum(enumType, value);
            }
            private System.Enum? ConvertStringToEnum(Type enumType, string value)
            {
                return TypeRegistry.Instance.StringToEnum(enumType, value);
            }
            public void EnterEnum(MetaField field, int value)
            {
                SetValueNumber(field.Name, value);
            }
            public void EnterEnum(MetaField field, string value)
            {
                SetValueString(field.Name, value);
            }

            public void EnterArrayBool(MetaField field, bool[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayInt8(MetaField field, sbyte[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayInt16(MetaField field, short[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayUInt16(MetaField field, ushort[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayInt32(MetaField field, int[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayUInt32(MetaField field, uint[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayInt64(MetaField field, long[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayUInt64(MetaField field, ulong[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayFloat(MetaField field, float[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayDouble(MetaField field, double[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayString(MetaField field, IList<string> value)
            {
                SetValueArrayString(field.Name, value);
            }
            public void EnterArrayBytes(MetaField field, IList<byte[]> value)
            {
                string fieldName = field.Name;
                Debug.Assert(value != null);
                PropertyInfo? property = GetProperty(fieldName);
                if (property != null)
                {
                    Type propertyType = property.PropertyType;
                    if (IsTypeOf(propertyType, typeof(IList<byte[]>)))
                    {
                        SetValue(property, value);
                    }
                    else
                    {
                        IList<string> listString = new List<string>();
                        foreach (var e in value)
                        {
                            string entryString = Encoding.UTF8.GetString(e);
                            listString.Add(entryString);
                        }
                        dynamic? v = ConvertArrayString(property, listString);
                        if (v != null)
                        {
                            SetValue(property, v);
                        }
                    }
                }
            }
            public void EnterArrayEnum(MetaField field, int[] value)
            {
                SetValueArrayNumber(field.Name, value);
            }
            public void EnterArrayEnum(MetaField field, IList<string> value)
            {
                SetValueArrayString(field.Name, value);
            }

            class StackEntry
            {
                public StackEntry(StructBase? structBase, string? structArrayName)
                {
                    m_structBase = structBase;
                    m_structArrayName = structArrayName;
                }
                public StructBase? StructBase
                {
                    get { return m_structBase; }
                    set { m_structBase = value; }
                }
                public string? StructArrayName
                {
                    get { return m_structArrayName; }
                    set { m_structArrayName = value; }
                }
                StructBase? m_structBase = null;
                string? m_structArrayName;
            };

            readonly StructBase m_root;
            StackEntry? m_current = null;
            readonly IList<StackEntry> m_stack = new List<StackEntry>();
            VarValueToVariant.FuncExit? m_funcExit = null;
            VarValueToVariant? m_varValueToVariant = null;
            bool m_wasStartStructCalled = false;
            readonly SerializerStruct m_outer;
        }

        Internal m_internal;
    }

}
