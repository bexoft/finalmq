using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{

    public class ParserAbortAndIndex : IParserVisitor
    {
        public ParserAbortAndIndex(IParserVisitor? visitor = null)
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

            m_levelState.Add(new LevelState());
        }
        public void Finished()
        {
            if (m_levelState.Count > 0)
            {
                m_levelState.RemoveAt(m_levelState.Count - 1);
            }

            Debug.Assert(m_visitor != null);
            m_visitor.Finished();
        }

        public void EnterStruct(MetaField field)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                levelState = new LevelState();
                levelState.abortStruct = AbortStatus.ABORT_STRUCT;
                m_levelState.Add(levelState);
                return;
            }

            m_visitor?.EnterStruct(field);

            m_levelState.Add(new LevelState());
        }

        public void ExitStruct(MetaField field)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if (levelState.abortStruct == AbortStatus.ABORT_STRUCT)
            {
                m_levelState.RemoveAt(m_levelState.Count - 1);
                return;
            }

            m_visitor?.ExitStruct(field);

            m_levelState.RemoveAt(m_levelState.Count - 1);
        }

        public void EnterStructNull(MetaField field)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterStructNull(field);
        }

        public void EnterArrayStruct(MetaField field)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                levelState = new LevelState();
                levelState.abortStruct = AbortStatus.ABORT_STRUCT;
                m_levelState.Add(levelState);
                return;
            }

            m_visitor?.EnterArrayStruct(field);

            m_levelState.Add(new LevelState());
        }
        public void ExitArrayStruct(MetaField field)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if (levelState.abortStruct == AbortStatus.ABORT_STRUCT)
            {
                m_levelState.RemoveAt(m_levelState.Count - 1);
                return;
            }

            m_visitor?.ExitArrayStruct(field);

            m_levelState.RemoveAt(m_levelState.Count - 1);
        }

        static readonly string ABORTSTRUCT = "abortstruct";
        static readonly string ABORT_FALSE = "false";
        static readonly string ABORT_TRUE = "true";


        public void EnterBool(MetaField field, bool value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterBool(field, value);

            // check abort
            string? valueAbort = field.GetProperty(ABORTSTRUCT);
            if (valueAbort != null)
            {
                if (((valueAbort == ABORT_TRUE) && value) ||
                    ((valueAbort == ABORT_FALSE) && !value))
                {
                    levelState.abortStruct = AbortStatus.ABORT_FIELD;
                }
            }
        }
        public void EnterInt8(MetaField field, sbyte value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterInt8(field, value);
            CheckIndex(field, value);
        }
        public void EnterUInt8(MetaField field, byte value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterUInt8(field, value);
            CheckIndex(field, value);
        }
        public void EnterInt16(MetaField field, short value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterInt16(field, value);
            CheckIndex(field, value);
        }
        public void EnterUInt16(MetaField field, ushort value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterUInt16(field, value);
            CheckIndex(field, value);
        }
        public void EnterInt32(MetaField field, int value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterInt32(field, value);
            CheckIndex(field, value);
        }
        public void EnterUInt32(MetaField field, uint value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterUInt32(field, value);
            CheckIndex(field, value);
        }
        public void EnterInt64(MetaField field, long value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterInt64(field, value);
            CheckIndex(field, value);
        }
        public void EnterUInt64(MetaField field, ulong value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterUInt64(field, value);
            CheckIndex(field, (long)value);
        }
        public void EnterFloat(MetaField field, float value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterFloat(field, value);
        }
        public void EnterDouble(MetaField field, double value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterDouble(field, value);
        }
        public void EnterString(MetaField field, string value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterString(field, value);
        }
        public void EnterString(MetaField field, byte[] buffer, int offset, int size)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterString(field, buffer, offset, size);
        }
        public void EnterBytes(MetaField field, byte[] value, int offset, int size)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterBytes(field, value, offset, size);
        }
        public void EnterEnum(MetaField field, int value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterEnum(field, value);

            // check abort
            MetaEnum? en = MetaDataGlobal.Instance.GetEnum(field.TypeName);
            if (en == null)
            {
                //streamError << "enum not found " << field.typeName;
            }
            string? valueAbort = field.GetProperty(ABORTSTRUCT);
            if (valueAbort != null && en != null)
            {
                string strValue = en.GetNameByValue(value);
                string[] valuesAbort = valueAbort.Split('|');
                if (valuesAbort.Contains(strValue))
                {
                    levelState.abortStruct = AbortStatus.ABORT_FIELD;
                }
                else
                {
                    string aliasValue = en.GetAliasByValue(value);
                    if (valuesAbort.Contains(aliasValue))
                    {
                        levelState.abortStruct = AbortStatus.ABORT_FIELD;
                    }
                }
            }
            else
            {
                CheckIndex(field, value);
            }
        }
        public void EnterEnum(MetaField field, string value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterEnum(field, value);

            // check abort
            MetaEnum? en = MetaDataGlobal.Instance.GetEnum(field.TypeName);
            if (en == null)
            {
                //streamError << "enum not found " << field.typeName;
            }
            string? valueAbort = field.GetProperty(ABORTSTRUCT);
            if (valueAbort != null && en != null)
            {
                int v = en.GetValueByName(value);
                string strValue = en.GetNameByValue(v);
                string[] valuesAbort = valueAbort.Split('|');
                if (valuesAbort.Contains(strValue))
                {
                    levelState.abortStruct = AbortStatus.ABORT_FIELD;
                }
                else
                {
                    string aliasValue = en.GetAliasByValue(v);
                    if (valuesAbort.Contains(aliasValue))
                    {
                        levelState.abortStruct = AbortStatus.ABORT_FIELD;
                    }
                }
            }
            else
            {
                if (en != null)
                {
                    int v = en.GetValueByName(value);
                    CheckIndex(field, v);
                }
            }
        }

        public void EnterArrayBool(MetaField field, bool[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayBool(field, value);
        }
        public void EnterArrayInt8(MetaField field, sbyte[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayInt8(field, value);
        }
        public void EnterArrayInt16(MetaField field, short[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayInt16(field, value);
        }
        public void EnterArrayUInt16(MetaField field, ushort[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayUInt16(field, value);
        }
        public void EnterArrayInt32(MetaField field, int[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayInt32(field, value);
        }
        public void EnterArrayUInt32(MetaField field, uint[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayUInt32(field, value);
        }
        public void EnterArrayInt64(MetaField field, long[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayInt64(field, value);
        }
        public void EnterArrayUInt64(MetaField field, ulong[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayUInt64(field, value);
        }
        public void EnterArrayFloat(MetaField field, float[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayFloat(field, value);
        }
        public void EnterArrayDouble(MetaField field, double[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayDouble(field, value);
        }
        public void EnterArrayString(MetaField field, IList<string> value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayString(field, value);
        }
        public void EnterArrayBytes(MetaField field, IList<byte[]> value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayBytes(field, value);
        }
        public void EnterArrayEnum(MetaField field, int[] value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayEnum(field, value);
        }
        public void EnterArrayEnum(MetaField field, IList<string> value)
        {
            Debug.Assert(m_levelState.Count > 0);
            LevelState levelState = m_levelState[m_levelState.Count - 1];

            if ((levelState.abortStruct != AbortStatus.ABORT_NONE) || (levelState.index != INDEX_NOT_AVAILABLE && levelState.index != field.Index && levelState.indexOfIndexField < field.Index))
            {
                return;
            }

            m_visitor?.EnterArrayEnum(field, value);
        }

        static readonly string INDEXMODE = "indexmode";
        static readonly string INDEXMODE_MAPPING = "mapping";


        void CheckIndex(MetaField field, long value)
        {
            if ((field.Flags & (int)MetaFieldFlags.METAFLAG_INDEX) != 0)
            {
                Debug.Assert(m_levelState.Count > 0);
                LevelState levelState = m_levelState[m_levelState.Count - 1];

                string? indexmode = field.GetProperty(INDEXMODE);
                if (indexmode == INDEXMODE_MAPPING)
                {
                    string strIndex = value.ToString();
                    string? strIndexMapped = field.GetProperty(strIndex);
                    if (strIndexMapped == null)
                    {
                        levelState.abortStruct = AbortStatus.ABORT_FIELD;
                    }
                    else
                    {
                        int indexMapped = 0;
                        Int32.TryParse(strIndexMapped, out indexMapped);
                        levelState.indexOfIndexField = field.Index;
                        levelState.index = field.Index + 1 + indexMapped;
                    }
                }
                else
                {
                    if (value < 0)
                    {
                        levelState.abortStruct = AbortStatus.ABORT_FIELD;
                    }
                    else
                    {
                        levelState.indexOfIndexField = field.Index;
                        levelState.index = field.Index + 1 + value;
                    }
                }
            }
        }

        IParserVisitor? m_visitor = null;

        static readonly long INDEX_NOT_AVAILABLE = -1;

        enum AbortStatus
        {
            ABORT_NONE = 0,
            ABORT_FIELD = 1,
            ABORT_STRUCT = 2,
        };

        class LevelState
        {

            public AbortStatus abortStruct = AbortStatus.ABORT_NONE;
            public long indexOfIndexField = INDEX_NOT_AVAILABLE;
            public long index = INDEX_NOT_AVAILABLE;
        };

        IList<LevelState> m_levelState = new List<LevelState>();
    }

}
