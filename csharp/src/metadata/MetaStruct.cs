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
    public class MetaStruct
    {
        public MetaStruct(string typeName, string description, IList<MetaField> fields)
        {
            m_typeName = typeName;
            m_description = description;
            foreach (var field in fields)
            {
                AddField(field);
            }
        }

        public string TypeName
        {
            get { return m_typeName; }
            set { m_typeName = value; }
        }
        public string Description
        {
            get { return m_description; }
            set { m_description = value; }
        }
        public MetaField GetFieldByIndex(int index)
        {
            return m_fields[index];
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

        string m_typeName;
        string m_description;
        readonly IList<MetaField>                 m_fields = new List<MetaField>();
        readonly IDictionary<string, MetaField>   m_name2Field = new Dictionary<string, MetaField>();
    }

}   // namespace finalmq
