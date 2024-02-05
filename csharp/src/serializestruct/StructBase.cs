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

    public abstract class StructBase
    {
        public abstract MetaStruct MetaStruct { get; }

        public static MetaEnum CreateMetaEnum(Type type)
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
            string[]? attributesEnum = null;
            object[]? attributes = type.GetCustomAttributes(false);
            if (attributes != null)
            {
                foreach (var attribute in attributes)
                {
                    MetaEnumAttribute? attr = attribute as MetaEnumAttribute;
                    if (attr != null)
                    {
                        description = attr.Desc;
                        attributesEnum = attr.Attributes;
                        break;
                    }
                }
            }
            MetaEnum en = new MetaEnum(type.FullName!, description, attributesEnum, entries);
            return en;
        }


        public static MetaStruct CreateMetaStruct(Type type)
        {
            string description = "";
            object[]? attributes = type.GetCustomAttributes(false);
            MetaStructFlags flagsStruct = MetaStructFlags.METASTRUCTFLAG_NONE;
            string[]? attrsStruct = null;
            if (attributes != null)
            {
                foreach (var attribute in attributes)
                {
                    MetaStructAttribute? attr = attribute as MetaStructAttribute;
                    if (attr != null)
                    {
                        description = attr.Desc;
                        flagsStruct = attr.Flags;
                        attrsStruct = attr.Attributes;
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
                    if (propertyType.FullName == "finalmq.MetaStruct")  // skip property "public override finalmq.MetaStruct MetaStruct"
                    {
                        continue;
                    }
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

                    object[]? attributesProperty = propertyInfo.GetCustomAttributes(false);
                    string descriptionProperty = "";
                    MetaFieldFlags flags = MetaFieldFlags.METAFLAG_NONE;
                    string[]? attrs = null;
                    if (attributesProperty != null)
                    {
                        foreach (var attribute in attributesProperty)
                        {
                            MetaFieldAttribute? attr = attribute as MetaFieldAttribute;
                            if (attr != null)
                            {
                                descriptionProperty = attr.Desc;
                                flags = attr.Flags;
                                attrs = attr.Attributes;
                                break;
                            }
                        }
                    }

                    string? fullNameProperty = propertyInfo.PropertyType?.FullName;
                    if (fullNameProperty != null)
                    {
                        string typeName;
                        MetaTypeId typeId = TypeName2Id(fullNameProperty, isClass, isEnum, isIList, genericFullName, isGenericClass, isGenericEnum, out typeName);
                        MetaField field = new MetaField(typeId, typeName, name, description, (int)flags, attrs);
                        fields.Add(field);
                    }
                }
            }
            MetaStruct stru = new MetaStruct(type.FullName!, description, fields, (int)flagsStruct, attrsStruct);
            return stru;
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
                case "System.SByte":
                    return MetaTypeId.TYPE_INT8;
                case "System.Byte":
                    return MetaTypeId.TYPE_UINT8;
                case "System.Int16":
                    return MetaTypeId.TYPE_INT16;
                case "System.UInt16":
                    return MetaTypeId.TYPE_UINT16;
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
                case "System.SByte[]":
                    return MetaTypeId.TYPE_ARRAY_INT8;
                case "System.Int16[]":
                    return MetaTypeId.TYPE_ARRAY_INT16;
                case "System.UInt16[]":
                    return MetaTypeId.TYPE_ARRAY_UINT16;
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

        class RawDataStruct
        {
            public RawDataStruct(string type, int contentType, BufferRef data)
            {
                m_type = type;
                m_contentType = contentType;
                m_data = data;
            }
            public string Type { get { return m_type; } }
            public int ContentType { get { return m_contentType; } }
            public BufferRef Data { get { return m_data; } }

            string m_type;
            int m_contentType;
            BufferRef m_data;
        }

        public void SetRawData(string type, int contentType, BufferRef rawData)
        {
            if (m_rawData != null)
            {
                m_rawData = new RawDataStruct(type, contentType, rawData);
            }
        }
        public string? GetRawType()
        {
            return m_rawData?.Type;
        }
        public int GetRawContentType()
        {
            if (m_rawData != null)
            {
                return m_rawData.ContentType;
            }
            return 0;
        }
        public BufferRef? GetRawData()
        {
            return m_rawData?.Data;
        }

        RawDataStruct? m_rawData = null;
    }


}   // namespace finalmq
