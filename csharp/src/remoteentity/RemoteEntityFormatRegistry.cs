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
using System.Text;

namespace finalmq
{
    using VariantStruct = IList<NameValue>;

    enum FormatStatus
    {
        FORMATSTATUS_NONE = 0,
        FORMATSTATUS_SYNTAX_ERROR = 1,
        FORMATSTATUS_AUTOMATIC_CONNECT = 2,
        FORMATSTATUS_HEADER_PARSED_BY_FORMAT = 4,
    };


    public interface IRemoteEntityFormat
    {
        StructBase? Parse(IProtocolSession session, BufferRef bufferRef, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus);
        StructBase? ParseData(IProtocolSession session, BufferRef bufferRef, bool storeRawData, string type, ref int formatStatus);
        void Serialize(IProtocolSession session, IMessage message, Header header, StructBase? structBase = null);
        void SerializeData(IProtocolSession session, IMessage message, StructBase? structBase = null);
    };


    public interface IRemoteEntityFormatRegistry
    {
        StructBase? Parse(IProtocolSession session, IMessage message, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus);
        StructBase? ParseHeaderInMetainfo(IProtocolSession session, IMessage message, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus);
        void Send(IProtocolSession session, string virtualSessionId, Header header, Variant echoData, StructBase? structBase = null, Metainfo? metainfo = null, Variant? controlData = null);

        void RegisterFormat(string contentTypeName, int contentType, IRemoteEntityFormat format);
        bool IsRegistered(int contentType);
        int GetContentType(string contentTypeName);
    };

    class RemoteEntityFormatRegistryImpl : IRemoteEntityFormatRegistry
    {
        static readonly string FMQ_HTTP = "fmq_http";
        static readonly string FMQ_METHOD = "fmq_method";
        static readonly string FMQ_HTTP_STATUS = "fmq_http_status";
        static readonly string FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
        static readonly string HTTP_RESPONSE = "response";
        static readonly string FMQ_PATH = "fmq_path";
        static readonly string FMQ_DESTNAME = "fmq_destname";
        static readonly string FMQ_DESTID = "fmq_destid";
        static readonly string FMQ_SRCID = "fmq_srcid";
        static readonly string FMQ_MODE = "fmq_mode";
        static readonly string FMQ_CORRID = "fmq_corrid";
        static readonly string FMQ_STATUS = "fmq_status";
        static readonly string FMQ_SUBPATH = "fmq_subpath";
        static readonly string FMQ_TYPE = "fmq_type";
        static readonly string MSG_REPLY = "MSG_REPLY";

        static readonly string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";

        public StructBase? Parse(IProtocolSession session, IMessage message, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus)
        {
            formatStatus = 0;
            BufferRef bufferRef = message.GetReceivePayload();

            StructBase? structBase = null;
            IRemoteEntityFormat? format;

            int contentType = session.ContentType;
            m_contentTypeToFormat.TryGetValue(contentType, out format);
            if (format != null)
            {
                structBase = format.Parse(session, bufferRef, storeRawData, name2Entity, header, out formatStatus);
                MetainfoToMessage(message, header.meta);
            }

            return structBase;
        }
        public StructBase? ParseHeaderInMetainfo(IProtocolSession session, IMessage message, bool storeRawData, IDictionary<string, IRemoteEntity> name2Entity, Header header, out int formatStatus)
        {
            BufferRef? data = ParseMetainfo(message, name2Entity, header);

            formatStatus = 0;
            BufferRef bufferRef = message.GetReceivePayload();

            StructBase? structBase = null;

            if (header.type != typeof(RawBytes).GetType().FullName)
            {
                // special feature for the browser: json data can be written into the path
                if (bufferRef.Length == 0 && data != null && data.Length != 0)
                {
                    bufferRef = data;
                }

                IRemoteEntityFormat? format;
                int contentType = session.ContentType;
                m_contentTypeToFormat.TryGetValue(contentType, out format);
                if (format != null)
                {
                    structBase = format.ParseData(session, bufferRef, storeRawData, header.type, ref formatStatus);
                }
            }
            else
            {
                RawBytes structRawBytes = new RawBytes();
                if (bufferRef.Offset == 0 && bufferRef.Length == bufferRef.Buffer.Length)
                {
                    structRawBytes.data = bufferRef.Buffer;
                }
                else
                {
                    structRawBytes.data = new byte[bufferRef.Length];
                    Array.Copy(bufferRef.Buffer, bufferRef.Offset, structRawBytes.data, 0, bufferRef.Length);
                }
                structBase = structRawBytes;
            }

            return structBase;
        }
        private static bool ShallSend(Header header, IProtocolSession session)
        {
            if ((header.mode != MsgMode.MSG_REPLY) ||
                (header.corrid != IRemoteEntity.CORRELATIONID_NONE) ||
                session.NeedsReply())
            {
                return true;
            }
            return false;
        }

        static void StatusToProtocolStatus(Status status, Variant controlData, Metainfo? metainfo, IProtocolSession session)
        {
            controlData.Add(FMQ_HTTP, HTTP_RESPONSE);
            switch (status)
            {
            case Status.STATUS_OK:
                {
                    bool statusOk = true;
                    if (metainfo != null)
                    {
                        string? statusnumber;
                        metainfo.TryGetValue(FMQ_HTTP_STATUS, out statusnumber);
                        if (statusnumber != null)
                        {
                            string? statustext;
                            statusOk = false;
                            metainfo.TryGetValue(FMQ_HTTP_STATUSTEXT, out statustext);
                            if (statustext == null || statustext.Length == 0)
                            {
                                    statustext = "_";
                            }
                            controlData.Add(FMQ_HTTP_STATUS, statusnumber);
                            controlData.Add(FMQ_HTTP_STATUSTEXT, statustext);
                        }
                    }
                    if (statusOk)
                    {
                        controlData.Add(FMQ_HTTP_STATUS, 200);
                        controlData.Add(FMQ_HTTP_STATUSTEXT, "OK");
                    }
                }
                break;
            case Status.STATUS_ENTITY_NOT_FOUND:
            case Status.STATUS_REQUEST_NOT_FOUND:
            case Status.STATUS_REQUESTTYPE_NOT_KNOWN:
                controlData.Add(FMQ_HTTP_STATUS, 404);
                controlData.Add(FMQ_HTTP_STATUSTEXT, "Not Found");
                break;
            case Status.STATUS_SYNTAX_ERROR:
                controlData.Add(FMQ_HTTP_STATUS, 400);
                controlData.Add(FMQ_HTTP_STATUSTEXT, "Bad Request");
                break;
            case Status.STATUS_NO_REPLY:
                if (session.NeedsReply())
                {
                    controlData.Add(FMQ_HTTP_STATUS, 200);
                    controlData.Add(FMQ_HTTP_STATUSTEXT, "OK");
                }
                else
                {
                    controlData.Add(FMQ_HTTP_STATUS, 500);
                    controlData.Add(FMQ_HTTP_STATUSTEXT, "Internal Server Error");
                }
                break;
            default:
                controlData.Add(FMQ_HTTP_STATUS, 500);
                controlData.Add(FMQ_HTTP_STATUSTEXT, "Internal Server Error");
                break;
            }
        }

        static void MetainfoToHeader(Header header, Metainfo metainfo)
        {
            foreach (var entry in metainfo)
            {
                header.meta.Add(entry.Key);
                header.meta.Add(entry.Value);
            }
            metainfo.Clear();
        }



        public void Send(IProtocolSession session, string virtualSessionId, Header header, Variant echoData, StructBase? structBase = null, Metainfo? metainfo = null, Variant? controlData = null)
        {
            if (ShallSend(header, session))
            {
                byte[]? pureData = null;
                IMessage message = session.CreateMessage();
                if (header.mode == MsgMode.MSG_REPLY)
                {
                    message.EchoData = echoData;
                }
                if (header.destname.Length != 0)
                {
                    Variant controlDataTmp = message.ControlData;
                    controlDataTmp.Add(FMQ_DESTNAME, header.destname);
                }
                if (virtualSessionId.Length != 0)
                {
                    Variant controlDataTmp = message.ControlData;
                    controlDataTmp.Add(FMQ_VIRTUAL_SESSION_ID, virtualSessionId);
                }
                bool writeMetainfoToHeader = (metainfo != null);
                if (session.DoesSupportMetainfo())
                {
                    if (header.mode == MsgMode.MSG_REPLY)
                    {
                        Variant controlDataTmp = message.ControlData;
                        StatusToProtocolStatus(header.status, controlDataTmp, metainfo, session);
                        RawBytes? rawBytes = structBase as RawBytes;
                        if (rawBytes != null)
                        {
                            pureData = rawBytes.data;
                        }
                    }

                    // not a poll request
                    if (metainfo != null && (!session.IsSendRequestByPoll() || header.mode == MsgMode.MSG_REPLY))
                    {
                        Metainfo metainfoMessage = message.AllMetainfo;
                        foreach (var entry in metainfo)
                        {
                            metainfoMessage[entry.Key] = metainfo[entry.Key];
                        }
                        metainfo.Clear();
                        writeMetainfoToHeader = false;
                    }

                    SerializeHeaderToMetainfo(message, header);
                }

                if (writeMetainfoToHeader && metainfo != null)
                {
                    MetainfoToHeader(header, metainfo);
                }

                if (controlData != null)
                {
                    VariantStruct varstruct = message.ControlData.GetData<VariantStruct>();
                    VariantStruct varstruct2 = controlData.GetData<VariantStruct>();
                    foreach (var entry in varstruct2)
                    {
                        varstruct.Add(entry);
                    }
                    varstruct2.Clear();
                }
                if (pureData == null)
                {
                    bool ok = false;
                    if (!session.DoesSupportMetainfo() || (session.IsSendRequestByPoll() && header.mode == MsgMode.MSG_REQUEST))
                    {
                        ok = Serialize(session, message, header, structBase);
                    }
                    else
                    {
                        ok = SerializeData(session, message, structBase);
                    }
                    Debug.Assert(ok);   // "Could not send, because of error in serialization (content type). should never happen, because bind() or connect() have failed before."
                }
                else
                {
                    message.AddSendPayload(pureData);
                }
                if (virtualSessionId.Length != 0)
                {
                    message.ControlData.Add(FMQ_VIRTUAL_SESSION_ID, virtualSessionId);
                }
                session.SendMessage(message, (header.mode == MsgMode.MSG_REPLY));
            }
        }
        public void RegisterFormat(string contentTypeName, int contentType, IRemoteEntityFormat format)
        {
            m_contentTypeToFormat[contentType] = format;
            m_contentTypeNameToContentType[contentTypeName] = contentType;
        }
        public bool IsRegistered(int contentType)
        {
            return m_contentTypeToFormat.TryGetValue(contentType, out var result);
        }
        public int GetContentType(string contentTypeName)
        {
            m_contentTypeNameToContentType.TryGetValue(contentTypeName, out var result);
            return result;  // if not found, return 0
        }

        static bool IsDestinationDefined(Header header)
        {
            return (header.destname.Length != 0 || (header.destid != IRemoteEntity.ENTITYID_INVALID && header.destid != IRemoteEntity.ENTITYID_DEFAULT));
        }

        static bool IsSubPathDefined(Header header)
        {
            return (header.path.Length != 0);
        }

        static bool IsDestAndSubPathDefined(Header header)
        {
            return (IsDestinationDefined(header) && IsSubPathDefined(header));
        }


        void SerializeHeaderToMetainfo(IMessage message, Header header)
        {
            Metainfo metainfo = message.AllMetainfo;
            if (header.destname.Length != 0)
            {
                metainfo[FMQ_DESTNAME] = header.destname;
            }
            if (header.destid != 0)
            {
                metainfo[FMQ_DESTID] = header.destid.ToString();
            }
            metainfo[FMQ_SRCID] = header.srcid.ToString();
            metainfo[FMQ_MODE] = header.mode.ToString();
            if (header.corrid != 0)
            {
                metainfo[FMQ_CORRID] = header.corrid.ToString();
            }
            metainfo[FMQ_STATUS] = header.status.ToString();
            if (header.path.Length != 0)
            {
                metainfo[FMQ_SUBPATH] = header.path;
            }
            metainfo[FMQ_TYPE] = header.type;
        }

        static readonly string WILDCARD = "*";

        BufferRef? ParseMetainfo(IMessage message, IDictionary<string, IRemoteEntity> name2Entity, Header header)
        {
            Metainfo metainfo = message.AllMetainfo;
            metainfo.TryGetValue(FMQ_PATH, out var path);
            metainfo.TryGetValue(FMQ_SRCID, out var srcid);
            metainfo.TryGetValue(FMQ_MODE, out var mode);
            metainfo.TryGetValue(FMQ_CORRID, out var corrid);
            metainfo.TryGetValue(FMQ_STATUS, out var status);
            metainfo.TryGetValue(FMQ_DESTNAME, out var destname);
            metainfo.TryGetValue(FMQ_DESTID, out var destid);
            metainfo.TryGetValue(FMQ_SUBPATH, out var subpath);
            metainfo.TryGetValue(FMQ_TYPE, out var type);

            if (srcid != null)
            {
                try
                {
                    header.srcid = UInt64.Parse(srcid);
                }
                catch (Exception)
                {
                }
            }

            if (mode != null)
            {
                if (mode == MSG_REPLY)
                {
                    header.mode = MsgMode.MSG_REPLY;
                }
                else
                {
                    header.mode = MsgMode.MSG_REQUEST;
                }
            }

            if (corrid != null)
            {
                try
                {
                    header.corrid = UInt64.Parse(corrid);
                }
                catch (Exception)
                {
                }
            }

            if (status != null)
            {
                if (Enum.TryParse<finalmq.Status>(status, out var enStatus))
                {
                    header.status = enStatus;
                }
            }

            if (destname != null)
            {
                header.destname = destname;
            }

            if (destid != null)
            {
                try
                {
                    header.destid = UInt64.Parse(destid);
                }
                catch (Exception)
                {
                }
            }

            if (type != null)
            {
                header.type = type;
            }

            if (subpath != null)
            {
                header.path = subpath;
            }

            IRemoteEntity? remoteEntity = null;
            BufferRef? data = null;
            if (path != null && !IsDestAndSubPathDefined(header))
            {
                // 012345678901234567890123456789
                // /MyServer/test.TestRequest!1{}
                string pathWithoutFirstSlash;
                if (path[0] == '/')
                {
                    pathWithoutFirstSlash = path.Substring(1);
                }
                else
                {
                    pathWithoutFirstSlash = path;
                }
                int ixEndHeader = pathWithoutFirstSlash.IndexOf('{');   //28
                if (ixEndHeader == -1)
                {
                    ixEndHeader = pathWithoutFirstSlash.Length;
                }
                data = new BufferRef(Encoding.UTF8.GetBytes(pathWithoutFirstSlash, ixEndHeader, pathWithoutFirstSlash.Length - ixEndHeader), 0);

                string? foundEntityName = null;
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
                        break;
                    }
                }
                if (foundEntityName != null)
                {
                    header.destname = pathWithoutFirstSlash.Substring(0, foundEntityName.Length);
                    if (pathWithoutFirstSlash.Length > foundEntityName.Length)
                    {
                        header.path = pathWithoutFirstSlash.Substring(foundEntityName.Length + 1, ixEndHeader - foundEntityName.Length - 1);
                    }
                }
                else
                {
                    header.destname = "*";
                    header.path = pathWithoutFirstSlash;
                    name2Entity.TryGetValue(header.destname, out remoteEntity);
                }
            }

            if (header.type.Length == 0)
            {
                var entity = remoteEntity;
                if (entity != null)
                {
                    name2Entity.TryGetValue(header.destname, out entity);
                }
                if (entity != null)
                {
                    metainfo.TryGetValue(FMQ_METHOD, out var method);
                    string pathTemp = header.path;
                    header.type = entity.GetTypeOfCommandFunction(ref pathTemp, method);
                    header.path = pathTemp;
                }
            }

            if (header.path.Length == 0)
            {
                header.path = header.type;
            }

            return data;
        }
        bool Serialize(IProtocolSession session, IMessage message, Header header, StructBase? structBase = null)
        {
            int contentType = session.ContentType;
            m_contentTypeToFormat.TryGetValue(contentType, out var format);
            if (format != null)
            {
                format.Serialize(session, message, header, structBase);
                return true;
            }
//todo            streamError << "ContentType not found: " << contentType;
            return false;
        }

        bool SerializeData(IProtocolSession session, IMessage message, StructBase? structBase)
        {
            int contentType = session.ContentType;
            m_contentTypeToFormat.TryGetValue(contentType, out var format);
            if (format != null)
            {
                format.SerializeData(session, message, structBase);
                return true;
            }
//todo            streamError << "ContentType not found: " << contentType;
            return false;
        }

        static void MetainfoToMessage(IMessage message, IList<string> meta)
        {
            int size = meta.Count;
            if (size > 0)
            {
                --size;
            }
            Metainfo metainfo = message.AllMetainfo;
            for (int i = 0; i < size; i += 2)
            {
                metainfo[meta[i]] = meta[i + 1];
            }
        }


        readonly IDictionary<int, IRemoteEntityFormat> m_contentTypeToFormat = new Dictionary<int, IRemoteEntityFormat>();
        readonly IDictionary<string, int> m_contentTypeNameToContentType = new Dictionary<string, int>();
    };

    public class RemoteEntityFormatRegistry
    {
        public static IRemoteEntityFormatRegistry Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }
        private RemoteEntityFormatRegistry()
        {
        }

        private static IRemoteEntityFormatRegistry m_instance = new RemoteEntityFormatRegistryImpl();
    };
    

}   // namespace finalmq
