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

    public class RemoteEntityFormatProto : IRemoteEntityFormat
    {
        static readonly int CONTENT_TYPE = 1;
        static readonly string CONTENT_TYPE_NAME = "protobuf";

        static readonly int PROTOBUFBLOCKSIZE = 512;
        static readonly string WILDCARD = "*";


#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void RegisterFormat()
        {
            RemoteEntityFormatRegistry.Instance.RegisterFormat(CONTENT_TYPE_NAME, CONTENT_TYPE, new RemoteEntityFormatProto());
        }


        public StructBase? Parse(IProtocolSession session, BufferRef bufferRef, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus)
        {
            formatStatus = 0;
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;
            if (sizeBuffer < 4)
            {
//                streamError << "buffer size too small: " << sizeBuffer;
                return null;
            }

            int sizePayload = sizeBuffer - 4;
            int sizeHeader = 0;
            if (sizeBuffer >= 4)
            {
                sizeHeader = (int)buffer[offset];
                ++offset;
                sizeHeader |= (int)buffer[offset] << 8;
                ++offset;
                sizeHeader |= (int)buffer[offset] << 16;
                ++offset;
                sizeHeader |= (int)buffer[offset] << 24;
                ++offset;
            }
            bool ok = false;

            if (sizeHeader <= sizePayload)
            {
                SerializerStruct serializerHeader = new SerializerStruct(header);
                ParserProto parserHeader = new ParserProto(serializerHeader, buffer, offset, sizeHeader);
                ok = parserHeader.ParseStruct(typeof(Header).FullName!);
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

            if (ok)
            {
                int sizeData = sizePayload - sizeHeader;
                offset += sizeHeader;
                Debug.Assert(sizeData >= 0);

                BufferRef bufferRefData = new BufferRef( buffer, offset, sizeData );
                data = ParseData(session, bufferRefData, storeRawData, header.type, out formatStatus);
            }

            return data;
        }
        public StructBase? ParseData(IProtocolSession session, BufferRef bufferRef, bool storeRawData, string type, out int formatStatus)
        {
            formatStatus = 0;
            byte[] buffer = bufferRef.Buffer;
            int offset = bufferRef.Offset;
            int sizeBuffer = bufferRef.Length;
            if (sizeBuffer < 4)
            {
//todo                streamError << "buffer size too small: " << sizeBuffer;
                return null;
            }

            StructBase? data = null;

            if (type.Length != 0)
            {
                bool ok = true;
                int sizeRemaining = sizeBuffer;

                if (sizeBuffer < 4)
                {
                    ok = false;
                }

                int sizeDataInStream = 0;
                if (ok)
                {
                    sizeDataInStream = (int)buffer[offset];
                    ++offset;
                    sizeDataInStream |= (int)buffer[offset] << 8;
                    ++offset;
                    sizeDataInStream |= (int)buffer[offset] << 16;
                    ++offset;
                    sizeDataInStream |= (int)buffer[offset] << 24;
                    ++offset;
                    sizeRemaining -= 4;
                }

                if (sizeDataInStream > sizeRemaining)
                {
                    ok = false;
                }

                if (ok)
                {
                    if (type.Length != 0)
                    {
                        data = TypeRegistry.Instance.CreateStruct(type);
                        Debug.Assert(sizeBuffer >= 0);

                        if (data != null)
                        {
                            if (sizeDataInStream > 0)
                            {
                                Debug.Assert(sizeDataInStream >= 0);
                                SerializerStruct serializerData = new SerializerStruct(data);
                                ParserProto parserData = new ParserProto(serializerData, buffer, offset, sizeDataInStream);
                                ok = parserData.ParseStruct(type);
                                if (!ok)
                                {
                                    formatStatus |= (int)FormatStatus.FORMATSTATUS_SYNTAX_ERROR;
                                    data = null;
                                }
                            }
                        }
                    }

                    if (storeRawData)
                    {
                        if (data == null)
                        {
                            data = new RawDataMessage();
                        }
                        data.SetRawData(type, CONTENT_TYPE, new BufferRef(bufferRef.Buffer, offset, sizeDataInStream));
                    }
                }
            }

            return data;
        }
        public void Serialize(IProtocolSession session, IMessage message, Header header, StructBase? structBase = null)
        {
            BufferRef bufferSizeHeader = message.AddSendPayload(4, PROTOBUFBLOCKSIZE);

            SerializerProto serializerHeader = new SerializerProto(message, PROTOBUFBLOCKSIZE);
            ParserStruct parserHeader = new ParserStruct(serializerHeader, header);
            parserHeader.ParseStruct();
            int sizeHeader = message.GetTotalSendPayloadSize() - 4;
            Debug.Assert(sizeHeader >= 0);
            uint usizeHeader = (uint)sizeHeader;

            bufferSizeHeader[0] = (byte)usizeHeader;
            bufferSizeHeader[1] = (byte)(usizeHeader >> 8);
            bufferSizeHeader[2] = (byte)(usizeHeader >> 16);
            bufferSizeHeader[3] = (byte)(usizeHeader >> 24);

            SerializeData(session, message, structBase);
        }
        public void SerializeData(IProtocolSession session, IMessage message, StructBase? structBase = null)
        {
            BufferRef bufferSizePayload = message.AddSendPayload(4, PROTOBUFBLOCKSIZE);
            int sizePayload = 0;
            if (structBase != null)
            {
                int sizeStart = message.GetTotalSendPayloadSize();
                if (structBase.GetRawContentType() == CONTENT_TYPE)
                {
                    BufferRef? rawData = structBase.GetRawData();
                    Debug.Assert(rawData != null);
                    message.AddSendPayload(rawData);
                }
                else
                {
                    SerializerProto serializerData = new SerializerProto(message, PROTOBUFBLOCKSIZE);
                    ParserStruct parserData = new ParserStruct(serializerData, structBase);
                    parserData.ParseStruct();
                }
                int sizeEnd = message.GetTotalSendPayloadSize();
                sizePayload = sizeEnd - sizeStart;
            }
            Debug.Assert(sizePayload >= 0);
            uint uSizePayload = (uint)sizePayload;
            bufferSizePayload[0] = (byte)uSizePayload;
            bufferSizePayload[1] = (byte)(uSizePayload >> 8);
            bufferSizePayload[2] = (byte)(uSizePayload >> 16);
            bufferSizePayload[3] = (byte)(uSizePayload >> 24);
        }
    }

}   // namespace finalmq
