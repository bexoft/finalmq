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

    class ProtocolHttpServer : IProtocol
    {
        public static readonly uint PROTOCOL_ID = 4;
        public static readonly string PROTOCOL_NAME = "httpserver";

        static readonly string FMQ_HTTP = "fmq_http";
        static readonly string FMQ_METHOD = "fmq_method";
        static readonly string FMQ_PROTOCOL = "fmq_protocol";
        static readonly string FMQ_PATH = "fmq_path";
        static readonly string FMQ_QUERY_PREFIX = "QUERY_";
        static readonly string FMQ_HTTP_STATUS = "fmq_http_status";
        static readonly string FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
        static readonly string HTTP_REQUEST = "request";
        static readonly string HTTP_RESPONSE = "response";

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
        static readonly string FMQ_MULTIPART_BOUNDARY = "B9BMAhxAhY.mQw1IDRBA";

        static readonly string FILE_NOT_FOUND = "file not found";
        static readonly string HEADER_KEEP_ALIVE = "Connection: keep-alive\r\n";

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
            var callback = m_callback;
            if (callback != null)
            {
                callback.Connected();
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
        public void Received(IStreamConnection connection, byte[] buffer, int count)
        {
            IMessage message = new ProtocolMessage(0);
            message.SetReceiveBuffer(buffer, 0, count);
            var callback = m_callback;
            if (callback != null)
            {
                callback.Received(message);
            }
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
            StringBuilder firstLine = new StringBuilder();
            Variant controlData = message.ControlData;
            bool pollStop = false;
            if (m_chunkedState != (int)ChunkedState.STATE_STOP)
            {
                pollStop = controlData.GetData<bool>("fmq_poll_stop");
            }
            string? filename = controlData.GetData<string>("filetransfer");
            int filesize = -1;
            if (filename != null)
            {
                FileInfo fileinfo = new FileInfo(filename);
                filesize = (int)fileinfo.Length;
                message.DownsizeLastSendPayload(0);
            }
            string? http = controlData.GetData<string>(FMQ_HTTP);
            if (m_chunkedState < (int)ChunkedState.STATE_FIRST_CHUNK)
            {
                if (http != null && http == HTTP_REQUEST)
                {
                    if (filename != null && filesize == -1)
                    {
                        filesize = 0;
                    }
                    string? method = controlData.GetData<string> (FMQ_METHOD);
                    string? path = controlData.GetData<string> (FMQ_PATH);
                    if (method != null && path != null)
                    {
                        string pathEncode = Encode(path);
                        firstLine.Append(method);
                        firstLine.Append(' ');
                        firstLine.Append(pathEncode);

                        VariantStruct? queries = controlData.GetData<VariantStruct>("queries");
                        if (queries != null)
                        {
                            foreach (var query in queries)
                            {
                                if (query == queries.First())
                                {
                                    firstLine.Append('?');
                                }
                                else
                                {
                                    firstLine.Append('&');
                                }
                                string key = Encode(query.Name); ;
                                string value = Encode(query.Value);
                                firstLine.Append(key);
                                firstLine.Append('=');
                                firstLine.Append(value);
                            }
                        }

                        firstLine.Append(" HTTP/1.1");
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
                    firstLine.Append("HTTP/1.1 ");
                    if (status.Length != 0)
                    {
                        firstLine.Append(status);
                        firstLine.Append(' ');
                    }
                    else
                    {
                        firstLine.Append("200 ");
                    }
                    if (statustext != string.Empty)
                    {
                        firstLine.Append(statustext);
                    }
                    else
                    {
                        firstLine.Append("OK");
                    }
                }
            }

            int sumHeaderSize = 0;
            if (m_chunkedState < (int)ChunkedState.STATE_FIRST_CHUNK)
            {
                sumHeaderSize += firstLine.Length + 2 + 2;   // 2 = '\r\n' and 2 = last empty line
                sumHeaderSize += HEADER_KEEP_ALIVE.Length;  // Connection: keep-alive\r\n
            }
            
            if (http == HTTP_REQUEST)
            {
                sumHeaderSize += m_headerHost.Length;   // Host: hostname\r\n
            }

            int sizeBody = message.GetTotalSendPayloadSize();
            if (filesize != -1)
            {
                sizeBody = filesize;
            }
            Metainfo metainfo = message.AllMetainfo;
            if (m_chunkedState == (int)ChunkedState.STATE_STOP)
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
/*
            size_t index = 0;
            if (!pollStop || m_multipart)
            {
                if (m_chunkedState >= STATE_FIRST_CHUNK)
                {
                    assert(sumHeaderSize == 0);
                    message->addSendPayload("\r\n");
                    int sizeChunkedData = static_cast<int>(sizeBody);
                    firstLine.clear();
                    if (m_chunkedState == STATE_FIRST_CHUNK || m_chunkedState == STATE_BEGIN_MULTIPART)
                    {
                        m_chunkedState = STATE_CONTINUE;
                    }
                    else
                    {
                        //                firstLine = "\r\n";
                    }
                    char buffer[50];
            # ifdef WIN32
                    _itoa_s(sizeChunkedData, buffer, sizeof(buffer), 16);
            #else
                    snprintf(buffer, sizeof(buffer), "%X", sizeChunkedData);
            #endif
                    firstLine += buffer;
                    for (auto & c : firstLine)
                    {
                        c = toupper(c);
                    }
                    sumHeaderSize += firstLine.size() + 2;  // "\r\n"
                }

                headerBuffer = message.AddSendHeader(sumHeaderSize);
                index = 0;
                assert(index + firstLine.size() + 2 <= sumHeaderSize);
                memcpy(headerBuffer + index, firstLine.data(), firstLine.size());
                index += firstLine.size();
                memcpy(headerBuffer + index, "\r\n", 2);
                index += 2;
            }
            else
            {
                headerBuffer = message->addSendHeader(sumHeaderSize);
                index = 0;
            }

            if (http && *http == HTTP_REQUEST)
            {
                // Host: hostname\r\n
                assert(index + m_headerHost.size() <= sumHeaderSize);
                memcpy(headerBuffer + index, m_headerHost.data(), m_headerHost.size());
                index += m_headerHost.size();
            }

            // Connection: keep-alive\r\n
            if (m_chunkedState < STATE_FIRST_CHUNK)
            {
                assert(index + HEADER_KEEP_ALIVE.size() <= sumHeaderSize);
                memcpy(headerBuffer + index, HEADER_KEEP_ALIVE.data(), HEADER_KEEP_ALIVE.size());
                index += HEADER_KEEP_ALIVE.size();
            }

            for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
            {
                const std::string&key = it->first;
                const std::string&value = it->second;
                if (!key.empty())
                {
                    assert(index + key.size() + value.size() + 4 <= sumHeaderSize);
                    memcpy(headerBuffer + index, key.data(), key.size());
                    index += key.size();
                    memcpy(headerBuffer + index, ": ", 2);
                    index += 2;
                    memcpy(headerBuffer + index, value.data(), value.size());
                    index += value.size();
                    memcpy(headerBuffer + index, "\r\n", 2);
                    index += 2;
                }
            }
            if (m_chunkedState < STATE_FIRST_CHUNK)
            {
                assert(index + 2 == sumHeaderSize);
                memcpy(headerBuffer + index, "\r\n", 2);
            }

            if (pollStop)
            {
                message->addSendPayload("0\r\n\r\n");
                m_chunkedState = STATE_STOP;
                m_multipart = false;
            }

            message->prepareMessageToSend();

            assert(m_connection);
            m_connection->sendMessage(message);


            if (filename)
            {
                std::string file = *filename;
                std::weak_ptr<ProtocolHttpServer> pThisWeak = shared_from_this();
                GlobalExecutorWorker::instance().addAction([pThisWeak, file, filesize]() {
                    std::shared_ptr<ProtocolHttpServer> pThis = pThisWeak.lock () ;
                    if (!pThis)
                    {
                        return;
                    }
                    int flags = O_RDONLY;
            # ifdef WIN32
                    flags |= O_BINARY;
            #endif
                    int fd = OperatingSystem::instance().open(file.c_str(), flags);
                    if (fd != -1)
                    {
                        int len = static_cast<int>(filesize);
                        int err = 0;
                        int lenReceived = 0;
                        bool ex = false;
                        while (!ex)
                        {
                            int size = std::min(1024, len);
                            IMessagePtr messageData = std::make_shared<ProtocolMessage>(0);
                            char* buf = messageData->addSendPayload(size);
                            do
                            {
                                err = OperatingSystem::instance().read(fd, buf, size);
                            } while (err == -1 && OperatingSystem::instance().getLastError() == SOCKETERROR(EINTR));

                            if (err > 0)
                            {
                                assert(err <= size);
                                if (err < size)
                                {
                                    messageData->downsizeLastSendPayload(err);
                                }
                                pThis->m_connection->sendMessage(messageData);
                                buf += err;
                                len -= err;
                                lenReceived += err;
                                err = 0;
                                assert(len >= 0);
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
                            pThis->m_connection->disconnect();
                        }
                    }
                    else
                    {
                        pThis->m_connection->disconnect();
                    }
                });
            }
            */
        }
        public void MoveOldProtocolState(IProtocol protocolOld)
        {

        }
        public IMessage? PollReply(IList<IMessage>? messages = null)
        {
            return null;
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
            ulong c = 0;

            int len = src.Length;
            for (int i = 0; i < len; ++i)
            {
                if (src[i] == '%')
                {
                    ++i;
                    uint code = HexToUInt(src[i]) << 8;
                    ++i;
                    code |= HexToUInt(src[i]);
                    dest.Append(code);
                }
                else
                {
                    dest.Append(src[i]);
                }
            }
            return dest.ToString();
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
        string m_headerHost = "";
        long m_connectionId = 0;
        bool m_createSession = false;
        string m_sessionName = "";
        IProtocolCallback? m_callback = null;
        IStreamConnection? m_connection = null;
        int m_chunkedState = (int)ChunkedState.STATE_STOP;
        bool m_multipart = false;

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
