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
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;

namespace finalmq
{

    public class RemoteEntityFormatHl7 : IRemoteEntityFormat
    {
        static readonly int CONTENT_TYPE = 3;
        static readonly string CONTENT_TYPE_NAME = "hl7";

        public static readonly string PROPERTY_NAMESPACE = "namespace";
        public static readonly string PROPERTY_ENTITY = "entity";

        public static readonly string PROPERTY_LINEEND = "lineend";
        public static readonly string PROPERTY_MESSAGESTART = "messagestart";
        public static readonly string PROPERTY_MESSAGEEND = "messageend";


#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void RegisterFormat()
        {
            RemoteEntityFormatRegistry.Instance.RegisterFormat(CONTENT_TYPE_NAME, CONTENT_TYPE, new RemoteEntityFormatHl7());
        }


        static int FindSequence(byte[] haystack, int offset, byte[] needle, int len)
        {
            int end = Math.Min(haystack.Length, len + offset);

            if (needle.Length == 0)
            {
                return offset;
            }

            int j = offset;
            while (j < end)
            {
                int i = 0;
                while ((j < end) && (i < needle.Length) && needle[i] == haystack[j])
                {
                    ++i;
                    ++j;
                }
                if (i == needle.Length)
                {
                    return j - i;
                }
                j = j - i + 1;
            }
            return -1;
        }


        static readonly byte[] BYTES_LINEEND = { (byte)'\r' };
        static readonly byte[] BYTES_MSH = Encoding.ASCII.GetBytes("MSH");

        static void ReplaceSerialize(IMessage source, string lineend, IMessage destination)
        {
            int sizeSource = source.GetTotalSendPayloadSize();
            // just to reserve enough memory
            destination.AddSendPayload(0, 2 * sizeSource);

            byte[] lineendBytes = Encoding.ASCII.GetBytes(lineend);

            IList<BufferRef> payloads = source.GetAllSendPayloads();
            foreach (var payload in payloads)
            {
                byte[] buffer = payload.Buffer;
                int current = payload.Offset;
                int found = -1;
                int sizeRest = payload.Length;

                while ((sizeRest > 0) && (-1 != (found = FindSequence(buffer, current, BYTES_LINEEND, sizeRest))))
                {
                    int sizeData = found - current;
                    int sizeTotal = sizeData + lineend.Length;
                    BufferRef dest = destination.AddSendPayload(sizeTotal, 512);
                    Array.Copy(buffer, current, dest.Buffer, dest.Offset, sizeData);
                    Array.Copy(lineendBytes, 0, dest.Buffer, dest.Offset + sizeData, lineendBytes.Length);
                    current = found + 1;  // 1: size of character '\r'
                    sizeRest -= sizeData + 1;
                }

                if (sizeRest > 0)
                {
                    destination.AddSendPayload(new BufferRef(buffer, current, sizeRest), 512);
                }
            }
        }

        static byte[] ReplaceParser(byte[] source, int offsetSource, int sizeSource, string lineend)
        {
            byte[] buffer = source;
            
            int found = -1;
            int sizeRest = sizeSource;

            int current = FindSequence(source, offsetSource, BYTES_MSH, sizeRest);
            if (current == -1)
            {
                return Array.Empty<byte>();
            }

            sizeRest -= current - offsetSource;

            ArrayBuilder<byte> destination = new ArrayBuilder<byte>();
            byte[] lineendBytes = Encoding.ASCII.GetBytes(lineend);

            while ((sizeRest > 0) && (-1 != (found = FindSequence(buffer, current, lineendBytes, sizeRest))))
            {
                int sizeData = found - current;
                destination.Add(buffer, current, sizeData);
                destination.Add(BYTES_LINEEND[0]);
                current = found + lineendBytes.Length;
                sizeRest -= sizeData + lineendBytes.Length;
            }

            if (sizeRest > 0)
            {
                destination.Add(buffer, current, sizeRest);
            }

            return destination.ToArray();
        }

        static bool IsReplaceNeeded(IProtocolSession session, out string lineend, out string messagestart, out string messageend)
        {
            lineend = "";
            messageend = "";
            string? hl7lineend = null;
            string? hl7messagestart = null;
            string? hl7messageend = null;
            Variant? formatData = session.FormatData;
            bool replaceNeeded = false;
            if (formatData != null && formatData.VarType != Variant.VARTYPE_NONE)
            {
                hl7lineend = formatData.GetData<string>(RemoteEntityFormatHl7.PROPERTY_LINEEND);
                hl7messagestart = formatData.GetData<string>(RemoteEntityFormatHl7.PROPERTY_MESSAGESTART);
                hl7messageend = formatData.GetData<string>(RemoteEntityFormatHl7.PROPERTY_MESSAGEEND);

                replaceNeeded = ( (hl7lineend != null && hl7lineend != "\r") );

                if (hl7lineend != null)
                {
                    lineend = hl7lineend;
                }
                if (hl7messagestart != null)
                {
                    messagestart = hl7messagestart;
                }
                if (hl7messageend != null)
                {
                    messageend = hl7messageend;
                }
            }
            return replaceNeeded;
        }


        public StructBase? Parse(IProtocolSession session, BufferRef bufferRef, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus)
        {
            formatStatus = 0;
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;

            byte[] bufferReplaced;
            string lineend;
            string messageend;
            string messagestart;
            bool replaceNeeded = IsReplaceNeeded(session, out lineend, out messagestart, out messageend);

            if (messagestart.Length != 0)
            {
                if (sizeBuffer >= messagestart.Length)
                {
                    byte[] messagestartBytes = Encoding.ASCII.GetBytes(messagestart);
                    if (FindSequence(buffer, offset, messagestartBytes, messagestartBytes.Length) != -1)
                    {
                        offset += messagestart.Length;
                        sizeBuffer -= messagestart.Length;
                    }
                }
            }

            if (messageend.Length != 0)
            {
                if (sizeBuffer >= messageend.Length)
                {
                    int posEnd = offset + sizeBuffer - messageend.Length;
                    byte[] messageendBytes = Encoding.ASCII.GetBytes(messageend);
                    if (FindSequence(buffer, posEnd, messageendBytes, messageendBytes.Length) != -1)
                    {
                        sizeBuffer -= messageend.Length;
                    }
                }
            }

            if (replaceNeeded)
            {
                bufferReplaced = ReplaceParser(buffer, offset, sizeBuffer, lineend);
                buffer = bufferReplaced;
                offset = 0;
                sizeBuffer = bufferReplaced.Length;
            }

            if (sizeBuffer == 0)
            {
                return null;
            }

            StructBase? data = null;

            finalmq.Hl7Header hl7Header = new finalmq.Hl7Header();
            SerializerStruct serializer = new SerializerStruct(hl7Header);
            ParserHl7 parserData = new ParserHl7(serializer, buffer, offset, sizeBuffer);
            int endData = parserData.ParseStruct("finalmq.Hl7Header");

            if (endData != -1)
            {
                string type = hl7Header.msh.messageType.messageStructure;
                if (type.Length == 0)
                {
                    type = hl7Header.msh.messageType.messageCode;
                    if (hl7Header.msh.messageType.triggerEvent.Length != 0)
                    {
                        type += "_" + hl7Header.msh.messageType.triggerEvent;
                    }
                }
                string? hl7Namespace = null;
                string? hl7DestName = null;
                Variant? formatData = session.FormatData;
                if (formatData != null && formatData.VarType != Variant.VARTYPE_NONE)
                {
                    hl7Namespace = formatData.GetData<string>(PROPERTY_NAMESPACE);
                    hl7DestName = formatData.GetData<string> (PROPERTY_ENTITY);
                }
                if (hl7Namespace == null)
                {
                    hl7Namespace = "hl7";
                }
                if (hl7DestName == null)
                {
                    hl7DestName = "hl7";
                }
                type = hl7Namespace + "." + type;
                header.destname = hl7DestName;
                header.type = type;
                if (header.path.Length == 0)
                {
                    header.path = header.type;
                }
                header.corrid = 1;

                BufferRef bufferRefData = new BufferRef(buffer, 0, sizeBuffer);
                data = ParseData(session, bufferRefData, storeRawData, header.type, ref formatStatus);

                formatStatus |= (int)FormatStatus.FORMATSTATUS_AUTOMATIC_CONNECT;
            }

            return data;
        }
        public StructBase? ParseData(IProtocolSession session, BufferRef bufferRef, bool storeRawData, string type, ref int formatStatus)
        {
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;

            if (sizeBuffer > 0 && buffer[offset + 0] == '{')
            {
                ++offset;
                --sizeBuffer;
            }
            if (sizeBuffer > 0 && buffer[offset + sizeBuffer - 1] == '}')
            {
                --sizeBuffer;
            }

            byte[] bufferReplaced;
            if ((formatStatus & (int)FormatStatus.FORMATSTATUS_HEADER_PARSED_BY_FORMAT) == 0)
            {
                string lineend;
                string messagestart;
                string messageend;
                bool replaceNeeded = IsReplaceNeeded(session, out lineend, out messagestart, out messageend);

                if (messagestart.Length != 0)
                {
                    if (sizeBuffer >= messagestart.Length)
                    {
                        byte[] messagestartBytes = Encoding.ASCII.GetBytes(messagestart);
                        if (FindSequence(buffer, offset, messagestartBytes, messagestartBytes.Length) != -1)
                        {
                            offset += messagestart.Length;
                            sizeBuffer -= messagestart.Length;
                        }
                    }
                }

                if (messageend.Length != 0)
                {
                    if (sizeBuffer >= messageend.Length)
                    {
                        int posEnd = offset + sizeBuffer - messageend.Length;
                        byte[] messageendBytes = Encoding.ASCII.GetBytes(messageend);
                        if (FindSequence(buffer, posEnd, messageendBytes, messageendBytes.Length) != -1)
                        {
                            sizeBuffer -= messageend.Length;
                        }
                    }
                }

                if (replaceNeeded)
                {
                    bufferReplaced = ReplaceParser(buffer, offset, sizeBuffer, lineend);
                    buffer = bufferReplaced;
                    offset = 0;
                    sizeBuffer = bufferReplaced.Length;
                }
            }

            StructBase? data = null;

            if (type.Length != 0)
            {
                data = TypeRegistry.Instance.CreateStruct(type);
                Debug.Assert(sizeBuffer >= 0);

                if (data != null)
                {
                    if (sizeBuffer > 0)
                    {
                        SerializerStruct serializerData = new SerializerStruct(data);
                        ParserHl7 parserData = new ParserHl7(serializerData, buffer, offset, sizeBuffer);
                        int endData = parserData.ParseStruct(type);
                        if (endData == -1)
                        {
                            formatStatus |= (int)FormatStatus.FORMATSTATUS_SYNTAX_ERROR;
                            data = null;
                        }
                    }
                }

                if (storeRawData)
                {
                    if (data == null)
                    {
                        data = new RawDataMessage();
                    }
                    data.SetRawData(type, CONTENT_TYPE, new BufferRef(buffer, offset, sizeBuffer));
                }
            }

            return data;
        }
        public void Serialize(IProtocolSession session, IMessage message, Header header, StructBase? structBase = null)
        {
            SerializeData(session, message, structBase);
        }
        public void SerializeData(IProtocolSession session, IMessage message, StructBase? structBase = null)
        {
            if (structBase != null)
            {
                string lineend;
                string messageend;
                bool replaceNeeded = IsReplaceNeeded(session, out lineend, out messageend);

                IMessage? messageHelper = replaceNeeded ? new ProtocolMessage(0) : null;
                IMessage messageToSerialize = (messageHelper != null) ? messageHelper : message;

                if (messagestart.Length > 0)
                {
                    messageToSerialize.AddSendPayload(Encoding.ASCII.GetBytes(messagestart), 512);
                }

                // payload
                if (structBase.GetRawContentType() == CONTENT_TYPE)
                {
                    BufferRef? rawData = structBase.GetRawData();
                    Debug.Assert(rawData != null);
                    messageToSerialize.AddSendPayload(rawData);
                }
                else
                {
                    SerializerHl7 serializerData = new SerializerHl7(messageToSerialize, 512);
                    ParserStruct parserData = new ParserStruct(serializerData, structBase);
                    parserData.ParseStruct();
                }

                if (messageend.Length > 0)
                {
                    messageToSerialize.AddSendPayload(Encoding.ASCII.GetBytes(messageend));
                }

                if (replaceNeeded)
                {
                    ReplaceSerialize(messageToSerialize, lineend, message);
                }
            }
        }
    }

}   // namespace finalmq
