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
        public static readonly string PROPERTY_SERIALIZE_ENUM_AS_STRING = "enumAsSt";


#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void RegisterFormat()
        {
            RemoteEntityFormatRegistry.Instance.RegisterFormat(CONTENT_TYPE_NAME, CONTENT_TYPE, new RemoteEntityFormatHl7());
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
                data = ParseData(session, bufferRefData, storeRawData, header.type, out formatStatus);

                formatStatus |= (int)FormatStatus.FORMATSTATUS_AUTOMATIC_CONNECT;
            }

            return data;
        }
        public StructBase? ParseData(IProtocolSession session, BufferRef bufferRef, bool storeRawData, string type, out int formatStatus)
        {
            formatStatus = 0;
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
                    data.SetRawData(type, CONTENT_TYPE, bufferRef);
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
                    Variant? formatData = session.FormatData;
                    if (formatData != null && formatData.VarType != Variant.VARTYPE_NONE)
                    {
                        Variant? propEnumAsString = formatData.GetVariant(PROPERTY_SERIALIZE_ENUM_AS_STRING);
                        if (propEnumAsString != null)
                        {
                            enumAsString = propEnumAsString;
                        }
                    }

                    SerializerHl7 serializerData = new SerializerHl7(message, 512, enumAsString);
                    ParserStruct parserData = new ParserStruct(serializerData, structBase);
                    parserData.ParseStruct();
                }
            }
        }
    }

}   // namespace finalmq
