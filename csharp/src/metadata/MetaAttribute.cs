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

    public class MetaFieldAttribute : Attribute
    {
        public MetaFieldAttribute(string desc, MetaFieldFlags flags = MetaFieldFlags.METAFLAG_NONE, string[]? attrs = null)
        {
            m_desc = desc;
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
        }

        public string Desc
        {
            get { return m_desc; }
        }

        public MetaFieldFlags Flags
        {
            get { return m_flags; }
        }

        public string[] Attributes
        {
            get { return m_attrs; }
        }

        public IDictionary<string, string> Properties
        {
            get { return m_properties; }
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

        readonly string m_desc;
        readonly MetaFieldFlags m_flags;
        readonly string[] m_attrs;
        readonly IDictionary<string, string> m_properties;
    }

    public class MetaStructAttribute : Attribute
    {
        public MetaStructAttribute(string desc, MetaStructFlags flags = MetaStructFlags.METASTRUCTFLAG_NONE, string[]? attrs = null)
        {
            m_desc = desc;
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
        }

        public string Desc
        {
            get { return m_desc; }
        }

        public MetaStructFlags Flags
        {
            get { return m_flags; }
        }

        public string[] Attributes
        {
            get { return m_attrs; }
        }

        public IDictionary<string, string> Properties
        {
            get { return m_properties; }
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

        readonly string m_desc;
        readonly MetaStructFlags m_flags;
        readonly string[] m_attrs;
        readonly IDictionary<string, string> m_properties;
    }

    public class MetaEnumAttribute : Attribute
    {
        public MetaEnumAttribute(string desc, string[]? attrs = null)
        {
            m_desc = desc;
            if (attrs != null)
            {
                m_attrs = attrs;
            }
            else
            {
                m_attrs = new string[0];
            }
        }
        public string Desc
        {
            get { return m_desc; }
        }
        public string[] Attributes
        {
            get { return m_attrs; }
        }

        readonly string m_desc;
        readonly string[] m_attrs;
    }

    public class MetaEnumEntryAttribute : Attribute
    {
        public MetaEnumEntryAttribute(string desc, string? alias = null)
        {
            m_desc = desc;
            m_alias = alias;
        }
        public string Desc
        {
            get { return m_desc; }
        }

        public string? Alias
        {
            get { return m_alias; }
        }

        readonly string m_desc;
        readonly string? m_alias;
    }

}   // namespace finalmq
