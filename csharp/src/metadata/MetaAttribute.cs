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
        public MetaFieldAttribute(string desc, MetaFieldFlags flags = MetaFieldFlags.METAFLAG_NONE)
        {
            m_desc = desc;
            m_flags = flags;
        }

        public string Desc
        {
            get { return m_desc; }
        }

        public MetaFieldFlags Flags
        {
            get { return m_flags; }
        }

        readonly string m_desc;
        readonly MetaFieldFlags m_flags;
    }

    public class MetaStructAttribute : Attribute
    {
        public MetaStructAttribute(string desc)
        {
            m_desc = desc;
        }

        public string Desc
        {
            get { return m_desc; }
        }

        readonly string m_desc;
    }

    public class MetaEnumAttribute : Attribute
    {
        public MetaEnumAttribute(string desc)
        {
            m_desc = desc;
        }
        public string Desc
        {
            get { return m_desc; }
        }

        readonly string m_desc;
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
