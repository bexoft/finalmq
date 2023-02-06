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
    public delegate StructBase FuncStructBaseFactory();

    public interface ITypeRegistry
    {
        void RegisterStruct(Type type, MetaStruct metaStruct, FuncStructBaseFactory factory);
        void RegisterEnum(Type type, MetaEnum metaEnum);
        StructBase? CreateStruct(string typeName);

        System.Enum? IntToEnum(Type type, int value);
        System.Enum? StringToEnum(Type type, string value);
    };

    class TypeRegistryImpl : ITypeRegistry
    {
        public void RegisterStruct(Type type, MetaStruct metaStruct, FuncStructBaseFactory factory)
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

            MetaDataGlobal.Instance.AddStruct(metaStruct);
        }

        public void RegisterEnum(Type type, MetaEnum metaEnum)
        {
            EnumData enumData = new EnumData(type, metaEnum);
            m_enumData.Add(type, enumData);
            MetaDataGlobal.Instance.AddEnum(metaEnum);
        }

        public StructBase? CreateStruct(string typeName)
        {
            if (m_factories.TryGetValue(typeName, out var factory))
            {
                return factory!();
            }
            return null;
        }

        public System.Enum? IntToEnum(Type type, int value)
        {
            if (m_enumData.TryGetValue(type, out var enumData))
            {
                return enumData.IntToEnum(type, value);
            }
            return null;
        }
        public System.Enum? StringToEnum(Type type, string value)
        {
            if (m_enumData.TryGetValue(type, out var enumData))
            {
                return enumData.StringToEnum(type, value);
            }
            return null;
        }

        readonly IDictionary<string, FuncStructBaseFactory> m_factories = new Dictionary<string, FuncStructBaseFactory>();
        readonly IDictionary<Type, EnumData> m_enumData = new Dictionary<Type, EnumData>();
    };

    class EnumData
    {
        public EnumData(Type type, MetaEnum metaEnum)
        {
            System.Array enumValues = System.Enum.GetValues(type);
            foreach (System.Enum enumEntry in enumValues)
            {
                int enumVal = System.Convert.ToInt32(enumEntry);
                string enumString = enumEntry.ToString()!;
                string alias = metaEnum.GetAliasByValue(enumVal);
                m_intToEnum.Add(enumVal, enumEntry);
                m_stringToEnum.Add(enumString, enumEntry);
                if (alias.Length != 0)
                {
                    m_aliasToEnum.Add(alias, enumEntry);
                }
            }
        }
        public System.Enum? IntToEnum(Type type, int value)
        {
            if (m_intToEnum.TryGetValue(value, out var en1))
            {
                return en1;
            }
            if (m_intToEnum.TryGetValue(0, out var en2))
            {
                return en2;
            }
            return null;
        }
        public System.Enum? StringToEnum(Type type, string value)
        {
            if (m_stringToEnum.TryGetValue(value, out var en1))
            {
                return en1;
            }
            if (m_aliasToEnum.TryGetValue(value, out var en2))
            {
                return en2;
            }
            if (m_intToEnum.TryGetValue(0, out var en3))
            {
                return en3;
            }
            return null;
        }

        IDictionary<int, System.Enum> m_intToEnum = new Dictionary<int, System.Enum>();
        IDictionary<string, System.Enum> m_stringToEnum = new Dictionary<string, System.Enum>();
        IDictionary<string, System.Enum> m_aliasToEnum = new Dictionary<string, System.Enum>();
    }


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
