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
    public class MetaEnumEntry
    {
        public MetaEnumEntry(string name, int id, string description, string? alias)
        {
            m_name = name;
            m_id = id;
            m_description = description;
            m_alias = alias;
        }

        public string Name { get { return m_name; } }
        public int Id { get { return m_id; } }
        public string Description { get { return m_description; } }
        public string? Alias { get { return m_alias; } }
        
        readonly string m_name;
        readonly int m_id = 0;
        readonly string m_description;
        readonly string? m_alias;
    };

    public class MetaEnum
    {
        public MetaEnum(string typeName, string description, IList<MetaEnumEntry> entries)
        {
            m_typeName = typeName;
            m_description = description;
            foreach (var entry in entries)
            {
                AddEntry(entry);
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

        public MetaEnumEntry? GetEntryById(int id)
        {
            m_id2Entry.TryGetValue(id, out var entry);
            return entry;
        }
        public MetaEnumEntry? GetEntryByName(string name)
        {
            m_name2Entry.TryGetValue(name, out var entry);
            if (entry != null)
            {
                return entry;
            }
            m_alias2Entry.TryGetValue(name, out entry);
            return entry;
        }
        public MetaEnumEntry? GetEntryByIndex(int index)
        {
            if (0 < index && index < m_entries.Count)
            {
                return m_entries[index];
            }
            return null;
        }
        public bool IsId(int id)
        {
            return m_id2Entry.ContainsKey(id);
        }
        public int GetValueByName(string name)
        {
            int value = 0;
            MetaEnumEntry? entry = GetEntryByName(name);
            if (entry != null)
            {
                value = entry.Id;
            }
            return value;
        }
        public string GetNameByValue(int value)
        {
            MetaEnumEntry? entry = GetEntryById(value);
            if (entry != null)
            {
                return entry.Name;
            }
            else
            {
                entry = GetEntryById(0);
                if (entry != null)
                {
                    return entry.Name;
                }
            }
            return "";
        }

        static readonly string aliasEmpty = "$empty";

        public string GetAliasByValue(int value)
        {
            MetaEnumEntry? entry = GetEntryById(value);
            if (entry != null)
            {
                string? alias = entry.Alias;
                if (alias?.Length != 0)
                {
                    Debug.Assert(alias != null);
                    if (alias != aliasEmpty)
                    {
                        return alias;
                    }
                    return "";
                }
                return entry.Name;
            }
            else
            {
                entry = GetEntryById(0);
                if (entry != null)
                {
                    string? alias = entry.Alias;
                    if (alias?.Length != 0)
                    {
                        Debug.Assert(alias != null);
                        if (alias != aliasEmpty)
                        {
                            return alias;
                        }
                        return "";
                    }
                    return entry.Name;
                }
            }
            return "Unknown enum value (" + value + ") of " + m_typeName + ", default enum is also not defined";
        }

        public void AddEntry(MetaEnumEntry entry)
        {
            if (m_id2Entry.ContainsKey(entry.Id))
            {
                // entry already added
                return;
            }

            m_entries.Add(entry);
            m_id2Entry.Add(entry.Id, entry);
            m_name2Entry.Add(entry.Name, entry);
            if (entry.Alias != null && entry.Alias != "")
            {
                m_alias2Entry.Add(entry.Alias, entry);
            }
        }

        public int EntrySize
        {
            get { return m_entries.Count; }
        }

        string m_typeName;
        string m_description;
        readonly IList<MetaEnumEntry> m_entries = new List<MetaEnumEntry>();
        readonly IDictionary<int, MetaEnumEntry> m_id2Entry = new Dictionary<int, MetaEnumEntry>();
        readonly IDictionary<string, MetaEnumEntry> m_name2Entry = new Dictionary<string, MetaEnumEntry>();
        readonly IDictionary<string, MetaEnumEntry> m_alias2Entry = new Dictionary<string, MetaEnumEntry>();
    }

}   // namespace finalmq
