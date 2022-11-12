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

using System.Threading;
using System.Diagnostics;

namespace finalmq 
{


    internal interface IProtocolSessionPrivate : IProtocolSession
    {
        void Connect();
        void SetConnection(IStreamConnection? connection, bool verified);
        void SetProtocol(IProtocol protocol);
        void SetSessionNameInternal(string sessionName);
    }


        
    internal class ProtocolSession : IProtocolSessionPrivate
                                   , IProtocolCallback
    {
        public ProtocolSession(IProtocolSessionCallback callback, IExecutor? executor, IProtocol protocol, IProtocolSessionList protocolSessionList, BindProperties? bindProperties, int contentType)
        {
            m_callback = callback;
            m_executor = executor;
            m_protocol = protocol;
            m_protocolSessionList = protocolSessionList;
            m_sessionId = protocolSessionList.GetNextSessionId();
            m_instanceId = m_sessionId | INSTANCEID_PREFIX;
            m_contentType = contentType;
            m_bindProperties = bindProperties;
        }
        public ProtocolSession(IProtocolSessionCallback callback, IExecutor? executor, IProtocol protocol, IProtocolSessionList protocolSessionList, IStreamConnectionContainer streamConnectionContainer, string endpointStreamConnection, ConnectProperties? connectProperties, int contentType)
        {
            m_callback = callback;
            m_executor = executor;
            m_protocol = protocol;
            m_protocolSessionList = protocolSessionList;
            m_sessionId = protocolSessionList.GetNextSessionId();
            m_instanceId = m_sessionId | INSTANCEID_PREFIX;
            m_contentType = contentType;
            m_streamConnectionContainer = streamConnectionContainer;
            m_endpointStreamConnection = endpointStreamConnection;
            m_connectionProperties = connectProperties;
        }
        public ProtocolSession(IProtocolSessionCallback callback, IExecutor? executor, IProtocolSessionList protocolSessionList, IStreamConnectionContainer streamConnectionContainer)
        {
            m_callback = callback;
            m_executor = executor;
            m_protocolSessionList = protocolSessionList;
            m_sessionId = protocolSessionList.GetNextSessionId();
            m_instanceId = m_sessionId | INSTANCEID_PREFIX;
            m_streamConnectionContainer = streamConnectionContainer;
        }

        // IProtocolSession
        public IMessage CreateMessage()
        {
            if (m_protocolSet && m_messageFactory != null)
            {
                return m_messageFactory();
            }
            return new ProtocolMessage(0);
        }
        public void SendMessage(IMessage msg, bool isReply = false)
        {
            bool doDisconnect = false;
            // lock whole function, because the protocol can be changed by setProtocol (lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.)
            lock (m_mutex)
            {
                if (!isReply && m_protocolFlagIsSendRequestByPoll)
                {
                    if (m_pollProtocol != null)
                    {
                        ++m_pollCounter;
                        IList<IMessage> messages = new List<IMessage>();
                        messages.Add(msg);
                        IMessage? pollReply = m_pollProtocol.PollReply(messages);
                        Debug.Assert(pollReply != null);    // wrong implementation of the protocol
                        SendMessage(pollReply, m_pollProtocol);
                        if (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax)
                        {
                            PollRelease();
                        }
                    }
                    else
                    {
                        m_pollMessages.Add(msg);
                    }
                    if (m_pollMaxRequests != -1 && m_pollMessages.Count >= m_pollMaxRequests)
                    {
                        doDisconnect = true;
                    }
                }
                else
                {
                    if (!m_protocolSet)
                    {
                        m_messagesBuffered.Add(msg);
                        return;
                    }

                    IProtocol? protocol = m_protocol;
                    if (m_protocolFlagIsMultiConnectionSession)
                    {
                        //todo                        Variant & echoData = msg->getEchoData();
                        long connectionId = 0;  //todo echoData.getDataValue<std::int64_t>(FMQ_CONNECTION_ID);
                        GetProtocolFromConnectionId(ref protocol, connectionId);
                    }

                    SendMessage(msg, protocol);
                }
            }
            if (doDisconnect)
            {
                Disconnect();
            }
        }
        public long SessionId
        {
            get
            {
                return m_sessionId;
            }
        }
        public ConnectionData ConnectionData
        {
            get
            {
                IStreamConnection? connection = null;
                lock (m_mutex)
                {
                    if (m_protocol != null)
                    {
                        connection = m_protocol.Connection;
                    }
                    if (m_protocolFlagIsMultiConnectionSession &&
                        ((connection == null) || connection.ConnectionData.ConnectionState != ConnectionState.CONNECTIONSTATE_CONNECTED))
                    {
                        foreach (var entry in m_multiProtocols)
                        {
                            IStreamConnection? connIter = entry.Value.Connection;
                            if (connection == null)
                            {
                                connection = connIter;
                            }
                            if (connIter != null && connIter.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED)
                            {
                                connection = connIter;
                                break;
                            }
                        }
                    }
                }

                if (connection != null)
                {
                    return connection.ConnectionData;
                }
                ConnectionData defaultConnectionData = new ConnectionData();
                defaultConnectionData.ConnectionState = ConnectionState.CONNECTIONSTATE_DISCONNECTED;
                return defaultConnectionData;
            }
        }
        public int ContentType 
        {
            get
            {
                return m_contentType;
            }
        }
        public bool DoesSupportMetainfo()
        {
            return m_protocolFlagSupportMetainfo;
        }
        public bool NeedsReply()
        {
            return m_protocolFlagNeedsReply;
        }
        public bool IsMultiConnectionSession()
        {
            return m_protocolFlagIsMultiConnectionSession;
        }
        public bool IsSendRequestByPoll()
        {
            return m_protocolFlagIsSendRequestByPoll;
        }
        public bool DoesSupportFileTransfer()
        {
            return m_protocolFlagSupportFileTransfer;
        }
        public void Disconnect()
        {
            IList<IProtocol>? protocols = null;
            lock (m_mutex)
            {
                protocols = new List<IProtocol>();
                if (m_protocol != null)
                {
                    protocols.Add(m_protocol);
                }
                foreach (var entry in m_multiProtocols)
                {
                    if (entry.Value != null)
                    {
                        protocols.Add(entry.Value);
                    }
                }
            }

            foreach (var protocol in protocols)
            {
                protocol.Disconnect();
            }

            m_protocolSessionList.RemoveProtocolSession(m_sessionId);

            Disconnected();
        }
        public void Connect(string endpoint, ConnectProperties? connectionProperties = null, int contentType = 0)
        {
            int ixEndpoint = endpoint.LastIndexOf(':');
            if (ixEndpoint == -1)
            {
                throw new System.ArgumentException("Wrong endpoint format: " + endpoint);
            }
            string protocolName = endpoint.Substring(ixEndpoint + 1, endpoint.Length - (ixEndpoint + 1));
            FuncCreateProtocol protocolFactory = ProtocolRegistry.Instance.GetProtocolFactory(protocolName);

            IProtocol? protocol = protocolFactory(/* todo connectionProperties.ProtocolData */);
            Debug.Assert(protocol != null);


            Debug.Assert(m_protocol == null);
            Debug.Assert(m_streamConnectionContainer != null);

            IStreamConnection connection = m_streamConnectionContainer.CreateConnection(protocol);

            lock (m_mutex)
            {
                m_endpointStreamConnection = endpoint.Substring(0, ixEndpoint);
                m_connectionProperties = connectionProperties;
                m_contentType = contentType;
                m_protocol = protocol;
                m_connectionId = connection.ConnectionId;
                InitProtocolValues();
                m_protocol.SetCallback(this);
                m_protocol.SetConnection(connection);
                SendBufferedMessages();
            }

            m_streamConnectionContainer.Connect(m_endpointStreamConnection, connection, m_connectionProperties);
        }
        public IExecutor? Executor
        {
            get
            {
                return m_executor;
            }
        }
        public void Subscribe(IList<string> subscribtions)
        {
            IProtocol? protocol = null;
            lock (m_mutex)
            {
                if (m_protocol != null)
                {
                    protocol = m_protocol;
                }
            }
            if (protocol != null)
            {
                protocol.Subscribe(subscribtions);
            }
        }

        // IProtocolSessionPrivate
        public void Connect()
        {
            Debug.Assert(m_streamConnectionContainer != null);
            Debug.Assert(m_protocol != null);
            Debug.Assert(m_endpointStreamConnection != null);
            IStreamConnection connection = m_streamConnectionContainer.CreateConnection(m_protocol);
            SetConnection(connection, true);
            m_streamConnectionContainer.Connect(m_endpointStreamConnection, connection, m_connectionProperties);
        }

        private void RetriggerActivityTimer()
        {
            m_activityTimer.Stop();
            if (m_activityTimeout > 0)
            {
                m_activityTimer.Interval = m_activityTimeout;
                m_activityTimer.Start();
            }
        }

        public void SetConnection(IStreamConnection? connection, bool verified)
        {
            if (connection != null)
            {
                if (m_protocol != null)
                {
                    InitProtocolValues();
                    m_protocol.SetCallback(this);
                    m_protocol.SetConnection(connection);
                }

                m_connectionId = connection.ConnectionId;
                Activity();
            }
            AddSessionToList(verified);
        }
        public void SetProtocol(IProtocol protocol)
        {
            long connectionId = 0;
            IStreamConnection? connection = protocol.Connection;
            if (connection != null)
            {
                connectionId = connection.ConnectionId;
            }

            lock (m_mutex)
            {
                if (m_protocolFlagIsMultiConnectionSession)
                {
                    if (connectionId != 0)
                    {
                        if (!m_multiProtocols.ContainsKey(connectionId))
                        {
                            CleanupMultiConnection();
                            m_multiProtocols[connectionId] = protocol;
                        }
                    }
                }
                else
                {
                    Debug.Assert(m_protocol != null);
                    protocol.MoveOldProtocolState(m_protocol);
                    m_connectionId = connectionId;
                    m_protocol = protocol;
                }
                protocol.SetCallback(this);
            }
        }
        public void SetSessionNameInternal(string sessionName)
        {
            bool setSuccessful = m_protocolSessionList.SetSessionName(m_sessionId, sessionName);
            if (setSuccessful)
            {
                lock (m_mutex)
                {
                    m_verified = true;
                    m_sessionName = sessionName;
                }
                Connected();    // we are in the poller loop  thread
            }
        }

        // IProtocolCallback
        public void Connected()
        {
            bool trigger = false;
            lock (m_mutex)
            {
                if (!m_triggerConnected)
                {
                    trigger = true;
                    m_triggerConnected = true;
                }
            }

            if (trigger)
            {
                if (m_executor != null)
                {
                    m_executor.AddAction(() => {
                        m_callback.Connected(this);
                    }, m_instanceId);
                }
                else
                {
                    m_callback.Connected(this);
                }
            }
        }
        public void Disconnected()
        {
            bool doDisconnected = false;
            lock (m_mutex)
            {
                PollRelease();

                doDisconnected = (!m_triggerDisconnected) && (m_triggerConnected || m_protocolSet);
                m_triggerDisconnected = true;
            }

            if (doDisconnected)
            {
                if (m_executor != null)
                {
                    m_executor.AddAction(() => {
                        m_callback.Disconnected(this);
                    }, m_instanceId);
                }
                else
                {
                     m_callback.Disconnected(this);
                }
            }

            m_protocolSessionList.RemoveProtocolSession(m_sessionId);
        }
        public void DisconnectedVirtualSession(string virtualSessionId)
        {
            if (m_executor != null)
            {
                m_executor.AddAction(() => {
                    m_callback.DisconnectedVirtualSession(this, virtualSessionId);
                }, m_instanceId);
            }
            else
            {
                m_callback.DisconnectedVirtualSession(this, virtualSessionId);
            }
        }
        public void Received(IMessage message, long connectionId)
        {
            Activity();
            bool writeChannelIdIntoEchoData = false;
            if (m_protocolFlagIsMultiConnectionSession && connectionId != 0)
            {
                writeChannelIdIntoEchoData = (connectionId != m_connectionId);
            }

            if (writeChannelIdIntoEchoData)
            {
                //todo
                //Variant & echoData = message->getEchoData();
                //if (echoData.getType() == VARTYPE_NONE)
                //{
                //    echoData = VariantStruct{ { FMQ_CONNECTION_ID, connectionId} };
                //}
                //else
                //{
                //    echoData.add(FMQ_CONNECTION_ID, connectionId);
                //}
            }

            if (m_executor != null)
            {
                m_executor.AddAction(() => {
                    m_callback.Received(this, message);
                }, m_instanceId);
            }
            else
            {
                m_callback.Received(this, message);
            }
        }
        public void SocketConnected()
        {
            if (m_executor != null)
            {
                m_executor.AddAction(() => {
                    m_callback.SocketConnected(this);
                }, m_instanceId);
            }
            else
            {
                m_callback.SocketConnected(this);
            }
        }
        public void SocketDisconnected()
        {
            if (m_executor != null)
            {
                m_executor.AddAction(() => {
                    m_callback.SocketDisconnected(this);
                }, m_instanceId);
            }
            else
            {
                m_callback.SocketDisconnected(this);
            }
        }
        public void Reconnect()
        {
            Debug.Assert(m_endpointStreamConnection != null);
            Debug.Assert(m_streamConnectionContainer != null);
            Debug.Assert(m_protocol != null);
            IStreamConnection connection = m_streamConnectionContainer.CreateConnection(m_protocol);
            IProtocol? protocol = null;
            string? endpointStreamConnection;
            lock (m_mutex)
            {
                protocol = m_protocol;
                endpointStreamConnection = m_endpointStreamConnection;
            }
            m_connectionId = connection.ConnectionId;
            protocol.SetConnection(connection);
            m_streamConnectionContainer.Connect(endpointStreamConnection, connection, m_connectionProperties);
        }
        public bool FindSessionByName(string sessionName, IProtocol protocol)
        {
            IProtocolSessionPrivate? session = m_protocolSessionList.FindSessionByName(sessionName);
            if (session != null)
            {
                if (session != this)
                {
                    if (m_verified)
                    {
                        session.SetProtocol(protocol);
                    }
                    else
                    {
                        Disconnected(); // we are in the poller loop thread
                        IProtocol? prot = null;
                        lock (m_mutex)
                        {
                            prot = m_protocol;
                            m_protocol = null;
                            m_connectionId = 0;
                        }
                        Debug.Assert(prot != null);
                        session.SetProtocol(prot);
                    }
                }
                return true;
            }
            return false;
        }
        public void SetSessionName(string sessionName, IProtocol protocol, IStreamConnection connection)
        {
            bool setSuccessful = m_protocolSessionList.SetSessionName(m_sessionId, sessionName);
            if (setSuccessful)
            {
                lock (m_mutex)
                {
                    m_verified = true;
                    m_sessionName = sessionName;
                }
                Connected();    // we are in the poller loop  thread
            }
            else
            {
                IProtocolSessionPrivate protocolSession = new ProtocolSession(m_callback, m_executor, protocol, m_protocolSessionList, m_bindProperties, m_contentType);
                protocolSession.SetConnection(connection, false);
                protocolSession.SetSessionNameInternal(sessionName);
            }
        }
        public void PollRequest(IProtocol protocol, int timeout, int pollCountMax)
        {
            Debug.Assert(protocol != null);
            Activity();
            lock (m_mutex)
            {
                PollRelease();
                m_pollCounter = 0;
                m_pollCountMax = pollCountMax;
                m_pollProtocol = protocol;
                m_pollTimer.Stop();
                if (timeout > 0)
                {
                    m_pollTimer.Interval = timeout;
                }
                
                if (m_pollMessages.Count > 0)
                {
                    IMessage? pollReply = protocol.PollReply(m_pollMessages);
                    Debug.Assert(pollReply != null);    // wrong implementation of the protocol
                    m_pollMessages.Clear();
                    SendMessage(pollReply, protocol);
                }

                if (timeout == 0 || (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax))
                {
                    PollRelease();
                }
            }
        }
        public void Activity()
        {
            RetriggerActivityTimer();
        }
        public int ActivityTimeout
        {
            set
            {
                m_activityTimeout = value;
            }
        }
        public int PollMaxRequests
        {
            set
            {
                m_pollMaxRequests = value;
            }
        }
        public void DisconnectedMultiConnection(IProtocol protocol)
        {
            lock (m_mutex)
            {
                if (m_pollProtocol == protocol)
                {
                    m_pollProtocol = null;
                    m_pollTimer.Stop();
                }
            }

            if (!m_verified)
            {
                Disconnected();
            }
        }

        private void InitProtocolValues()
        {
            IProtocol? protocol = m_protocol;
            Debug.Assert(protocol != null);
            m_protocolId = protocol.ProtocolId;
            m_protocolFlagMessagesResendable = protocol.AreMessagesResendable;
            m_protocolFlagSupportMetainfo = protocol.DoesSupportMetainfo;
            m_protocolFlagNeedsReply = protocol.NeedsReply;
            m_protocolFlagIsMultiConnectionSession = protocol.IsMultiConnectionSession;
            m_protocolFlagIsSendRequestByPoll = protocol.IsSendRequestByPoll;
            m_protocolFlagSupportFileTransfer = protocol.DoesSupportFileTransfer;
            m_messageFactory = protocol.MessageFactory;

            Thread.MemoryBarrier();
            m_protocolSet = true;
        }

        private IMessage ConvertMessageToProtocol(IMessage msg)
        {
            IMessage? message = msg;
            if (message.ProtocolId != m_protocolId ||
                (!m_protocolFlagMessagesResendable && message.WasSent) ||
                (message.GetTotalSendPayloadSize() == 0 && msg.GetReceivePayload().Length > 0))
            {
                message = null;
                if (m_protocolFlagMessagesResendable)
                {
                    message = msg.GetMessage(m_protocolId);
                }
                if (message == null)
                {
                    message = CreateMessage();
//todo                    message.GetAllMetainfo() = msg.GetAllMetainfo();
                    int sizePayload = msg.GetTotalSendPayloadSize();
                    if (sizePayload > 0)
                    {
                        BufferRef payload = message.AddSendPayload(sizePayload);
                        IList<BufferRef> payloads = msg.GetAllSendPayloads();
                        int offset = 0;
                        foreach (BufferRef bufferRef in payloads)
                        {
                            Debug.Assert(offset + bufferRef.Length <= sizePayload);
                            bufferRef.Buffer.CopyTo(payload.Buffer, offset);
                            offset += bufferRef.Length;
                        }
                        Debug.Assert(offset == sizePayload);
                    }
                    else
                    {
                        BufferRef receivePayload = msg.GetReceivePayload();
                        BufferRef payload = message.AddSendPayload(receivePayload.Length);
                        receivePayload.Buffer.CopyTo(payload.Buffer, 0);
                    }
                    if (m_protocolFlagMessagesResendable)
                    {
                        msg.AddMessage(message);
                    }
                }
            }

            return message;
        }
        private void SendBufferedMessages()
        {
            // mutext is already locked
            foreach (var message in m_messagesBuffered)
            {
                SendMessage(message, m_protocol);
            }
            m_messagesBuffered.Clear();
        }
        private void AddSessionToList(bool verified)
        {
            lock (m_mutex)
            {
                m_verified = verified;
            }

            m_protocolSessionList.AddProtocolSession(this, m_sessionId, verified);
        }
        private void GetProtocolFromConnectionId(ref IProtocol? protocol, long connectionId)
        {
            // mutext is already locked
            Debug.Assert(protocol != null);
            Debug.Assert(protocol.Connection != null);
            if (connectionId != 0 && connectionId != protocol.Connection.ConnectionId)
            {
                protocol = null;
                m_multiProtocols.TryGetValue(connectionId, out protocol);
            }
        }
        private void SendMessage(IMessage? message, IProtocol? protocol)
        {
            // the mutex must be locked before calling sendMessage, lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
            if (protocol != null && message != null)
            {
                IMessage messageProtocol = ConvertMessageToProtocol(message);
                protocol.SendMessage(messageProtocol);
            }
        }
        private void CleanupMultiConnection()
        {
            IList<long> connectionIdsToRemove = new List<long>();
            foreach (var protocol in m_multiProtocols)
            {
                IStreamConnection? connection = protocol.Value.Connection;
                if (connection == null || connection.ConnectionData.Stream == null)
                {
                    connectionIdsToRemove.Add(protocol.Key);
                }
            }
            foreach (var connectionId in connectionIdsToRemove)
            {
                m_multiProtocols.Remove(connectionId);
            }
        }
        private void PollRelease()
        {
            // mutext is already locked
            if (m_pollProtocol != null)
            {
                IMessage? reply = m_pollProtocol.PollReply();    // wrong implementation of the protocol
                Debug.Assert(reply != null);
                if (reply != null)
                {
                    //todo  Variant & controlData = reply->getControlData();
                    //todo  controlData.add("fmq_poll_stop", true);
                    SendMessage(reply, m_pollProtocol);
                }
                m_pollProtocol = null;
                m_pollTimer.Stop();
            }
        }

        static readonly long INSTANCEID_PREFIX = 0x0100000000000000;
        //todo static readonly string FMQ_CONNECTION_ID = "fmq_echo_connid";


        readonly IProtocolSessionCallback               m_callback;
        readonly IExecutor?                             m_executor;
        IProtocol?                                      m_protocol = null;
        long                                            m_connectionId;
        readonly IDictionary<long, IProtocol>           m_multiProtocols = new Dictionary<long, IProtocol>();

        readonly long                                   m_sessionId;
        readonly long                                   m_instanceId;
        readonly IProtocolSessionList                   m_protocolSessionList;
        int                                             m_contentType;
        uint                                            m_protocolId;
        bool                                            m_protocolFlagMessagesResendable = false;
        bool                                            m_protocolFlagSupportMetainfo = false;
        bool                                            m_protocolFlagNeedsReply = false;
        bool                                            m_protocolFlagIsMultiConnectionSession = false;
        bool                                            m_protocolFlagIsSendRequestByPoll = false;
        bool                                            m_protocolFlagSupportFileTransfer = false;
        FuncCreateMessage?                              m_messageFactory = null;
        bool                                            m_protocolSet = false;   // atomic
        bool                                            m_triggerConnected = false;
        bool                                            m_triggerDisconnected = false;

        readonly IStreamConnectionContainer?            m_streamConnectionContainer = null;
        string?                                         m_endpointStreamConnection = null;

        BindProperties?                                 m_bindProperties = null;
        ConnectProperties?                              m_connectionProperties = null;

        IList<IMessage>                                 m_messagesBuffered = new List<IMessage>();

        IList<IMessage>                                 m_pollMessages = new List<IMessage>();
        int                                             m_pollMaxRequests = 10000;
        IProtocol?                                      m_pollProtocol = null;
        System.Timers.Timer                             m_pollTimer = new System.Timers.Timer();
        int                                             m_pollCountMax;
        int                                             m_pollCounter;

        int                                             m_activityTimeout = -1;
        System.Timers.Timer                             m_activityTimer = new System.Timers.Timer();

        bool                                            m_verified = false;
        string                                          m_sessionName = "";

        readonly object                                 m_mutex = new object();
    }

}   // namespace finalmq
