using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace finalmq
{

    class SerializerStruct : ParserConverter
    {
        static readonly string STR_VARVALUE = "finalmq.variant.VarValue";

        public SerializerStruct(object root)
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
            public Internal(SerializerStruct outer, object root)
            {
                m_root = root;
                m_outer = outer;
                ClearStruct(m_root);
                m_stack.Add(new StackEntry(m_root, -1));
                m_current = m_stack.Last();
            }
            public void SetExitNotification(VarValueToVariant.FuncExit? funcExit)
            {
                m_funcExit = funcExit;
            }

            void ClearStruct(object stru)
            {
                Type type = stru.GetType();
                object? struEmpty = Activator.CreateInstance(type);
                if (struEmpty != null)
                {
                    PropertyInfo[] properties = type.GetProperties();
                    foreach (var property in properties)
                    {
                        Type typeProp = property.PropertyType;
                        property.SetValue(stru, property.GetValue(struEmpty));
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
                Debug.Assert(m_stack.Count != 0);
                Debug.Assert(m_current != null);

                if (m_wasStartStructCalled && field.TypeName == STR_VARVALUE)
                {
                    m_varValueToVariant = null;
                    Variant? variant = null;
                    object? structBase = m_current.StructBase;
                    if (structBase != null)
                    {
                        PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
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
                    object? sub = null;
                    object? structBase = m_current.StructBase;
                    if (structBase != null)
                    {
                        PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
                        if (property != null)
                        {
                            if (m_current.StructArrayIndex == -1)
                            {
                                sub = property.GetValue(structBase);
                            }
                            else
                            {
                                object? arr = property.GetValue(structBase);
                                if (arr == null)
                                {
                                    arr = Activator.CreateInstance(property.PropertyType);
                                    property.SetValue(structBase, arr);
                                }
                                if (arr != null)
                                {
                                    System.Collections.IList list = (System.Collections.IList)arr;
                                    if (list != null)
                                    {
                                        Type[]? genericTypes = property.PropertyType.GenericTypeArguments;
                                        if (genericTypes != null && genericTypes.Length > 0)
                                        {
                                            Type genericType = genericTypes[0];
                                            sub = Activator.CreateInstance(genericType);
                                            list.Add(sub);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    m_stack.Add(new StackEntry( sub, -1 ));
                    m_current = m_stack.Last();
                }
            }
            public void ExitStruct(MetaField field)
            {
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

            public void EnterArrayStruct(MetaField field)
            {
                Debug.Assert(m_current != null);
                m_current.StructArrayIndex = field.Index;
            }
            public void ExitArrayStruct(MetaField field)
            {
                Debug.Assert(m_current != null);
                m_current.StructArrayIndex = -1;
            }

            void SetValue<T>(string fieldName, T value)
            {
                Debug.Assert(m_current != null);
                object? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = structBase.GetType().GetProperty(fieldName);
                    if (property != null)
                    {
                        property.SetValue(structBase, value);
                    }
                }
            }
            public void EnterBool(MetaField field, bool value)
            {
                SetValue(field.Name, value);
            }
            public void EnterInt32(MetaField field, int value)
            {
                SetValue(field.Name, value);
            }
            public void EnterUInt32(MetaField field, uint value)
            {
                SetValue(field.Name, value);
            }
            public void EnterInt64(MetaField field, long value)
            {
                SetValue(field.Name, value);
            }
            public void EnterUInt64(MetaField field, ulong value)
            {
                SetValue(field.Name, value);
            }
            public void EnterFloat(MetaField field, float value)
            {
                SetValue(field.Name, value);
            }
            public void EnterDouble(MetaField field, double value)
            {
                SetValue(field.Name, value);
            }
            public void EnterString(MetaField field, string value)
            {
                SetValue(field.Name, value);
            }
            public void EnterString(MetaField field, byte[] buffer, int offset, int size)
            {
                SetValue(field.Name, Encoding.UTF8.GetString(buffer, offset, size));
            }
            public void EnterBytes(MetaField field, byte[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterEnum(MetaField field, int value)
            {
                Debug.Assert(m_current != null);
                object? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
                    if (property != null)
                    {
                        Type typeEnum = property.PropertyType;
                        if (typeEnum.IsEnum)
                        {
                            System.Array enumValues = System.Enum.GetValues(typeEnum);
                            object? enumValueToSet = null;
                            foreach (var enumEntry in enumValues)
                            {
                                int enumVal = System.Convert.ToInt32(enumEntry);
                                if (enumVal == value)
                                {
                                    enumValueToSet = enumEntry;
                                    break;
                                }
                                if (enumVal == 0)
                                {
                                    enumValueToSet = enumEntry;
                                }
                            }
                            if (enumValueToSet != null)
                            {
                                property.SetValue(structBase, enumValueToSet);
                            }
                        }
                    }
                }
            }
            public void EnterEnum(MetaField field, string value)
            {
                Debug.Assert(m_current != null);
                object? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
                    if (property != null)
                    {
                        Type typeEnum = property.PropertyType;
                        if (typeEnum.IsEnum)
                        {
                            System.Array enumValues = System.Enum.GetValues(typeEnum);
                            object? enumValueToSet = null;
                            foreach (var enumEntry in enumValues)
                            {
                                if (enumEntry.ToString() == value)
                                {
                                    enumValueToSet = enumEntry;
                                    break;
                                }
                                int enumVal = System.Convert.ToInt32(enumEntry);
                                if (enumVal == 0)
                                {
                                    enumValueToSet = enumEntry;
                                }
                            }
                            if (enumValueToSet != null)
                            {
                                property.SetValue(structBase, enumValueToSet);
                            }
                        }
                    }
                }
            }

            public void EnterArrayBool(MetaField field, bool[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayInt32(MetaField field, int[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayUInt32(MetaField field, uint[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayInt64(MetaField field, long[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayUInt64(MetaField field, ulong[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayFloat(MetaField field, float[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayDouble(MetaField field, double[] value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayString(MetaField field, IList<string> value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayBytes(MetaField field, IList<byte[]> value)
            {
                SetValue(field.Name, value);
            }
            public void EnterArrayEnum(MetaField field, int[] value)
            {
                Debug.Assert(m_current != null);
                object? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
                    if (property != null)
                    {
                        Type typeArrayEnum = property.PropertyType;
                        Type[] typesGeneric = typeArrayEnum.GenericTypeArguments;
                        if (typesGeneric.Length > 0)
                        {
                            Type typeEnum = typesGeneric[0];
                            if (typeEnum.IsEnum)
                            {
                                System.Collections.IList? list = Activator.CreateInstance(typeArrayEnum) as Array;
                                if (list != null)
                                {
                                    System.Array enumValues = System.Enum.GetValues(typeEnum);
                                    foreach (var val in value)
                                    {
                                        object? enumValueToSet = null;
                                        foreach (var enumEntry in enumValues)
                                        {
                                            int enumVal = System.Convert.ToInt32(enumEntry);
                                            if (enumVal == val)
                                            {
                                                enumValueToSet = enumEntry;
                                                break;
                                            }
                                            if (enumVal == 0)
                                            {
                                                enumValueToSet = enumEntry;
                                            }
                                        }
                                        if (enumValueToSet != null)
                                        {
                                            list.Add(enumValueToSet);
                                        }
                                    }
                                    property.SetValue(structBase, list);
                                }
                            }
                        }
                    }
                }
            }
            public void EnterArrayEnum(MetaField field, IList<string> value)
            {
                Debug.Assert(m_current != null);
                object? structBase = m_current.StructBase;
                if (structBase != null)
                {
                    PropertyInfo? property = structBase.GetType().GetProperty(field.Name);
                    if (property != null)
                    {
                        Type typeArrayEnum = property.PropertyType;
                        Type[] typesGeneric = typeArrayEnum.GenericTypeArguments;
                        if (typesGeneric.Length > 0)
                        {
                            Type typeEnum = typesGeneric[0];
                            if (typeEnum.IsEnum)
                            {
                                System.Collections.IList? list = Activator.CreateInstance(typeArrayEnum) as Array;
                                if (list != null)
                                {
                                    System.Array enumValues = System.Enum.GetValues(typeEnum);
                                    foreach (var val in value)
                                    {
                                        object? enumValueToSet = null;
                                        foreach (var enumEntry in enumValues)
                                        {
                                            if (enumEntry.ToString() == val)
                                            {
                                                enumValueToSet = enumEntry;
                                                break;
                                            }
                                            int enumVal = System.Convert.ToInt32(enumEntry);
                                            if (enumVal == 0)
                                            {
                                                enumValueToSet = enumEntry;
                                            }
                                        }
                                        if (enumValueToSet != null)
                                        {
                                            list.Add(enumValueToSet);
                                        }
                                    }
                                    property.SetValue(structBase, list);
                                }
                            }
                        }
                    }
                }
            }

            class StackEntry
            {
                public StackEntry(object? structBase, int structArrayIndex)
                {
                    m_structBase = structBase;
                    m_structArrayIndex = structArrayIndex;
                }
                public object? StructBase
                {
                    get { return m_structBase; }
                    set { m_structBase = value; }
                }
                public int StructArrayIndex
                {
                    get { return m_structArrayIndex; }
                    set { m_structArrayIndex = value; }
                }
                object? m_structBase = null;
                int m_structArrayIndex = -1;
            };

            readonly object m_root;
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
