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

    public enum MetaFieldFlags
    {
        METAFLAG_NONE = 0,
        METAFLAG_PROTO_VARINT = 1,
        METAFLAG_PROTO_ZIGZAG = 2,
        METAFLAG_NULLABLE = 4,
    };

    public class MetaField
    {
        public MetaField(MetaTypeId typeId, string typeName, string name, string description, int flags = 0, int index = -1)
        {
            m_typeId = typeId;
            m_typeName = typeName;
            m_name = name;
            m_description = description;
            m_flags = flags;
            m_index = index;
        }

        public MetaTypeId TypeId { get { return m_typeId; } }
        public string TypeName { get { return m_typeName; } }
        public string Name { get { return m_name; } }
        public string Description { get { return m_description; } }
        public int Flags { get { return m_flags; } }
        public int Index {
            get { return m_index; }
            set { m_index = value; }
        }
        public MetaEnum? MetaEnum 
        {
            get { return m_metaEnum; }
            set { m_metaEnum = value; }
        }
        public MetaStruct? MetaStruct 
        {
            get { return m_metaStruct; }
            set { m_metaStruct = value; }
        }
        public MetaField? FieldWithoutArray
        {
            get { return m_fieldWithoutArray; }
            set { m_fieldWithoutArray = value; }
        }

        readonly MetaTypeId m_typeId;           ///< type id of the parameter
        readonly string m_typeName;             ///< is needed for struct and enum
        readonly string m_name;                 ///< parameter name
        readonly string m_description;          ///< description of the parameter
        readonly int m_flags;                   ///< flaggs of the parameter
        int m_index;                            ///< index of field inside struct

        MetaEnum? m_metaEnum = null;              ///< cache to find MetaEnum of typeName faster
        MetaStruct? m_metaStruct = null;          ///< cache to find MetaStruct of typeName faster
        MetaField? m_fieldWithoutArray = null;    ///< in case of an array, this is the MetaField for its entries
    };


}   // namespace finalmq
