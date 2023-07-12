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
using System.Runtime.CompilerServices;
using System.Text;

namespace finalmq
{
    using VariantStruct = IList<NameValue>;

    public class ProtocolHttpServer : IProtocol
    {
        public static readonly uint PROTOCOL_ID = 4;
        public static readonly string PROTOCOL_NAME = "httpserver";

        public static readonly string FMQ_HTTP = "fmq_http";
        public static readonly string FMQ_METHOD = "fmq_method";
        public static readonly string FMQ_PROTOCOL = "fmq_protocol";
        public static readonly string FMQ_PATH = "fmq_path";
        public static readonly string FMQ_QUERY_PREFIX = "QUERY_";
        public static readonly string FMQ_HTTP_STATUS = "fmq_http_status";
        public static readonly string FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
        public static readonly string HTTP_REQUEST = "request";
        public static readonly string HTTP_RESPONSE = "response";

        static readonly string CONTENT_LENGTH = "Content-Length";
        static readonly string FMQ_SESSIONID = "fmq_sessionid";
        static readonly string HTTP_COOKIE = "Cookie";

        static readonly string FMQ_CREATESESSION = "fmq_createsession";
        static readonly string FMQ_SET_SESSION = "fmq_setsession";
        static readonly string HTTP_SET_COOKIE = "Set-Cookie";
        static readonly string COOKIE_PREFIX = "fmq=";

        static readonly string FMQ_PATH_POLL = "/fmq/poll";
        static readonly string FMQ_PATH_PING = "/fmq/ping";
        static readonly string FMQ_PATH_CONFIG = "/fmq/config";
        static readonly string FMQ_PATH_CREATESESSION = "/fmq/createsession";
        static readonly string FMQ_PATH_REMOVESESSION = "/fmq/removesession";
        static readonly byte[] FMQ_MULTIPART_BOUNDARY = Encoding.ASCII.GetBytes("B9BMAhxAhY.mQw1IDRBA");
        static readonly byte[] CR_LF_MINUS_MINUS_MULTIPART_BOUNDARY = Encoding.ASCII.GetBytes("\r\n--B9BMAhxAhY.mQw1IDRBA");
        static readonly byte[] MINUS_MINUS_MULTIPART_BOUNDARY_MINUS_MINUS_CR_LF = Encoding.ASCII.GetBytes("--B9BMAhxAhY.mQw1IDRBA--\r\n");
        static readonly byte[] MINUS_MINUS_CR_LF = Encoding.ASCII.GetBytes("--\r\n");
        static readonly string FILE_NOT_FOUND = "file not found";
        static readonly byte[] HEADER_KEEP_ALIVE = Encoding.ASCII.GetBytes("Connection: keep-alive\r\n");

        static readonly byte[] CR_LF = Encoding.ASCII.GetBytes("\r\n");
        static readonly byte[] COLON = Encoding.ASCII.GetBytes(": ");
        static readonly byte[] POLL_STOP = Encoding.ASCII.GetBytes("0\r\n\r\n");
        static readonly byte[] SPACE_HTTP_VERSION = Encoding.ASCII.GetBytes(" HTTP/1.1");
        static readonly byte[] HTTP_VERSION_SPACE = Encoding.ASCII.GetBytes("HTTP/1.1 ");
        static readonly byte[] NUMBER_200_SPACE = Encoding.ASCII.GetBytes("200 ");
        static readonly byte[] BYTES_OK = Encoding.ASCII.GetBytes("OK");
        static readonly byte[] MINUS_MINUS = Encoding.ASCII.GetBytes("--");
        static readonly byte[] CR_LF_CR_LF = Encoding.ASCII.GetBytes("\r\n\r\n");

        static readonly string FORMAT_X8 = "X8";
        static readonly string FORMAT_X = "X";

        enum ChunkedState
        {
            STATE_STOP = 0,
            STATE_START_CHUNK_STREAM = 1,
            STATE_FIRST_CHUNK = 2,
            STATE_BEGIN_MULTIPART = 3,
            STATE_CONTINUE = 4,
        };

        public ProtocolHttpServer()
        {

        }
        ~ProtocolHttpServer()
        {
            if (m_connection != null)
            {
                m_connection.Disconnect();
            }
        }

        // IStreamConnectionCallback
        public IStreamConnectionCallback? Connected(IStreamConnection connection)
        {
            ConnectionData connectionData = connection.ConnectionData;
            m_headerHost = Encoding.ASCII.GetBytes("Host: " + connectionData.Hostname + ":" + connectionData.Port.ToString() + "\r\n");
            m_connectionId = connectionData.ConnectionId;
            // for incomming connection -> do the connection event after checking the session ID
            if (connectionData.IncomingConnection)
            if (connectionData.IncomingConnection)
            {
                //        m_checkSessionName = true;
            }
            return null;
        }
        public void Disconnected(IStreamConnection connection)
        {
            var callback = m_callback;
            if (callback != null)
            {
                callback.Disconnected();
            }
        }

        bool HandleInternalCommands(IProtocolCallback callback, ref bool ok)
        {
            Debug.Assert(callback != null);
            bool handled = false;
            if (m_path != null)
            {
                if (m_path == FMQ_PATH_POLL)
                {
                    m_chunkedState = ChunkedState.STATE_START_CHUNK_STREAM;
                    Debug.Assert(m_message != null);
                    handled = true;
                    int timeout = -1;
                    int pollCountMax = 1;
                    string? strTimeout = m_message.GetMetainfo("QUERY_timeout");
                    string? strCount = m_message.GetMetainfo("QUERY_count");
                    string? strMultipart = m_message.GetMetainfo("QUERY_multipart");
                    if (strTimeout != null)
                    {
                        try
                        {
                            timeout = Int32.Parse(strTimeout);
                        }
                        catch(Exception)
                        {
                            timeout = -1;
                        }
                    }
                    if (strCount != null)
                    {
                        try
                        {
                            pollCountMax = Int32.Parse(strCount);
                        }
                        catch (Exception)
                        {
                            pollCountMax = -1;
                        }
                    }
                    if (strMultipart != null)
                    {
                        m_multipart = (strMultipart == "true") ? true : false;
                    }
                    IMessage message = MessageFactory();
                    string contentType;
                    if (m_multipart)
                    {
                        contentType = "multipart/x-mixed-replace; boundary=";
                        contentType += FMQ_MULTIPART_BOUNDARY;
                    }
                    else
                    {
                        contentType = "text/event-stream";
                    }
                    message.AddMetainfo("Content-Type", contentType);
                    message.AddMetainfo("Transfer-Encoding", "chunked");
                    SendMessage(message);
                    m_chunkedState = ChunkedState.STATE_FIRST_CHUNK;
                    callback.PollRequest(this, timeout, pollCountMax);
                }
                else if (m_path == FMQ_PATH_PING)
                {
                    handled = true;
                    Debug.Assert(m_connection != null);
                    SendMessage(MessageFactory());
                    callback.Activity();
                }
                else if (m_path == FMQ_PATH_CONFIG)
                {
                    Debug.Assert(m_message != null);
                    handled = true;
                    string? timeout = m_message.GetMetainfo("QUERY_activitytimeout");
                    if (timeout != null)
                    {
                        try
                        {
                            callback.ActivityTimeout = Int32.Parse(timeout);
                        }
                        catch (Exception)
                        {
                        }
                    }
                    string? pollMaxRequests = m_message.GetMetainfo("QUERY_pollmaxrequests");
                    if (pollMaxRequests != null)
                    {
                        try
                        {
                            callback.PollMaxRequests = Int32.Parse(pollMaxRequests);
                        }
                        catch (Exception)
                        {
                        }
                    }
                    SendMessage(MessageFactory());
                    callback.Activity();
                }
                else if (m_path == FMQ_PATH_CREATESESSION)
                {
                    handled = true;
                    SendMessage(MessageFactory());
                    callback.Activity();
                }
                else if (m_path == FMQ_PATH_REMOVESESSION)
                {
                    handled = true;
                    ok = false;
                    SendMessage(MessageFactory());
                    callback.Disconnected();
                }
            }

            return handled;
        }


        private void ResizeReceiveBuffer(int newSize)
        {
            byte[] bufferOld = m_receiveBuffer;
            m_receiveBuffer = new byte[newSize];
            if (bufferOld.Length > 0)
            {
                Array.Copy(bufferOld, 0, m_receiveBuffer, 0, Math.Min(bufferOld.Length, newSize));
            }
        }

        public bool Received(IStreamConnection connection, byte[] buffer, int count)
        {
            bool ok = true;

            if (m_state != State.STATE_CONTENT)
            {
                if (m_offsetRemaining == 0 || m_sizeRemaining == 0)
                {
                    ResizeReceiveBuffer(m_sizeRemaining + count);
                }
                else
                {
                    byte[] temp = m_receiveBuffer;
                    m_receiveBuffer = Array.Empty<byte>();
                    ResizeReceiveBuffer(m_sizeRemaining + count);
                    Array.Copy(temp, m_offsetRemaining, m_receiveBuffer, 0, m_sizeRemaining);
                }
                m_offsetRemaining = 0;

                Array.Copy(buffer, 0, m_receiveBuffer, m_sizeRemaining, count);

                ok = ReceiveHeaders(count);
                if (ok && m_state == State.STATE_CONTENT)
                {
                    Debug.Assert(m_message != null);
                    BufferRef payload = m_message.GetReceivePayload();
                    Debug.Assert(payload.Length == m_contentLength);
                    if (m_sizeRemaining <= m_contentLength)
                    {
                        Array.Copy(m_receiveBuffer, m_offsetRemaining, payload.Buffer, payload.Offset, m_sizeRemaining);
                        m_indexFilled = m_sizeRemaining;
                        Debug.Assert(m_indexFilled <= m_contentLength);
                        if (m_indexFilled == m_contentLength)
                        {
                            m_state = State.STATE_CONTENT_DONE;
                        }
                    }
                    else
                    {
                        // too much content
                        ok = false;
                    }
                }
            }
            else
            {
                Debug.Assert(m_message != null);
                BufferRef payload = m_message.GetReceivePayload();
                Debug.Assert(payload.Length == m_contentLength);
                int remainingContent = m_contentLength - m_indexFilled;
                if (count <= remainingContent)
                {
                    Array.Copy(buffer, 0, payload.Buffer, m_indexFilled, count);

                    m_indexFilled += count;
                    Debug.Assert(m_indexFilled <= m_contentLength);
                    if (m_indexFilled == m_contentLength)
                    {
                        m_state = State.STATE_CONTENT_DONE;
                    }
                }
                else
                {
                    // too much content
                    ok = false;
                }
            }

            if (ok)
            {
                if (m_state == State.STATE_CONTENT_DONE)
                {
                    Debug.Assert(m_message != null);
                    CheckSessionName();
                    var callback = m_callback;
                    if (callback != null)
                    {
                        bool handled = HandleInternalCommands(callback, ref ok);
                        if (!handled)
                        {
                            callback.Received(m_message, m_connectionId);
                        }
                    }
                    Reset();
                }
            }
            return ok;
        }

        // IProtocol
        public void SetCallback(IProtocolCallback callback)
        {
            m_callback = callback;
            // 5 minutes session timeout
            callback.ActivityTimeout = 5 * 60000;
        }
        public IStreamConnection? Connection 
        {
            get => m_connection;
            set => m_connection = value;
        }
        public void Disconnect()
        {
            IStreamConnection? connection = m_connection;
            if (connection != null)
            {
                connection.Disconnect();
            }
        }
        public uint ProtocolId 
        { 
            get
            {
                return PROTOCOL_ID;
            }
        }
        public bool AreMessagesResendable 
        {
            get
            {
                return false;
            }
        }
        public bool DoesSupportMetainfo
        {
            get
            {
                return true;
            }
        }
        public bool DoesSupportSession
        {
            get
            {
                return true;
            }
        }
        public bool NeedsReply
        {
            get
            {
                return true;
            }
        }
        public bool IsMultiConnectionSession
        {
            get
            {
                return true;
            }
        }
        public bool IsSendRequestByPoll
        {
            get
            {
                return true;
            }
        }
        public bool DoesSupportFileTransfer
        {
            get
            {
                return true;
            }
        }
        public bool IsSynchronousRequestReply
        {
            get
            {
                return false;
            }
        }
        public FuncCreateMessage MessageFactory
        {
            get
            {
                return () =>
                    {
                        return new ProtocolMessage(PROTOCOL_ID);
                    };
            }
        }


        public void SendMessage(IMessage message)
        {
            Debug.Assert(!message.WasSent);
            MemoryStream firstLine = new MemoryStream();
            Variant controlData = message.ControlData;
            bool pollStop = false;
            if (m_chunkedState != ChunkedState.STATE_STOP)
            {
                pollStop = controlData.GetData<bool>("fmq_poll_stop");
            }
            Variant? filename = controlData.GetVariant("filetransfer");
            int filesize = -1;
            if (filename != null)
            {
                FileInfo fileinfo = new FileInfo(filename);
                try
                {
                    filesize = (int)fileinfo.Length;
                }
                catch (Exception)
                {
                }
                message.DownsizeLastSendPayload(0);
            }
            Variant? http = controlData.GetVariant(FMQ_HTTP);
            if (m_chunkedState < ChunkedState.STATE_FIRST_CHUNK)
            {
                if (http != null && http == HTTP_REQUEST)
                {
                    if (filename != null && filesize == -1)
                    {
                        filesize = 0;
                    }
                    Variant? method = controlData.GetVariant(FMQ_METHOD);
                    Variant? path = controlData.GetVariant(FMQ_PATH);
                    if (method != null && path != null)
                    {
                        string pathEncode = Encode(path);
                        firstLine.Write(Encoding.ASCII.GetBytes(method));
                        firstLine.WriteByte((byte)' ');
                        firstLine.Write(Encoding.ASCII.GetBytes(pathEncode));

                        VariantStruct queries = controlData.GetData<VariantStruct>("queries");
                        foreach (var query in queries)
                        {
                            if (query == queries.First())
                            {
                                firstLine.WriteByte((byte)'?');
                            }
                            else
                            {
                                firstLine.WriteByte((byte)'&');
                            }
                            string key = Encode(query.Name); ;
                            string value = Encode(query.Value);
                            firstLine.Write(Encoding.ASCII.GetBytes(key));
                            firstLine.WriteByte((byte)'=');
                            firstLine.Write(Encoding.ASCII.GetBytes(value));
                        }

                        firstLine.Write(SPACE_HTTP_VERSION);
                    }
                }
                else
                {
                    string status = controlData.GetData<string>(FMQ_HTTP_STATUS);
                    string statustext = controlData.GetData<string>(FMQ_HTTP_STATUSTEXT);
                    if (filename != null && filesize == -1)
                    {
                        status = "404";
                        statustext = FILE_NOT_FOUND;
                    }
                    firstLine.Write(HTTP_VERSION_SPACE);
                    if (status.Length != 0)
                    {
                        firstLine.Write(Encoding.ASCII.GetBytes(status));
                        firstLine.WriteByte((byte)' ');
                    }
                    else
                    {
                        firstLine.Write(NUMBER_200_SPACE);
                    }
                    if (statustext != string.Empty)
                    {
                        firstLine.Write(Encoding.ASCII.GetBytes(statustext));
                    }
                    else
                    {
                        firstLine.Write(BYTES_OK);
                    }
                }
            }

            int sumHeaderSize = 0;
            if (m_chunkedState < ChunkedState.STATE_FIRST_CHUNK)
            {
                sumHeaderSize += (int)firstLine.Length + 2 + 2;   // 2 = '\r\n' and 2 = last empty line
                sumHeaderSize += HEADER_KEEP_ALIVE.Length;  // Connection: keep-alive\r\n
            }
            
            if (http != null && http == HTTP_REQUEST)
            {
                sumHeaderSize += m_headerHost.Length;   // Host: hostname\r\n
            }

            int sizeBody = message.GetTotalSendPayloadSize();
            if (filesize != -1)
            {
                sizeBody = filesize;
            }
            Metainfo metainfo = message.AllMetainfo;
            if (m_chunkedState == ChunkedState.STATE_STOP)
            {
                metainfo[CONTENT_LENGTH] = sizeBody.ToString();
            }
            if (m_headerSendNext.Count != 0)
            {
                foreach (var entry in m_headerSendNext)
                {
                    metainfo.Add(entry.Key, entry.Value);
                }
                m_headerSendNext.Clear();
            }
            foreach (var entry in metainfo)
            {
                string key = entry.Key;
                string value = entry.Value;
                if (key.Length != 0)
                {
                    sumHeaderSize += key.Length + value.Length + 4;    // 4 = ': ' and '\r\n'
                }
            }

            BufferRef? headerBuffer = null;

            int index = 0;
            if (!pollStop || m_multipart)
            {
                if (m_chunkedState >= ChunkedState.STATE_FIRST_CHUNK)
                {
                    Debug.Assert(sumHeaderSize == 0);
                    message.AddSendPayload(CR_LF);
                    int sizeChunkedData = sizeBody;
                    firstLine = new MemoryStream();     // clear
                    if (m_chunkedState == ChunkedState.STATE_FIRST_CHUNK || m_chunkedState == ChunkedState.STATE_BEGIN_MULTIPART)
                    {
                        m_chunkedState = ChunkedState.STATE_CONTINUE;
                    }
                    else
                    {
                        //                firstLine = "\r\n";
                    }
                    firstLine.Write(Encoding.ASCII.GetBytes(sizeChunkedData.ToString(FORMAT_X)));
                    sumHeaderSize += (int)firstLine.Length + 2;  // "\r\n"
                }

                byte[] bytesFirstLine = firstLine.ToArray();
                headerBuffer = message.AddSendHeader(sumHeaderSize);
                index = 0;
                Debug.Assert(index + bytesFirstLine.Length + 2 <= sumHeaderSize);
                Array.Copy(bytesFirstLine, 0, headerBuffer.Buffer, index, bytesFirstLine.Length);
                index += bytesFirstLine.Length;
                Array.Copy(CR_LF, 0, headerBuffer.Buffer, index, 2);
                index += 2;
            }
            else
            {
                headerBuffer = message.AddSendHeader(sumHeaderSize);
                index = 0;
            }

            if (http != null && http == HTTP_REQUEST)
            {
                // Host: hostname\r\n
                Debug.Assert(index + m_headerHost.Length <= sumHeaderSize);
                Array.Copy(m_headerHost, 0, headerBuffer.Buffer, index, m_headerHost.Length);
                index += m_headerHost.Length;
            }

            // Connection: keep-alive\r\n
            if (m_chunkedState < ChunkedState.STATE_FIRST_CHUNK)
            {
                Debug.Assert(index + HEADER_KEEP_ALIVE.Length <= sumHeaderSize);
                Array.Copy(HEADER_KEEP_ALIVE, 0, headerBuffer.Buffer, index, HEADER_KEEP_ALIVE.Length);
                index += HEADER_KEEP_ALIVE.Length;
            }

            foreach (var info in metainfo)
            {
//                byte[] bytesKey = Encoding.ASCII.GetBytes(info.Key);
//                byte[] bytesValue = Encoding.ASCII.GetBytes(info.Value);
                if (info.Key.Length != 0)
                {
                    Debug.Assert(index + info.Key.Length + info.Value.Length + 4 <= sumHeaderSize);
//                    Array.Copy(bytesKey, 0, headerBuffer.Buffer, index, bytesKey.Length);
                    Encoding.ASCII.GetBytes(info.Key, 0, info.Key.Length, headerBuffer.Buffer, index);
                    index += info.Key.Length;
                    Array.Copy(COLON, 0, headerBuffer.Buffer, index, 2);
                    index += 2;
//                    Array.Copy(bytesValue, 0, headerBuffer.Buffer, index, bytesValue.Length);
                    Encoding.ASCII.GetBytes(info.Value, 0, info.Value.Length, headerBuffer.Buffer, index);
                    index += info.Value.Length;
                    Array.Copy(CR_LF, 0, headerBuffer.Buffer, index, 2);
                    index += 2;
                }
            }
            if (m_chunkedState < ChunkedState.STATE_FIRST_CHUNK)
            {
                Debug.Assert(index + 2 == sumHeaderSize);
                Array.Copy(CR_LF, 0, headerBuffer.Buffer, index, 2);
            }

            if (pollStop)
            {
                message.AddSendPayload(POLL_STOP);
                m_chunkedState = ChunkedState.STATE_STOP;
                m_multipart = false;
            }

            message.PrepareMessageToSend();

            Debug.Assert(m_connection != null);
            m_connection.SendMessage(message);


            if (filename != null && filesize > 0)
            {
                string file = filename;
                GlobalExecutorWorker.Instance.AddAction(() => {

                    try
                    {
                        using (var fs = new FileStream(file, FileMode.Open))
                        {
                            if (fs != null)
                            {
                                long len = fs.Length;
                                int err = 0;
                                int lenReceived = 0;
                                bool ex = false;
                                while (!ex)
                                {
                                    int size = (int)Math.Min(1024, len);
                                    IMessage messageData = new ProtocolMessage(0);
                                    BufferRef buf = messageData.AddSendPayload(size);

                                    try
                                    {
                                        err = fs.Read(buf.Buffer, 0, size);
                                    }
                                    catch (IOException)
                                    {
                                        err = -1;
                                    }

                                    if (err > 0)
                                    {
                                        Debug.Assert(err <= size);
                                        if (err < size)
                                        {
                                            messageData.DownsizeLastSendPayload(err);
                                        }
                                        m_connection.SendMessage(messageData);
                                        buf.AddOffset(err);
                                        len -= err;
                                        lenReceived += err;
                                        err = 0;
                                        Debug.Assert(len >= 0);
                                        if (len == 0)
                                        {
                                            ex = true;
                                        }
                                    }
                                    else
                                    {
                                        ex = true;
                                    }
                                }
                                if (lenReceived < filesize)
                                {
                                    m_connection.Disconnect();
                                }
                            }
                            else
                            {
                                m_connection.Disconnect();
                            }
                        }
                    }
                    catch (Exception)
                    {
                        m_connection.Disconnect();
                    }
                });
            }
            
        }
        public void MoveOldProtocolState(IProtocol protocolOld)
        {

        }


        public IMessage? PollReply(IList<IMessage> messages)
        {
            IMessage message = MessageFactory();
            if (m_multipart)
            {
                if (messages.Count != 0)
                {
                    foreach (var msg in messages)
                    {
                        MemoryStream payload = new MemoryStream();
                        if (m_chunkedState == ChunkedState.STATE_FIRST_CHUNK)
                        {
                            m_chunkedState = ChunkedState.STATE_BEGIN_MULTIPART;
                            payload.Write(MINUS_MINUS);
                            payload.Write(FMQ_MULTIPART_BOUNDARY);
                        }
                        payload.Write(CR_LF_CR_LF);
                        message.AddSendPayload(payload.ToArray());
                        IList<BufferRef> payloads = msg.GetAllSendPayloads();
                        message.MoveSendBuffers(payloads);
                        message.AddSendPayload(CR_LF_MINUS_MINUS_MULTIPART_BOUNDARY);
                    }
                }
                else
                {
                    if (m_chunkedState == ChunkedState.STATE_FIRST_CHUNK)
                    {
                        m_chunkedState = ChunkedState.STATE_BEGIN_MULTIPART;
                        message.AddSendPayload(MINUS_MINUS_MULTIPART_BOUNDARY_MINUS_MINUS_CR_LF);
                    }
                    else
                    {
                        message.AddSendPayload(MINUS_MINUS_CR_LF);
                    }
                }
            }
            else
            {
                foreach (var msg in messages)
                {
                    IList<BufferRef> payloads = msg.GetAllSendPayloads();
                    message.MoveSendBuffers(payloads);
                }
            }
            return message;
        }
        public void Subscribe(IList<string> subscribtions)
        {

        }

        static char[] tabDecToHex = {
            '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
        };

        static string Encode(string src)
        {
            StringBuilder dest = new StringBuilder();
            byte uc;

            foreach (char c in src)
            {
                if (Char.IsLetter(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
                {
                    dest.Append(c);
                }
                else 
                {
                    uc = (byte)c;
                    int first = (uc >> 4) & 0x0f;
                    int second = uc & 0x0f;
                    Debug.Assert(0 <= first && first< 16);
                    Debug.Assert(0 <= second && second< 16);
                    dest.Append('%');
                    dest.Append(tabDecToHex[first]);
                    dest.Append(tabDecToHex[second]);
                }
            }
            return dest.ToString();
        }

        static uint HexToUInt(char hexChar)
        {
            hexChar = char.ToUpper(hexChar);
            return (uint)(hexChar < 'A' ? (hexChar - '0') : 10 + (hexChar - 'A'));
        }

        static string Decode(string src)
        {
            StringBuilder dest = new StringBuilder();

            int len = src.Length;
            for (int i = 0; i < len; ++i)
            {
                if (src[i] == '%')
                {
                    ++i;
                    uint code = HexToUInt(src[i]) << 4;
                    ++i;
                    code |= HexToUInt(src[i]);
                    char ch = (char)code;
                    dest.Append(ch);
                }
                else
                {
                    dest.Append(src[i]);
                }
            }
            return dest.ToString();
        }

        static void SplitOnce(string src, int indexBegin, int indexEnd, char delimiter, IList<string> dest)
        {
            int count = indexEnd - indexBegin;
            int pos = src.IndexOf(delimiter, indexBegin, count);
            if (pos == -1 || pos > indexEnd)
            {
                pos = indexEnd;
            }
            int len = pos - indexBegin;
            Debug.Assert(len >= 0);
            dest.Add(src.Substring(indexBegin, len));
            ++pos;

            if (pos < indexEnd)
            {
                len = indexEnd - pos;
                Debug.Assert(len >= 0);
                dest.Add(src.Substring(pos, len));
            }
        }


        string CreateSessionName()
        {
            long sessionCounter = Interlocked.Increment(ref m_nextSessionNameCounter);
            long v1 = m_randomGenerator1.NextInt64();
            long v2 = m_randomGenerator2.NextInt64();

            string sessionName = v1.ToString(FORMAT_X8) + v2.ToString(FORMAT_X8) + "." + sessionCounter.ToString();
            return sessionName;
        }


        
        void CheckSessionName()
        {
            if (m_path != null && m_path == FMQ_PATH_CREATESESSION)
            {
                m_createSession = true;
            }
            
            var callback = m_callback;
            if (callback != null)
            {
                bool found = false;
                if (!m_createSession)
                {
                    Debug.Assert(m_connection != null);
                    IList<string> sessionMatched = new List<string>();
                    foreach (string sessionName in m_sessionNames)
                    {
                        bool foundInNames = callback.FindSessionByName(sessionName, this);
                        if (foundInNames)
                        {
                            sessionMatched.Add(sessionName);
                        }
                    }
                    string? sessionFound = null;
                    if (sessionMatched.Count == 1)
                    {
                        sessionFound = sessionMatched[0];
                    }
                    else
                    {
                        long counterMax = 0;
                        foreach (string session in sessionMatched)
                        {
                            int pos = session.IndexOf('.');
                            if (pos != -1)
                            {
                                try
                                {
                                    long counter = Int64.Parse(session.Substring(pos + 1));
                                    if (counter > counterMax)
                                    {
                                        counterMax = counter;
                                        sessionFound = session;
                                    }
                                }
                                catch (Exception)
                                {
                                }
                            }
                        }
                    }
                    if (sessionFound != null && sessionFound.Length != 0)
                    {
                        found = true;
                        if (m_sessionName != sessionFound)
                        {
                            m_sessionName = sessionFound;
                        }
                    }
                }
                if (m_createSession || !found)
                {
                    m_sessionName = CreateSessionName();
                    Debug.Assert(m_connection != null);
                    callback.SetSessionName(m_sessionName, this, m_connection);
                    m_headerSendNext[FMQ_SET_SESSION] = m_sessionName;
                    m_headerSendNext[HTTP_SET_COOKIE] = COOKIE_PREFIX + m_sessionName + "; path=/"; // ; Partitioned";
                }
            }
            m_sessionNames.Clear();
        }


        void CookiesToSessionIds(string cookies)
        {
            m_sessionNames.Clear();
            int posStart = 0;
            while (posStart != -1)
            {
                int posEnd = cookies.IndexOf(';', posStart);
                string cookie;
                if (posEnd != -1)
                {
                    cookie = cookies.Substring(posStart, posEnd - posStart);
                    posStart = posEnd + 1;
                }
                else
                {
                    cookie = cookies.Substring(posStart);
                    posStart = posEnd;
                }

                int pos = cookie.IndexOf("fmq=");
                if (pos != -1)
                {
                    pos += 4;
                    string sessionId = cookie.Substring(pos);
                    if (sessionId.Length != 0)
                    {
                        m_sessionNames.Add(sessionId);
                    }
                }
            }
        }



        bool ReceiveHeaders(int bytesReceived)
        {
            bool ok = true;
            bytesReceived += m_sizeRemaining;
            Debug.Assert(bytesReceived <= m_receiveBuffer.Length);
            while (m_offsetRemaining < bytesReceived && ok)
            {
                int index = Array.IndexOf<byte>(m_receiveBuffer, (byte)'\n', m_offsetRemaining);
                if (index != -1)
                {
                    int indexEndLine = index;
                    --indexEndLine; // goto '\r'
                    int len = indexEndLine - m_offsetRemaining;
                    if (len < 0 || m_receiveBuffer[indexEndLine] != '\r')
                    {
                        ok = false;
                    }
                    if (ok)
                    {
                        if (m_state == State.STATE_FIND_FIRST_LINE)
                        {
                            m_contentLength = 0;
                            if (len < 4)
                            {
                                ok = false;
                            }
                            else
                            {
                                // is response
                                if (m_receiveBuffer[m_offsetRemaining] == 'H' && m_receiveBuffer[m_offsetRemaining + 1] == 'T')
                                {
                                    string line = Encoding.ASCII.GetString(m_receiveBuffer, m_offsetRemaining, indexEndLine - m_offsetRemaining);
                                    string[] lineSplit = line.Split(' ');
                                    if (lineSplit.Length == 3)
                                    {
                                        m_message = new ProtocolMessage(0);
                                        Variant controlData = m_message.ControlData;
                                        controlData.Add(FMQ_HTTP, HTTP_RESPONSE);
                                        controlData.Add(FMQ_PROTOCOL, lineSplit[0]);
                                        controlData.Add(FMQ_HTTP_STATUS, lineSplit[1]);
                                        controlData.Add(FMQ_HTTP_STATUSTEXT, lineSplit[2]);
                                        m_state = State.STATE_FIND_HEADERS;
                                    }
                                    else
                                    {
                                        ok = false;
                                    }
                                }
                                else
                                {
                                    string line = Encoding.ASCII.GetString(m_receiveBuffer, m_offsetRemaining, indexEndLine - m_offsetRemaining);
                                    string[] lineSplit = line.Split(' ');
                                    if (lineSplit.Length == 3)
                                    {
                                        string[] pathquerySplit = lineSplit[1].Split('?');
                                        m_message = new ProtocolMessage(0);
                                        Metainfo metainfo = m_message.AllMetainfo;
                                        metainfo[FMQ_HTTP] = HTTP_REQUEST;
                                        metainfo[FMQ_METHOD] = lineSplit[0];
                                        metainfo[FMQ_PROTOCOL] = lineSplit[2];
                                        if (pathquerySplit.Length >= 1)
                                        {
                                            string path = Decode(pathquerySplit[0]);
                                            metainfo[FMQ_PATH] = path;
                                            m_path = path;
                                        }
                                        if (pathquerySplit.Length >= 2)
                                        {
                                            string[] querySplit = pathquerySplit[1].Split('&');
                                            foreach (string query in querySplit)
                                            {
                                                string queryTotal = Decode(query);
                                                string[] queryNameValue = queryTotal.Split('=');
                                                if (queryNameValue.Length == 1)
                                                {
                                                    metainfo[FMQ_QUERY_PREFIX + queryNameValue[0]] = string.Empty;
                                                }
                                                else if (queryNameValue.Length >= 2)
                                                {
                                                    metainfo[FMQ_QUERY_PREFIX + queryNameValue[0]] = queryNameValue[1];
                                                }
                                            }
                                        }
                                        m_state = State.STATE_FIND_HEADERS;
                                    }
                                    else
                                    {
                                        ok = false;
                                    }
                                }
                            }
                        }
                        else if (m_state == State.STATE_FIND_HEADERS)
                        {
                            Debug.Assert(m_message != null);
                            if (len == 0)
                            {
                                if (m_contentLength == 0)
                                {
                                    m_state = State.STATE_CONTENT_DONE;
                                }
                                else
                                {
                                    m_state = State.STATE_CONTENT;
                                    m_message.ResizeReceiveBuffer(m_contentLength);
                                }
                                m_indexFilled = 0;
                                m_offsetRemaining += 2;
                                break;
                            }
                            else
                            {
                                IList<string> lineSplit = new List<string>();
                                string line = Encoding.ASCII.GetString(m_receiveBuffer, m_offsetRemaining, indexEndLine - m_offsetRemaining);
                                SplitOnce(line, 0, line.Length, ':', lineSplit);
                                if (lineSplit.Count == 2)
                                {
                                    string value = lineSplit[1];
                                    if (value.Length != 0 && value[0] == ' ')
                                    {
                                        value = value.Substring(1);
                                    }
                                    if (lineSplit[0] == CONTENT_LENGTH)
                                    {
                                        try
                                        {
                                            m_contentLength = Int32.Parse(value);
                                        }
                                        catch (Exception)
                                        {
                                            m_contentLength = 0;
                                        }
                                    }
                                    else if (lineSplit[0] == FMQ_CREATESESSION)
                                    {
                                        m_createSession = true;
                                    }
                                    else if (lineSplit[0] == FMQ_SESSIONID)
                                    {
                                        m_sessionNames.Clear();
                                        if (value.Length != 0)
                                        {
                                            m_sessionNames.Add(value);
                                        }
                                        m_stateSessionId = StateSessionId.SESSIONID_FMQ;
                                    }
                                    else if (lineSplit[0] == HTTP_COOKIE)
                                    {
                                        if (m_stateSessionId == StateSessionId.SESSIONID_NONE)
                                        {
                                            CookiesToSessionIds(value);
                                            m_stateSessionId = StateSessionId.SESSIONID_COOKIE;
                                        }
                                    }
                                    m_message.AddMetainfo(lineSplit[0], value);
                                }
                                else if (lineSplit.Count == 1)
                                {
                                    m_message.AddMetainfo(lineSplit[0], "");
                                }
                                else
                                {
                                    Debug.Assert(false);
                                    ok = false;
                                }
                            }
                        }
                        m_offsetRemaining += len + 2;
                    }
                }
                else
                {
                    break;
                }
            }
            m_sizeRemaining = bytesReceived - m_offsetRemaining;
            Debug.Assert(m_sizeRemaining >= 0);
            return ok;
        }

        void Reset()
        {
            m_offsetRemaining = 0;
            m_sizeRemaining = 0;
            m_contentLength = 0;
            m_indexFilled = 0;
            m_message = null;
            m_state = State.STATE_FIND_FIRST_LINE;
            m_stateSessionId = StateSessionId.SESSIONID_NONE;
            m_createSession = false;
            m_sessionNames.Clear();
            m_path = null;
        }



        enum State
        {
            STATE_FIND_FIRST_LINE,
            STATE_FIND_HEADERS,
            STATE_CONTENT,
            STATE_CONTENT_DONE
        }

        enum StateSessionId
        {
            SESSIONID_NONE = 0,
            SESSIONID_COOKIE = 1,
            SESSIONID_FMQ = 2
        }

//        random_device m_randomDevice;
//        mt19937 m_randomGenerator;
//        uniform_int_distribution<ulong> m_randomVariable;
        readonly Metainfo m_headerSendNext = new Metainfo();
        StateSessionId m_stateSessionId = StateSessionId.SESSIONID_NONE;
        IList<string> m_sessionNames = new List<string>();

        State m_state = State.STATE_FIND_FIRST_LINE;
        byte[] m_receiveBuffer = Array.Empty<byte>();
        int m_offsetRemaining = 0;
        int m_sizeRemaining = 0;
        IMessage? m_message = null;
        int m_contentLength = 0;
        int m_indexFilled = 0;
        byte[] m_headerHost = Array.Empty<byte>();
        long m_connectionId = 0;
        bool m_createSession = false;
        string m_sessionName = "";
        IProtocolCallback? m_callback = null;
        IStreamConnection? m_connection = null;
        ChunkedState m_chunkedState = ChunkedState.STATE_STOP;
        bool m_multipart = false;
        Random m_randomGenerator1 = new Random(Guid.NewGuid().GetHashCode());
        Random m_randomGenerator2 = new Random(Guid.NewGuid().GetHashCode());

        // path
        string? m_path = null;

        static long m_nextSessionNameCounter = 1;   // atomic
    };


    //---------------------------------------
    // register factory
    //---------------------------------------
    class RegisterProtocolHttpServerFactory
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register()
        {
            ProtocolRegistry.Instance.RegisterProtocolFactory(ProtocolHttpServer.PROTOCOL_NAME, ProtocolHttpServer.PROTOCOL_ID, (Variant? data) => { return new ProtocolHttpServer(); } );
        }
    }

}
