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

using System.Diagnostics;

namespace finalmq
{
    public interface IMetaData
    {
        MetaStruct? GetStruct(string typeName);
        MetaEnum? GetEnum(string typeName);
        MetaStruct? GetStruct(MetaField field);
        MetaField? GetField(string typeName, string fieldName);
        MetaEnum? GetEnum(MetaField field);
        MetaField? GetArrayField(MetaField field);
        MetaField? GetArrayField(string typeName, string fieldName);
        bool isEnumValue(MetaField field, int value);
        int GetEnumValueByName(MetaField field, string name);
        string GetEnumNameByValue(MetaField field, int value);
        string GetEnumAliasByValue(MetaField field, int value);
        MetaStruct AddStruct(MetaStruct stru);
        MetaEnum AddEnum(MetaEnum en);
        IDictionary<string, MetaStruct> AllStructs { get; }
        IDictionary<string, MetaEnum> AllEnums { get; }
    };

    public class MetaData : IMetaData
    {

        // IMetaData
        public MetaStruct? GetStruct(string typeName)
        {
            lock (m_mutex)
            {
                m_name2Struct.TryGetValue(typeName, out var stru);
                if (stru != null)
                {
                    return stru;
                }
                return null;
            }
        }
        public MetaEnum? GetEnum(string typeName)
        {
            lock (m_mutex)
            {
                m_name2Enum.TryGetValue(typeName, out var en);
                if (en != null)
                {
                    return en;
                }
                return null;
            }
        }
        public MetaStruct? GetStruct(MetaField field)
        {
            Debug.Assert(field.TypeId == MetaTypeId.TYPE_STRUCT || field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT);
            if (field.MetaStruct == null)
            {
                MetaStruct? stru = GetStruct(field.TypeName);
                field.MetaStruct = stru;
                if (field.MetaStruct == null)
                {
                    // struct not found
//todo                    streamError << "struct not found: " << field.typeName;
                }
            }
            return field.MetaStruct;
        }
        public MetaField? GetField(string typeName, string fieldName)
        {
            MetaStruct? stru = GetStruct(typeName);
            if (stru != null)
            {
                MetaField? fieldValue = stru.GetFieldByName(fieldName);
                return fieldValue;
            }
            return null;
        }
        public MetaEnum? GetEnum(MetaField field)
        {
            Debug.Assert(field.TypeId == MetaTypeId.TYPE_ENUM || field.TypeId == MetaTypeId.TYPE_ARRAY_ENUM);
            if (field.MetaEnum == null)
            {
                MetaEnum? en = GetEnum(field.TypeName);
                field.MetaEnum = en;
                if (field.MetaEnum != null)
                {
                    // enum not found
//todo                    streamError << "enum not found: " << field.typeName;
                }
            }
            return field.MetaEnum;
        }
        public MetaField? GetArrayField(MetaField field)
        {
            Debug.Assert((field.TypeId & MetaTypeId.OFFSET_ARRAY_FLAG) != 0);
            if (field.FieldWithoutArray == null)
            {
                MetaTypeId typeId = field.TypeId & ~MetaTypeId.OFFSET_ARRAY_FLAG;
                field.FieldWithoutArray = new MetaField(typeId, field.TypeName, "", field.Description, field.Flags, field.Index);
            }
            return field.FieldWithoutArray;
        }
        public MetaField? GetArrayField(string typeName, string fieldName)
        {
            MetaField? field = GetField(typeName, fieldName);
            if (field != null)
            {
                return GetArrayField(field);
            }
            return null;
        }
        public bool isEnumValue(MetaField field, int value)
        {
            MetaEnum? en = GetEnum(field);
            if (en != null)
            {
                return en.IsId(value);
            }
            return false;
        }
        public int GetEnumValueByName(MetaField field, string name)
        {
            int value = 0;
            MetaEnum? en = GetEnum(field);
            if (en != null)
            {
                value = en.GetValueByName(name);
            }
            return value;
        }
        public string GetEnumNameByValue(MetaField field, int value)
        {
            MetaEnum? en = GetEnum(field);
            if (en != null)
            {
                return en.GetNameByValue(value);
            }
            return "";
        }
        public string GetEnumAliasByValue(MetaField field, int value)
        {
            MetaEnum? en = GetEnum(field);
            if (en != null)
            {
                return en.GetAliasByValue(value);
            }
            return "";
        }
        public MetaStruct AddStruct(MetaStruct stru)
        {
            lock (m_mutex)
            {
                m_name2Struct.TryGetValue(stru.TypeName, out var st);
                if (st != null)
                {
                    // struct already added
                    return st;
                }

                string typeName = stru.TypeName;
                m_name2Struct.Add(typeName, stru);
                return stru;
            }
        }
        public MetaEnum AddEnum(MetaEnum en)
        {
            lock (m_mutex)
            {
                m_name2Enum.TryGetValue(en.TypeName, out var e);
                if (e != null)
                {
                    // enum already added
                    return e;
                }

                m_name2Enum.Add(en.TypeName, en);
                return en;
            }
        }
        public IDictionary<string, MetaStruct> AllStructs 
        { 
            get 
            {
                lock (m_mutex)
                {
                    return m_name2Struct;
                }
            }
        }
        public IDictionary<string, MetaEnum> AllEnums
        {
            get
            {
                lock (m_mutex)
                {
                    return m_name2Enum;
                }
            }
        }

        readonly IDictionary<string, MetaStruct> m_name2Struct = new Dictionary<string, MetaStruct>();
        readonly IDictionary<string, MetaEnum> m_name2Enum = new Dictionary<string, MetaEnum>();
        readonly object m_mutex = new object();
    }

    public class MetaDataGlobal
    {
        public static IMetaData Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }

        private static IMetaData m_instance = new MetaData();
    };


}   // namespace finalmq
