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

    public class RemoteEntityFormatJson : IRemoteEntityFormat
    {
        static readonly int CONTENT_TYPE = 2;
        static readonly string CONTENT_TYPE_NAME = "json";

        public static readonly string PROPERTY_SERIALIZE_ENUM_AS_STRING = "enumAsSt";
        public static readonly string PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES = "skipDefVal";

        static readonly int JSONBLOCKSIZE = 512;
        static readonly byte[] ARRAY_BEGIN = { (byte)'[' };
        static readonly byte[] ARRAY_END = { (byte)']', (byte)'\t' };
        static readonly byte[] ARRAY_END_NO_DATA = { (byte)',', (byte)'\t', (byte)'{', (byte)'}', (byte)']', (byte)'\t' };
        static readonly byte[] DELIMITER_HEADER_PAYLOAD = { (byte)',', (byte)'\t' };
        static readonly byte[] EMPTY_PAYLOAD = { (byte)'{', (byte)'}' };

        static readonly string WILDCARD = "*";

        static readonly string FMQ_PATH = "fmq_path";


#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void RegisterFormat()
        {
            RemoteEntityFormatRegistry.Instance.RegisterFormat(CONTENT_TYPE_NAME, CONTENT_TYPE, new RemoteEntityFormatJson());
        }

        static int FindEndOfPath(byte[] buffer, int offset, int size)
        {
            int i = offset;
            int iEnd = offset + size;
            while (i < iEnd)
            {
                byte c = buffer[i];
                if (c == '{')
                {
                    return i;
                }
                ++i;
            }
            return i;
        }

        public StructBase? Parse(IProtocolSession session, BufferRef bufferRef, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus)
        {
            formatStatus = 0;
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;

            if (sizeBuffer == 0)
            {
                return null;
            }

            if (sizeBuffer > 0 && buffer[offset + 0] == '[')
            {
                ++offset;
                --sizeBuffer;
            }
            if (sizeBuffer > 0 && buffer[offset + sizeBuffer - 1] == ']')
            {
                --sizeBuffer;
            }

            int endHeader = -1;
            if (buffer[offset + 0] == '/')
            {
                // 012345678901234567890123456789
                // /MyServer/test.TestRequest!1{}
                int ixEndHeader = FindEndOfPath(buffer, offset, sizeBuffer);   //28
                endHeader = ixEndHeader;

                string strHeader = Encoding.UTF8.GetString(buffer, offset, ixEndHeader);

                int ixCorrelationId = strHeader.LastIndexOf('!');
                if (ixCorrelationId != -1)
                {
                    try
                    {
                        header.corrid = UInt64.Parse(strHeader.Substring(ixCorrelationId + 1));
                    }
                    catch (Exception)
                    {
                    }
                }
                else
                {
                    ixCorrelationId = ixEndHeader;
                }

                string pathWithoutFirstSlash = strHeader.Substring(1, ixCorrelationId - 1);
                pathWithoutFirstSlash = pathWithoutFirstSlash.Trim();
                string? foundEntityName = null;
                IRemoteEntity? remoteEntity = null;
                foreach (var entry in name2Entity)
                {
                    string prefix = entry.Key;
                    if (prefix != WILDCARD &&
                        pathWithoutFirstSlash.Length >= prefix.Length &&
                        pathWithoutFirstSlash.StartsWith(prefix) &&
                        (pathWithoutFirstSlash.Length == prefix.Length || pathWithoutFirstSlash[prefix.Length] == '/'))
                    {
                        foundEntityName = prefix;
                        remoteEntity = entry.Value;
                    }
                }
                if (foundEntityName != null)
                {
                    header.destname = foundEntityName;  // pathWithoutFirstSlash.Substring(0, foundEntityName.Length);
                    if (pathWithoutFirstSlash.Length > foundEntityName.Length)
                    {
                        header.path = pathWithoutFirstSlash.Substring(foundEntityName.Length + 1, pathWithoutFirstSlash.Length - foundEntityName.Length - 1);
                    }
                }
                else
                {
                    header.destname = "*";
                    header.path = pathWithoutFirstSlash;
                    name2Entity.TryGetValue(header.destname, out remoteEntity);
                }
                if (remoteEntity != null)
                {
                    string pathTemp = header.path;
                    header.type = remoteEntity.GetTypeOfCommandFunction(ref pathTemp);
                    header.path = pathTemp;
                }

                string path = strHeader.Substring(0, ixCorrelationId);
                path = path.Trim();
                header.meta.Add(FMQ_PATH);
                header.meta.Add(path);

                header.mode = MsgMode.MSG_REQUEST;
            }
            else
            {
                SerializerStruct serializerHeader = new SerializerStruct(header);
                ParserJson parserHeader = new ParserJson(serializerHeader, buffer, offset, sizeBuffer);
                endHeader = parserHeader.ParseStruct(header.GetType().FullName!);
                if (endHeader != -1)
                {
                    // skip comma
                    ++endHeader;
                }
                if (header.type.Length == 0 && header.path.Length != 0)
                {
                    name2Entity.TryGetValue(header.destname, out var remoteEntity);
                    if (remoteEntity == null)
                    {
                        name2Entity.TryGetValue(WILDCARD, out remoteEntity);
                    }
                    if (remoteEntity != null)
                    {
                        string pathTemp = header.path;
                        header.type = remoteEntity.GetTypeOfCommandFunction(ref pathTemp);
                        header.path = pathTemp;
                    }
                }
                if (header.path.Length == 0 && header.type.Length != 0)
                {
                    header.path = header.type;
                }
                else if (header.path.Length != 0 && header.type.Length == 0)
                {
                    header.type = header.path;
                }                
            }
            
            StructBase? data = null;

            if (endHeader != -1)
            {
                int sizeHeader = endHeader - offset;
                Debug.Assert(sizeHeader >= 0);
                offset += sizeHeader;
                int sizeData = sizeBuffer - sizeHeader;
                Debug.Assert(sizeData >= 0);

                BufferRef bufferRefData = new BufferRef( buffer, offset, sizeData );
                data = ParseData(session, bufferRefData, storeRawData, header.type, ref formatStatus);
            }

            return data;
        }
        public StructBase? ParseData(IProtocolSession session, BufferRef bufferRef, bool storeRawData, string type, ref int formatStatus)
        {
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;

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
                        ParserJson parserData = new ParserJson(serializerData, buffer, offset, sizeBuffer);
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
                    data.SetRawData(type, CONTENT_TYPE, bufferRef);
                }
            }

            return data;
        }
        public void Serialize(IProtocolSession session, IMessage message, Header header, StructBase? structBase = null)
        {
            message.AddSendPayload(ARRAY_BEGIN, JSONBLOCKSIZE);

            SerializerJson serializerHeader = new SerializerJson(message, JSONBLOCKSIZE);
            ParserStruct parserHeader = new ParserStruct(serializerHeader, header);
            parserHeader.ParseStruct();

            // add end of header
            if (structBase != null)
            {
                // delimiter between header and payload
                message.AddSendPayload(DELIMITER_HEADER_PAYLOAD, JSONBLOCKSIZE);

                SerializeData(session, message, structBase);

                message.AddSendPayload(ARRAY_END, JSONBLOCKSIZE);
            }
            else
            {
                message.AddSendPayload(ARRAY_END_NO_DATA);
            }
        }
        public void SerializeData(IProtocolSession session, IMessage message, StructBase? structBase = null)
        {
            if (structBase != null)
            {
                // payload
                if (structBase.GetRawContentType() == CONTENT_TYPE)
                {
                    BufferRef? rawData = structBase.GetRawData();
                    Debug.Assert(rawData != null);
                    message.AddSendPayload(rawData);
                }
                else
                {
                    bool enumAsString = true;
                    bool skipDefaultValues = false;
                    Variant? formatData = session.FormatData;
                    if (formatData != null && formatData.VarType != Variant.VARTYPE_NONE)
                    {
                        Variant? propEnumAsString = formatData.GetVariant(PROPERTY_SERIALIZE_ENUM_AS_STRING);
                        Variant? propSkipDefaultValues = formatData.GetVariant(PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES);
                        if (propEnumAsString != null)
                        {
                            enumAsString = propEnumAsString;
                        }
                        if (propSkipDefaultValues != null)
                        {
                            skipDefaultValues = propSkipDefaultValues;
                        }
                    }

                    SerializerJson serializerData = new SerializerJson(message, JSONBLOCKSIZE, enumAsString, skipDefaultValues);
                    ParserStruct parserData = new ParserStruct(serializerData, structBase);
                    parserData.ParseStruct();
                }
            }
            else
            {
                message.AddSendPayload(EMPTY_PAYLOAD);
            }
        }
    }

}   // namespace finalmq
