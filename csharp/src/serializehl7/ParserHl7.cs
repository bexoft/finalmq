using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{
    
    public class ParserHl7
    {
        static readonly int PARSER_HL7_ERROR = -2;

        public ParserHl7(IParserVisitor visitor, byte[] buffer, int offset = 0, int size = -1)
        {
            m_parser = new Hl7Parser();
            m_buffer = buffer;
            m_offset = offset;
            m_size = size;
            m_visitor = visitor;
        }

        public ParserHl7(IParserVisitor visitor, string hl7, int offset = 0, int size = -1)
        {
            if (size == -1)
            {
                size = hl7.Length - offset;
            }
            byte[] buffer = Encoding.UTF8.GetBytes(hl7, offset, size);
            m_parser = new Hl7Parser();
            m_buffer = buffer;
            m_offset = 0;
            m_size = buffer.Length;
            m_visitor = visitor;
        }

        public int ParseStruct(string typeName)
        {
            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(typeName);
            if (stru == null)
            {
                m_visitor.NotifyError("", "typename not found");
                m_visitor.Finished();
                return -1;
            }

            bool ok = m_parser.StartParse(m_buffer, m_offset, m_size);
            if (!ok)
            {
                return -1;
            }

            m_visitor.StartStruct(stru);
            int level = ParseStruct(0, 0, stru);
            m_visitor.Finished();

            if (level <= PARSER_HL7_ERROR)
            {
                return -1;
            }
            return m_parser.GetCurrentPosition();
        }

        int ParseStruct(int levelStruct, int levelSegment, MetaStruct stru)
        {
            // segment ID
            if (levelSegment == 1)
            {
                string tokenSegId;
                int levelNew = m_parser.ParseToken(levelSegment, out tokenSegId);
                if (levelNew<levelSegment)
                {
                    return levelNew;
                }
                if (tokenSegId != stru.TypeNameWithoutNamespace)
                {
                    return m_parser.ParseTillEndOfStruct(0);
                }
            }

            int size = stru.FieldsSize;
            for (int i = 0; i < size; ++i)
            {
                MetaField? field = stru.GetFieldByIndex(i);
                Debug.Assert(field != null);

                if (field.TypeId == MetaTypeId.TYPE_STRUCT)
                {
                    MetaStruct? subStruct = MetaDataGlobal.Instance.GetStruct(field);
                    if (subStruct == null)
                    {
                        return PARSER_HL7_ERROR;
                    }
                    bool processStruct = true;
                    if (levelSegment == 0)
                    {
                        string typeName = field.TypeNameWithoutNamespace;
                        string segId = m_parser.GetSegmentId();
                        MetaField? subField = null;
                        if ((segId != typeName) && ((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) == 0) && (subStruct.FieldsSize > 0))
                        {
                            subField = subStruct.GetFieldByIndex(0);
                        }
                        if ((segId == typeName) || (subField != null && segId == subField.TypeNameWithoutNamespace))
                        {
                            processStruct = true;
                        }
                        else
                        {
                            processStruct = false;
                            m_parser.ParseTillEndOfStruct(0);
                        }
                    }
                    if (processStruct)
                    {
                        m_visitor.EnterStruct(field);
                        int LevelSegmentNext = levelSegment;
                        if ((LevelSegmentNext > 0) || ((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0))
                        {
                            ++LevelSegmentNext;
                        }
                        int levelNew = ParseStruct(levelStruct + 1, LevelSegmentNext, subStruct);
                        m_visitor.ExitStruct(field);
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }
                    }
                }
                else if (field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    if (levelSegment == 0)
                    {
                        MetaStruct? subStruct = MetaDataGlobal.Instance.GetStruct(field);
                        if (subStruct == null)
                        {
                            return PARSER_HL7_ERROR;
                        }
                        MetaField? subField = null;
                        if (((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) == 0) && (subStruct.FieldsSize > 0))
                        {
                            subField = subStruct.GetFieldByIndex(0);
                        }
                        string typeName = field.TypeNameWithoutNamespace;
                        bool firstLoop = true;
                        do
                        {
                            string segId = m_parser.GetSegmentId();
                            if ((segId == typeName) || (subField != null && segId == subField.TypeNameWithoutNamespace))
                            {
                                if (firstLoop)
                                {
                                    firstLoop = false;
                                    m_visitor.EnterArrayStruct(field);
                                }
                                m_visitor.EnterStruct(field);
                                int LevelSegmentNext = levelSegment;
                                if ((LevelSegmentNext > 0) || ((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0))
                                {
                                    ++LevelSegmentNext;
                                }
                                int levelNew = ParseStruct(levelStruct + 1, LevelSegmentNext, subStruct);
                                m_visitor.ExitStruct(field);
                                if (levelNew < levelSegment)
                                {
                                    return levelNew;
                                }
                            }
                            else
                            {
                                if (!firstLoop)
                                {
                                    m_visitor.ExitArrayStruct(field);
                                }
                                break;
                            }
                        } while (true);
                    }
                }
                else if ((field.TypeId & MetaTypeId.OFFSET_ARRAY_FLAG) != 0)
                {
                    if (levelSegment > 0)
                    {
                        IList<string> array = new List<string>();
                        int levelNew = m_parser.ParseTokenArray(levelSegment, array);
                        m_visitor.EnterArrayString(field, array);
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }
                    }
                }
                else
                {
                    if (levelSegment > 0)
                    {
                        string token;
                        int levelNew = m_parser.ParseToken(levelSegment, out token);
                        m_visitor.EnterString(field, token);
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }
                    }
                }
            }
            if (levelSegment > 0)
            {
                return m_parser.ParseTillEndOfStruct(levelSegment - 1);
            }
            return levelSegment;
            }

        readonly byte[] m_buffer;
        readonly int m_offset;
        readonly int m_size;
        readonly IParserVisitor m_visitor;
        readonly Hl7Parser m_parser;
    }
    
}
