//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.



using System.Reflection;
using System.Diagnostics;

namespace finalmq 
{
    public delegate object FuncStructBaseFactory();

    public interface ITypeRegistry
    {
        void RegisterType(Type type, FuncStructBaseFactory? factory = null);
        object? CreateStruct(string typeName);
    };

    class TypeRegistryImpl : ITypeRegistry
    {
        public void RegisterType(Type type, FuncStructBaseFactory? factory = null)
        {
            if (type.FullName == null)
            {
                return;
            }
            if (factory != null)
            {
                if (!m_factories.ContainsKey(type.FullName))
                {
                    m_factories.Add(type.FullName, factory);
                }
            }

            if (type.IsEnum)
            {
                RegisterEnum(type);
            }
            else
            {
                RegisterStruct(type);
            }

        }
        public object? CreateStruct(string typeName)
        {
            m_factories.TryGetValue(typeName, out var factory);
            return factory;
        }

        private void RegisterEnum(Type type)
        {
            System.Array enumValues = System.Enum.GetValues(type);
            IList<MetaEnumEntry> entries = new List<MetaEnumEntry>();
            foreach (var enumEntry in enumValues)
            {
                string? name = enumEntry.ToString();
                int value = System.Convert.ToInt32(enumEntry);

                object[]? attributesEntry = null;
                var fields = type.GetFields();
                foreach (var field in fields)
                {
                    if (field.Name == name)
                    {
                        attributesEntry = field.GetCustomAttributes(false);
                        break;
                    }
                }
                string descriptionEntry = "";
                string? alias = null;
                if (attributesEntry != null)
                {
                    foreach (var attribute in attributesEntry)
                    {
                        MetaEnumEntryAttribute? attr = attribute as MetaEnumEntryAttribute;
                        if (attr != null)
                        {
                            descriptionEntry = attr.Desc;
                            alias = attr.Alias;
                            break;
                        }
                    }
                }
                if (name != null)
                {
                    MetaEnumEntry entry = new MetaEnumEntry(name, value, descriptionEntry, alias);
                    entries.Add(entry);
                }
            }
            string description = "";
            object[]? attributes = type.GetCustomAttributes(false);
            if (attributes != null)
            {
                foreach (var attribute in attributes)
                {
                    MetaEnumAttribute? attr = attribute as MetaEnumAttribute;
                    if (attr != null)
                    {
                        description = attr.Desc;
                        break;
                    }
                }
            }
            string? fullName = type.FullName;
            if (fullName != null)
            {
                MetaEnum en = new MetaEnum(fullName, description, entries);
                MetaDataGlobal.Instance.AddEnum(en);
            }
        }

        private void RegisterStruct(Type type)
        {
            string description = "";
            object[]? attributes = type.GetCustomAttributes(false);
            if (attributes != null)
            {
                foreach (var attribute in attributes)
                {
                    MetaStructAttribute? attr = attribute as MetaStructAttribute;
                    if (attr != null)
                    {
                        description = attr.Desc;
                        break;
                    }
                }
            }
            IList<MetaField> fields = new List<MetaField>();
            PropertyInfo[] propertyInfos = type.GetProperties();
            foreach (var propertyInfo in propertyInfos)
            {
                string name = propertyInfo.Name;
                Type? propertyType = propertyInfo.PropertyType;
                if (propertyType != null)
                {
                    bool isEnum = propertyType.IsEnum;
                    bool isClass = propertyType.IsClass;
                    Type[]? genericTypes = propertyType.GenericTypeArguments;
                    Type? genericType = null;
                    string? genericFullName = null;
                    bool isGenericEnum = false;
                    bool isGenericClass = false;
                    bool isIList = (propertyType.Name == "IList`1");
                    if (genericTypes != null && genericTypes.Length == 1)
                    {
                        genericType = genericTypes[0];
                        genericFullName = genericType.FullName;
                        isGenericEnum = genericType.IsEnum;
                        isGenericClass = genericType.IsClass;
                    }

                    object[]? attributesProperty = propertyType.GetCustomAttributes(false);
                    string descriptionProperty = "";
                    MetaFieldFlags flags = MetaFieldFlags.METAFLAG_NONE;
                    if (attributesProperty != null)
                    {
                        foreach (var attribute in attributesProperty)
                        {
                            MetaFieldAttribute? attr = attribute as MetaFieldAttribute;
                            if (attr != null)
                            {
                                descriptionProperty = attr.Desc;
                                flags = attr.Flags;
                                break;
                            }
                        }
                    }

                    string? fullNameProperty = propertyInfo.PropertyType?.FullName;
                    if (fullNameProperty != null)
                    {
                        string typeName;
                        MetaTypeId typeId = TypeName2Id(fullNameProperty, isClass, isEnum, isIList, genericFullName, isGenericClass, isGenericEnum, out typeName);
                        MetaField field = new MetaField(typeId, typeName, name, description, (int)flags);
                        fields.Add(field);
                    }
                }
            }
            string? fullName = type.FullName;
            if (fullName != null)
            {
                MetaStruct stru = new MetaStruct(fullName, description, fields);
                MetaDataGlobal.Instance.AddStruct(stru);
            }
        }

        private static MetaTypeId TypeName2Id(string fullName, bool isClass, bool isEnum, bool isIList, string? genericFullName, bool isGenericClass, bool isGenericEnum, out string typeName)
        {
            typeName = "";
            if (isEnum)
            {
                typeName = fullName;
                return MetaTypeId.TYPE_ENUM;
            }
            if (isIList)
            {
                Debug.Assert(genericFullName != null);
                if (isGenericEnum)
                {
                    typeName = genericFullName;
                    return MetaTypeId.TYPE_ARRAY_ENUM;
                }
                else if (genericFullName == "System.String")
                {
                    return MetaTypeId.TYPE_ARRAY_STRING;
                }
                else if (genericFullName == "System.Byte[]")
                {
                    return MetaTypeId.TYPE_ARRAY_BYTES;
                }
                else if (isGenericClass)
                {
                    typeName = genericFullName;
                    return MetaTypeId.TYPE_ARRAY_STRUCT;
                }
                return MetaTypeId.TYPE_NONE;
            }
            switch (fullName)
            {
                case "System.Boolean":
                    return MetaTypeId.TYPE_BOOL;
                case "System.Int32":
                    return MetaTypeId.TYPE_INT32;
                case "System.UInt32":
                    return MetaTypeId.TYPE_UINT32;
                case "System.Int64":
                    return MetaTypeId.TYPE_INT64;
                case "System.UInt64":
                    return MetaTypeId.TYPE_UINT64;
                case "System.Single":
                    return MetaTypeId.TYPE_FLOAT;
                case "System.Double":
                    return MetaTypeId.TYPE_DOUBLE;
                case "System.String":
                    return MetaTypeId.TYPE_STRING;
                case "System.Byte[]":
                    return MetaTypeId.TYPE_BYTES;
                case "finalmq.Variant":
                    typeName = "finalmq.variant.VarValue";
                    return MetaTypeId.TYPE_STRUCT;
                case "System.Boolean[]":
                    return MetaTypeId.TYPE_ARRAY_BOOL;
                case "System.Int32[]":
                    return MetaTypeId.TYPE_ARRAY_INT32;
                case "System.UInt32[]":
                    return MetaTypeId.TYPE_ARRAY_UINT32;
                case "System.Int64[]":
                    return MetaTypeId.TYPE_ARRAY_INT64;
                case "System.UInt64[]":
                    return MetaTypeId.TYPE_ARRAY_UINT64;
                case "System.Single[]":
                    return MetaTypeId.TYPE_ARRAY_FLOAT;
                case "System.Double[]":
                    return MetaTypeId.TYPE_ARRAY_DOUBLE;
                default:
                    if (isClass)
                    {
                        typeName = fullName;
                        return MetaTypeId.TYPE_STRUCT;
                    }
                    return MetaTypeId.TYPE_NONE;
            }
        }

        IDictionary<string, FuncStructBaseFactory> m_factories = new Dictionary<string, FuncStructBaseFactory>();
    };


    public class TypeRegistry
    {
        public static ITypeRegistry Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }

        private TypeRegistry()
        {
        }

        private static ITypeRegistry m_instance = new TypeRegistryImpl();
    }


}   // namespace finalmq
