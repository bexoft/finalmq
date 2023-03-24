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

        bool Matches(string segId, MetaStruct stru, int ixStart)
        {
            if ((stru.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0)
            {
                return false;
            }
            for (int i = ixStart; i < stru.FieldsSize; ++i)
            {
                MetaField? field = stru.GetFieldByIndex(i);
                Debug.Assert(field != null);
                if (field.TypeId == MetaTypeId.TYPE_STRUCT || field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    if (segId == field.TypeNameWithoutNamespace)
                    {
                        return true;
                    }
                    MetaStruct? subStruct = MetaDataGlobal.Instance.GetStruct(field);
                    if (subStruct != null)
                    {
                        bool matche = Matches(segId, subStruct, 0);
                        if (matche)
                        {
                            return true;
                        }
                    }
                    if ((field.TypeId == MetaTypeId.TYPE_STRUCT) && 
                        (field.Flags & (int)MetaFieldFlags.METAFLAG_NULLABLE) == 0)
                    {
                        return false;
                    }
                    if ((field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT) && 
                        (field.Flags & (int)MetaFieldFlags.METAFLAG_ONE_REQUIRED) != 0)
                    {
                        return false;
                    }
                }
            }
            return false;
        }

        bool MatchesUp(string segId)
        {
            for (int i = m_stackStruct.Count - 1; i >= 0; --i)
            {
                Tuple<MetaStruct, int> entry = m_stackStruct[i];
                bool match = Matches(segId, entry.Item1, entry.Item2);
                if (match)
                {
                    return true;
                }
            }
            return false;
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
            bool isarrayDummy;
            int level = ParseStruct(0, stru, out isarrayDummy);
            m_visitor.Finished();

            if (level <= PARSER_HL7_ERROR)
            {
                return -1;
            }
            return m_parser.GetCurrentPosition();
        }

        static readonly string STR_MSH = "MSH";

        int ParseStruct(int levelSegment, MetaStruct stru, out bool isarray)
        {
            isarray = false;
            // segment ID
            if (levelSegment == 1)
            {
                string segId;
                bool isarrayDummy;
                int levelNew = m_parser.ParseToken(levelSegment, out segId, out isarrayDummy);
                Debug.Assert(isarray == false);
                if (levelNew < levelSegment)
                {
                    return levelNew;
                }
                string typeName = stru.TypeNameWithoutNamespace;
                if ((segId != typeName) && (segId != STR_MSH || !typeName.StartsWith(STR_MSH))) // for MSH -> startsWith("MSH") to allow also different types to read only the header (see MSH_RE)
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
                        if (segId == "")
                        {
                            return levelSegment;
                        }
                        else if ((segId == typeName) || (segId == STR_MSH && typeName.StartsWith(STR_MSH))) // for MSH -> startsWith("MSH") to allow also different types to read only the header (see MSH_RE)
                        {
                            processStruct = true;
                        }
                        else if (Matches(segId, subStruct, 0))
                        {
                            processStruct = true;
                        }
                        else if (Matches(segId, stru, i + 1))
                        {
                            processStruct = false;
                        }
                        else if (MatchesUp(segId))
                        {
                            return levelSegment;
                        }
                        else
                        {
                            processStruct = false;
                            m_parser.ParseTillEndOfStruct(0);
                            --i;
                        }
                    }
                    if (processStruct)
                    {
                        m_visitor.EnterStruct(field);
                        m_stackStruct.Add(new Tuple<MetaStruct, int>(stru, i + 1));
                        int LevelSegmentNext = levelSegment;
                        if ((LevelSegmentNext > 0) || ((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0))
                        {
                            ++LevelSegmentNext;
                        }
                        int levelNew = ParseStruct(LevelSegmentNext, subStruct, out isarray);
                        m_stackStruct.RemoveAt(m_stackStruct.Count - 1);
                        m_visitor.ExitStruct(field);
                        if (isarray && levelSegment == 1)
                        {
                            isarray = false;
                            levelNew = m_parser.ParseTillEndOfStruct(levelSegment);
                        }
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }
                        if ((levelSegment == 0) && ((stru.Flags & (int)MetaStructFlags.METASTRUCTFLAG_CHOICE) != 0))
                        {
                            return levelSegment;
                        }
                    }
                } 
                else if (field.TypeId == MetaTypeId.TYPE_ARRAY_STRUCT)
                {
                    MetaStruct? subStruct = MetaDataGlobal.Instance.GetStruct(field);
                    if (subStruct == null)
                    {
                        return PARSER_HL7_ERROR;
                    }
                    if (levelSegment == 0)
                    {
                        int returnWithLevel = -2;
                        MetaField? fieldWithoutArray = field.FieldWithoutArray;
                        Debug.Assert(fieldWithoutArray != null);
                        string typeName = field.TypeNameWithoutNamespace;
                        bool firstLoop = true;
                        while (true)
                        {
                            string segId = m_parser.GetSegmentId();
                            bool processStructArray = false;
                            if (segId == "")
                            {
                                returnWithLevel = levelSegment;
                            }
                            else if (segId == typeName)
                            {
                                processStructArray = true;
                            }
                            else if (Matches(segId, subStruct, 0))
                            {
                                processStructArray = true;
                            }
                            else if (Matches(segId, stru, i + 1))
                            {
                                processStructArray = false;
                            }
                            else if (MatchesUp(segId))
                            {
                                returnWithLevel = levelSegment;
                            }
                            else
                            {
                                processStructArray = false;
                                m_parser.ParseTillEndOfStruct(0);
                                --i;
                            }

                            if (processStructArray)
                            {
                                segId = m_parser.GetSegmentId();
                                if (firstLoop)
                                {
                                    m_stackStruct.Add(new Tuple<MetaStruct, int>(stru, i));
                                    firstLoop = false;
                                    m_visitor.EnterArrayStruct(field);
                                }
                                m_visitor.EnterStruct(fieldWithoutArray);
                                int LevelSegmentNext = levelSegment;
                                if ((subStruct.Flags & (int)MetaStructFlags.METASTRUCTFLAG_HL7_SEGMENT) != 0)
                                {
                                    ++LevelSegmentNext;
                                }
                                int levelNew = ParseStruct(LevelSegmentNext, subStruct, out isarray);
                                m_visitor.ExitStruct(fieldWithoutArray);
                                Debug.Assert(isarray == false);
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
                                    m_stackStruct.RemoveAt(m_stackStruct.Count - 1);
                                }
                                break;
                            }
                            if (returnWithLevel != -2)
                            {
                                return returnWithLevel;
                            }
                        }
                    }
                    else
                    {
                        bool filled = false;
                        int levelNew = m_parser.IsNextFieldFilled(levelSegment, out filled);
                        if (levelNew < levelSegment)
                        {
                            return levelNew;
                        }

                        if (filled)
                        {
                            levelNew = levelSegment;
                            m_visitor.EnterArrayStruct(field);
                            while (true)
                            {
                                m_visitor.EnterStruct(field);
                                levelNew = ParseStruct(levelSegment + 1, subStruct, out isarray);
                                m_visitor.ExitStruct(field);
                                if (levelNew < levelSegment || !isarray)
                                {
                                    break;
                                }
                            }
                            m_visitor.ExitArrayStruct(field);
                            isarray = false;
                            if (levelNew < levelSegment)
                            {
                                return levelNew;
                            }
                        }
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
                        int levelNew = m_parser.ParseToken(levelSegment, out token, out isarray);
                        if (token != "")
                        {
                            m_visitor.EnterString(field, token);
                        }
                        if (isarray && levelSegment == 1)
                        {
                            isarray = false;
                            levelNew = m_parser.ParseTillEndOfStruct(levelSegment);
                        }
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
        readonly IList<Tuple<MetaStruct, int>> m_stackStruct = new List<Tuple<MetaStruct, int>>();
    }
    
}
