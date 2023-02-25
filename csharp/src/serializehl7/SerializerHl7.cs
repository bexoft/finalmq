using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{
    
    public class SerializerHl7 : ParserConverter
    {
        static readonly int NO_ARRAY_STRUCT = -2;

        public SerializerHl7(IZeroCopyBuffer buffer, int maxBlockSize = 512, bool enumAsString = true, bool skipDefaultValues = true)
        {
            m_internal = new Internal(buffer, maxBlockSize, enumAsString);
            m_parserProcessDefaultValues = new ParserProcessDefaultValues(skipDefaultValues, m_internal);
            SetVisitor(m_parserProcessDefaultValues);
        }

        class Internal : IParserVisitor
        {
            public Internal(IZeroCopyBuffer buffer, int maxBlockSize, bool enumAsString)
            {
                m_hl7Builder = new Hl7Builder(buffer, maxBlockSize);
                m_enumAsString = enumAsString;
            }
            // IParserVisitor

            public void NotifyError(string str, string message) {}
            public void StartStruct(MetaStruct stru) 
            {
                string messageStructure = stru.TypeNameWithoutNamespace;
                string[] splitString = messageStructure.Split('_');

                ++m_ixIndex;
                Debug.Assert(m_ixIndex < m_indexOfLayer.Length);
                m_indexOfLayer[m_ixIndex] = -1;

                int[] indexMessageType = { 0, 8, 0 };
                if (splitString.Length >= 1)
                {
                    m_hl7Builder.EnterString(indexMessageType, 3, 0, splitString[0]);
                }
                if (splitString.Length >= 2)
                {
                    m_hl7Builder.EnterString(indexMessageType, 3, 1, splitString[1]);
                }
                m_hl7Builder.EnterString(indexMessageType, 3, 2, messageStructure);
            }
            public void Finished() 
            {
                m_hl7Builder.Finished();
            }

            public void EnterStruct(MetaField field) 
            {
                MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
                bool isSegment = ((stru != null) && ((stru.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0));

                if (m_inSegment || isSegment)
                {
                    if (isSegment)
                    {
                        Debug.Assert(m_ixIndex == 0);
                        ++m_indexOfLayer[m_ixIndex];
                        string typeNameWithoutNamespace = stru.TypeNameWithoutNamespace;
                        m_hl7Builder.EnterString(m_indexOfLayer, m_ixIndex + 1, -1, typeNameWithoutNamespace);
                        m_inSegment = true;
                    }
                    else
                    {
                        Debug.Assert(m_ixIndex < m_indexOfLayer.Length);
                        // at index 2 there is the array index
                        if (m_ixIndex != 1)
                        {
                            ++m_ixIndex;
                            m_indexOfLayer[m_ixIndex] = field.Index;
                        }
                        if (m_ixIndex == 1)
                        {
                            if (m_ixArrayStruct == NO_ARRAY_STRUCT)
                            {
                                ++m_ixIndex;
                                m_indexOfLayer[m_ixIndex] = 0;
                            }
                            else
                            {
                                ++m_ixArrayStruct;
                                ++m_ixIndex;
                                m_indexOfLayer[m_ixIndex] = m_ixArrayStruct;
                            }
                        }
                    }
                }
            }
            public void ExitStruct(MetaField field) 
            {
                if (m_ixIndex > 0)
                {
                    if (m_ixIndex == 2 && m_ixArrayStruct == NO_ARRAY_STRUCT)
                    {
                        --m_ixIndex;
                    }
                    --m_ixIndex;
                    Debug.Assert(m_ixIndex >= 0);
                }
                else
                {
                    m_inSegment = false;
                }
            }
            public void EnterStructNull(MetaField field)
            {
            }

            public void EnterArrayStruct(MetaField field) 
            {
                if (m_inSegment)
                {
                    m_ixArrayStruct = -1;
                    Debug.Assert(m_ixIndex == 0);
                    ++m_ixIndex;
                    m_indexOfLayer[m_ixIndex] = field.Index;
                }
            }
            public void ExitArrayStruct(MetaField field) 
            {
                if (m_inSegment)
                {
                    m_ixArrayStruct = NO_ARRAY_STRUCT;
                    Debug.Assert(m_ixIndex == 1);
                    --m_ixIndex;
                }
            }

            public void EnterBool(MetaField field, bool value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, value ? 1 : 0);
                }
            }
            public void EnterInt32(MetaField field, int value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterUInt32(MetaField field, uint value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterUInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterInt64(MetaField field, long value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterUInt64(MetaField field, ulong value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterUInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterFloat(MetaField field, float value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterDouble(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterDouble(MetaField field, double value) 
            {
                if (m_ixIndex >= 0)
                {
                    m_hl7Builder.EnterDouble(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                }
            }
            public void EnterString(MetaField field, string value) 
            {
                if (m_ixIndex >= 0 && value.Length != 0)
                {
                    // skip message type
                    if (!(m_ixIndex == 2 && m_indexOfLayer[0] == 0 && m_indexOfLayer[1] == 8))
                    {
                        m_hl7Builder.EnterString(m_indexOfLayer, m_ixIndex + 1, field.Index, value);
                    }
                }
            }
            public void EnterString(MetaField field, byte[] buffer, int offset, int size) 
            {
                string value = Encoding.UTF8.GetString(buffer, offset, size);
                EnterString(field, value);
            }
            public void EnterBytes(MetaField field, byte[] value, int offset, int size) 
            {
                // convert to base64
                string base64 = System.Convert.ToBase64String(value, offset, size);
                EnterString(field, base64);
            }
            public void EnterEnum(MetaField field, int value) 
            {
                if (m_enumAsString)
                {
                    string name = MetaDataGlobal.Instance.GetEnumAliasByValue(field, value);
                    EnterString(field, name);
                }
                else
                {
                    EnterInt32(field, value);
                }
            }
            public void EnterEnum(MetaField field, string value) 
            {
                if (m_enumAsString)
                {
                    EnterString(field, value);
                }
                else
                {
                    int v = MetaDataGlobal.Instance.GetEnumValueByName(field, value);
                    EnterInt32(field, v);
                }
            }

            public void EnterArrayBool(MetaField field, bool[] value) 
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, v ? 1 : 0);
                    }
                }
            }
            public void EnterArrayInt32(MetaField field, int[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayUInt32(MetaField field, uint[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterUInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayInt64(MetaField field, long[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayUInt64(MetaField field, ulong[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterUInt64(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayFloat(MetaField field, float[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterDouble(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayDouble(MetaField field, double[] value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterDouble(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayString(MetaField field, IList<string> value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        m_hl7Builder.EnterString(m_indexOfLayer, m_ixIndex + 1, field.Index, v);
                    }
                }
            }
            public void EnterArrayBytes(MetaField field, IList<byte[]> value)
            {
                if (m_ixIndex >= 0)
                {
                    foreach (var v in value)
                    {
                        // convert to base64
                        string base64 = System.Convert.ToBase64String(v);
                        m_hl7Builder.EnterString(m_indexOfLayer, m_ixIndex + 1, field.Index, base64);
                    }
                }
            }
            public void EnterArrayEnum(MetaField field, int[] value)
            {
                if (m_enumAsString)
                {
                    foreach (var v in value)
                    {
                        string name = MetaDataGlobal.Instance.GetEnumAliasByValue(field, v);
                        EnterString(field, name);
                    }
                }
                else
                {
                    foreach (var v in value)
                    {
                        EnterInt32(field, v);
                    }
                }
            }
            public void EnterArrayEnum(MetaField field, IList<string> value)
            {
                if (m_enumAsString)
                {
                    foreach (var v in value)
                    {
                        EnterString(field, v);
                    }
                }
                else
                {
                    foreach (var v in value)
                    {
                        int n = MetaDataGlobal.Instance.GetEnumValueByName(field, v);
                        EnterInt32(field, n);
                    }
                }
            }

            readonly Hl7Builder m_hl7Builder;
            readonly bool m_enumAsString;

            bool m_inSegment = false;

            int[] m_indexOfLayer = new int[10];
            int m_ixIndex = -1;
            int m_ixArrayStruct = NO_ARRAY_STRUCT;
        }

        readonly Internal m_internal;
        readonly IParserVisitor m_parserProcessDefaultValues;
    }

}
