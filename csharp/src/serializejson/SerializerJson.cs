using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{
    
    public class SerializerJson : ParserConverter
    {
        public SerializerJson(IZeroCopyBuffer buffer, int maxBlockSize = 512, bool enumAsString = true, bool skipDefaultValues = true)
        {
            m_internal = new Internal(buffer, maxBlockSize, enumAsString);
            m_parserProcessDefaultValues = new ParserProcessDefaultValues(skipDefaultValues, m_internal);
            SetVisitor(m_parserProcessDefaultValues);
        }

        class Internal : IParserVisitor
        {
            public Internal(IZeroCopyBuffer buffer, int maxBlockSize, bool enumAsString)
            {
                m_jsonBuilder = new JsonBuilder(buffer, maxBlockSize);
                m_enumAsString = enumAsString;
            }
            // IParserVisitor

            public void NotifyError(string str, string message) {}
            public void StartStruct(MetaStruct stru) 
            {
                // outer curly brackets
                m_jsonBuilder.EnterObject();
            }
            public void Finished() 
            {
                // outer curly brackets
                m_jsonBuilder.ExitObject();
                m_jsonBuilder.Finished();
            }

            public void EnterStruct(MetaField field) 
            {
                SetKey(field);
                m_jsonBuilder.EnterObject();
            }
            public void ExitStruct(MetaField field) 
            {
                m_jsonBuilder.ExitObject();
            }

            public void EnterArrayStruct(MetaField field) 
            {
                SetKey(field);
                m_jsonBuilder.EnterArray();
            }
            public void ExitArrayStruct(MetaField field) 
            {
                m_jsonBuilder.ExitArray();
            }

            public void EnterBool(MetaField field, bool value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_BOOL);
                SetKey(field);
                m_jsonBuilder.EnterBool(value);
            }
            public void EnterInt32(MetaField field, int value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_INT32);
                SetKey(field);
                m_jsonBuilder.EnterInt32(value);
            }
            public void EnterUInt32(MetaField field, uint value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_UINT32);
                SetKey(field);
                m_jsonBuilder.EnterUInt32(value);
            }
            public void EnterInt64(MetaField field, long value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_INT64);
                SetKey(field);
                m_jsonBuilder.EnterString(value.ToString());
            }
            public void EnterUInt64(MetaField field, ulong value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_UINT64);
                SetKey(field);
                m_jsonBuilder.EnterString(value.ToString());
            }
            public void EnterFloat(MetaField field, float value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_FLOAT);
                SetKey(field);
                HandleDouble(value);
            }
            public void EnterDouble(MetaField field, double value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_DOUBLE);
                SetKey(field);
                HandleDouble(value);
            }
            public void EnterString(MetaField field, string value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_STRING);
                SetKey(field);
                m_jsonBuilder.EnterString(value);
            }
            public void EnterString(MetaField field, byte[] buffer, int offset, int size) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_STRING);
                SetKey(field);
                m_jsonBuilder.EnterString(Encoding.UTF8.GetString(buffer, offset, size));
            }
            public void EnterBytes(MetaField field, byte[] value, int offset, int size) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_BYTES);
                SetKey(field);
                // convert to base64
                string base64 = System.Convert.ToBase64String(value, offset, size);
                m_jsonBuilder.EnterString(base64);
            }
            public void EnterEnum(MetaField field, int value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ENUM);
                SetKey(field);
                if (m_enumAsString)
                {
                    string name = MetaDataGlobal.Instance.GetEnumAliasByValue(field, value);
                    m_jsonBuilder.EnterString(name);
                }
                else
                {
                    m_jsonBuilder.EnterInt32(value);
                }
            }
            public void EnterEnum(MetaField field, string value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ENUM);
                SetKey(field);
                if (m_enumAsString)
                {
                    m_jsonBuilder.EnterString(value);
                }
                else
                {
                    int v = MetaDataGlobal.Instance.GetEnumValueByName(field, value);
                    m_jsonBuilder.EnterInt32(v);
                }
            }

            public void EnterArrayBool(MetaField field, bool[] value) 
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_BOOL);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterBool(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayInt32(MetaField field, int[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_INT32);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterInt32(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayUInt32(MetaField field, uint[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_UINT32);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterUInt32(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayInt64(MetaField field, long[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_INT64);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterString(entry.ToString());
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayUInt64(MetaField field, ulong[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_UINT64);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterString(entry.ToString());
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayFloat(MetaField field, float[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_FLOAT);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    HandleFloat(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayDouble(MetaField field, double[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_DOUBLE);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    HandleDouble(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayString(MetaField field, IList<string> value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_STRING);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    m_jsonBuilder.EnterString(entry);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayBytes(MetaField field, IList<byte[]> value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_BYTES);
                SetKey(field);
                m_jsonBuilder.EnterArray();
                foreach (var entry in value)
                {
                    string base64 = Convert.ToBase64String(entry);
                    m_jsonBuilder.EnterString(base64);
                }
                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayEnum(MetaField field, int[] value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_ENUM);
                SetKey(field);
                m_jsonBuilder.EnterArray();

                if (m_enumAsString)
                {
                    foreach (var entry in value)
                    {
                        string name = MetaDataGlobal.Instance.GetEnumAliasByValue(field, entry);
                        m_jsonBuilder.EnterString(name);
                    }
                }
                else
                {
                    foreach (var entry in value)
                    {
                        m_jsonBuilder.EnterInt32(entry);
                    }
                }

                m_jsonBuilder.ExitArray();
            }
            public void EnterArrayEnum(MetaField field, IList<string> value)
            {
                Debug.Assert(field.TypeId == MetaTypeId.TYPE_ARRAY_ENUM);
                SetKey(field);
                m_jsonBuilder.EnterArray();

                if (m_enumAsString)
                {
                    foreach (var entry in value)
                    {
                        m_jsonBuilder.EnterString(entry);
                    }
                }
                else
                {
                    foreach (var entry in value)
                    {
                        int v = MetaDataGlobal.Instance.GetEnumValueByName(field, entry);
                        m_jsonBuilder.EnterInt32(v);
                    }
                }

                m_jsonBuilder.ExitArray();
            }

            void SetKey(MetaField field)
            {
                string name = field.Name;
                if (name.Length != 0)
                {
                    m_jsonBuilder.EnterKey(name);
                }
            }

            void HandleFloat(float value)
            {
#pragma warning disable CS1718 // Vergleich erfolgte mit derselben Variable
                if (value != value) // in case of value == Single.NaN, this if will become true. (value == Double.NaN is always false)
#pragma warning restore CS1718 // Vergleich erfolgte mit derselben Variable
                {
                    m_jsonBuilder.EnterString("NaN");
                }
                else if (value == Single.PositiveInfinity)
                {
                    m_jsonBuilder.EnterString("Infinity");
                }
                else if (value == Single.NegativeInfinity)
                {
                    m_jsonBuilder.EnterString("-Infinity");
                }
                else
                {
                    m_jsonBuilder.EnterDouble(value);
                }
            }

            void HandleDouble(double value)
            {
#pragma warning disable CS1718 // Vergleich erfolgte mit derselben Variable
                if (value != value) // in case of value == Double.NaN, this if will become true. (value == Double.NaN is always false)
#pragma warning restore CS1718 // Vergleich erfolgte mit derselben Variable
                {
                    m_jsonBuilder.EnterString("NaN");
                }
                else if (value == Double.PositiveInfinity)
                {
                    m_jsonBuilder.EnterString("Infinity");
                }
                else if (value == Double.NegativeInfinity)
                {
                    m_jsonBuilder.EnterString("-Infinity");
                }
                else
                {
                    m_jsonBuilder.EnterDouble(value);
                }
            }

            readonly IJsonParserVisitor m_jsonBuilder;
            readonly bool m_enumAsString;
        }

        readonly Internal m_internal;
        readonly IParserVisitor m_parserProcessDefaultValues;
    }

}
