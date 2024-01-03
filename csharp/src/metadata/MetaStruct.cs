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


namespace finalmq
{

    public enum MetaStructFlags
    {
        METASTRUCTFLAG_NONE = 0,
        METASTRUCTFLAG_HL7_SEGMENT = 1,
        METASTRUCTFLAG_CHOICE = 2,          // at most one of the nullable fields is not null
    };


    public class MetaStruct
    {
        public MetaStruct(string typeName, string description, IList<MetaField> fields, int flags = 0, string[]? attrs = null)
        {
            m_typeName = typeName;
            m_typeNameWithoutNamespace = RemoveNamespace(typeName);
            m_description = description;
            m_flags = flags;

            if (attrs != null)
            {
                m_attrs = attrs;
            }
            else
            {
                m_attrs = new string[0];
            }

            m_properties = generateProperties(m_attrs);

            foreach (var field in fields)
            {
                AddField(field);
            }
        }

        public string TypeName
        {
            get { return m_typeName; }
        }
        public string TypeNameWithoutNamespace
        {
            get { return m_typeNameWithoutNamespace; }
        }
        public string Description
        {
            get { return m_description; }
        }
        public MetaField? GetFieldByIndex(int index)
        {
            if (index >= 0 && index < m_fields.Count)
            {
                return m_fields[index];
            }
            return null;
        }

        public MetaField? GetFieldByName(string name)
        {
            m_name2Field.TryGetValue(name, out var field);
            return field;
        }

        public void AddField(MetaField field)
        {
            if (m_name2Field.ContainsKey(field.Name))
            {
                // field already added
                return;
            }

            field.Index = m_fields.Count;

            MetaField f = field;
            m_fields.Add(f);
            m_name2Field.Add(f.Name, f);
        }
        public int FieldsSize
        {
            get { return m_fields.Count; }
        }

        public int Flags
        {
            get { return m_flags; }
        }

        public string[] Attributes { get { return m_attrs; } }
        public IDictionary<string, string> Properties { get { return m_properties; } }

        static string RemoveNamespace(string typeName)
        {
            int pos = typeName.LastIndexOf('.') + 1;
            return typeName.Substring(pos, typeName.Length - pos);
        }

        private static IDictionary<string, string> generateProperties(string[] attrs)
        {
            IDictionary<string, string> properties = new Dictionary<string, string>();
            foreach (string attr in attrs)
            {
                string[] props = attr.Split(',');
                foreach (string prop in props)
                {
                    int ix = prop.IndexOf(':');
                    if (ix != -1)
                    {
                        string key = prop.Substring(0, ix);
                        string value = prop.Substring(ix + 1);
                        properties[key] = value;
                    }
                    else
                    {
                        properties[prop] = "";
                    }
                }
            }
            return properties;
        }

        readonly string m_typeName;
        readonly string m_typeNameWithoutNamespace;
        readonly string m_description;
        readonly IList<MetaField> m_fields = new List<MetaField>();
        readonly int m_flags;
        readonly string[] m_attrs;
        readonly IDictionary<string, string> m_properties;
        readonly IDictionary<string, MetaField> m_name2Field = new Dictionary<string, MetaField>();
    }

}   // namespace finalmq
