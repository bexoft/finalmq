using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{

    public class ParserRedirect : IParserVisitor
    {
        public ParserRedirect(IParserVisitor? visitor = null)
        {
            m_visitor = visitor;
        }
        public void SetVisitor(IParserVisitor visitor)
        {
            m_visitor = visitor;
        }

        // IParserVisitor
        public void NotifyError(string str, string message)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.NotifyError(str, message);
        }
        public void StartStruct(MetaStruct stru)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.StartStruct(stru);
        }
        public void Finished()
        {
            Debug.Assert(m_visitor != null);
            m_visitor.Finished();
        }

        public void EnterStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            if (field.TypeId == MetaTypeId.TYPE_STRUCT)
            {
                m_visitor.EnterStruct(field);
            }
        }
        public void ExitStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            if (field.TypeId == MetaTypeId.TYPE_STRUCT)
            {
                m_visitor.ExitStruct(field);
            }
        }

        public void EnterArrayStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            if (field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT)
            {
                m_visitor.EnterArrayStruct(field);
            }
        }
        public void ExitArrayStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            if (field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT)
            {
                m_visitor.ExitArrayStruct(field);
            }
        }

        public void EnterBool(MetaField field, bool value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterBool(field, value);
        }
        public void EnterInt32(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterInt32(field, value);
        }
        public void EnterUInt32(MetaField field, uint value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterUInt32(field, value);
        }
        public void EnterInt64(MetaField field, long value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterInt64(field, value);
        }
        public void EnterUInt64(MetaField field, ulong value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterUInt64(field, value);
        }
        public void EnterFloat(MetaField field, float value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterFloat(field, value);
        }
        public void EnterDouble(MetaField field, double value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterDouble(field, value);
        }
        public void EnterString(MetaField field, string value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterString(field, value);
        }
        public void EnterString(MetaField field, byte[] buffer, int offset, int size)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterString(field, buffer, offset, size);
        }
        public void EnterBytes(MetaField field, byte[] value, int offset, int size)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterBytes(field, value, offset, size);
        }
        public void EnterEnum(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterEnum(field, value);
        }
        public void EnterEnum(MetaField field, string value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterEnum(field, value);
        }

        public void EnterArrayBool(MetaField field, bool[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayBool(field, value);
        }
        public void EnterArrayInt32(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayInt32(field, value);
        }
        public void EnterArrayUInt32(MetaField field, uint[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayUInt32(field, value);
        }
        public void EnterArrayInt64(MetaField field, long[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayInt64(field, value);
        }
        public void EnterArrayUInt64(MetaField field, ulong[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayUInt64(field, value);
        }
        public void EnterArrayFloat(MetaField field, float[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayFloat(field, value);
        }
        public void EnterArrayDouble(MetaField field, double[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayDouble(field, value);
        }
        public void EnterArrayString(MetaField field, IList<string> value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayString(field, value);
        }
        public void EnterArrayBytes(MetaField field, IList<byte[]> value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayBytes(field, value);
        }
        public void EnterArrayEnum(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayEnum(field, value);
        }
        public void EnterArrayEnum(MetaField field, IList<string> value)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.EnterArrayEnum(field, value);
        }

        IParserVisitor? m_visitor = null;
    }
}




