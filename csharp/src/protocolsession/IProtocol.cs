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


namespace finalmq 
{

    public interface IProtocolCallback
    {
        void Connected();
        void Disconnected();
        void DisconnectedVirtualSession(string virtualSessionId);
        void Received(IMessage message, long connectionId = 0);
        void SocketConnected();
        void SocketDisconnected();
        void Reconnect();
        bool FindSessionByName(string sessionName, IProtocol protocol);
        void SetSessionName(string sessionName, IProtocol protocol, IStreamConnection connection);
        void PollRequest(IProtocol protocol, int timeout, int pollCountMax);
        void Activity();
        int ActivityTimeout { set; }
        int PollMaxRequests { set; }
        void DisconnectedMultiConnection(IProtocol protocol);
    }

    public delegate IMessage FuncCreateMessage();

    public interface IProtocol : IStreamConnectionCallback
    {
        void SetCallback(IProtocolCallback callback);
        IStreamConnection? Connection { get; set; }
        void Disconnect();
        uint ProtocolId { get; }
        bool AreMessagesResendable { get; }
        bool DoesSupportMetainfo { get; }
        bool DoesSupportSession { get; }
        bool NeedsReply { get; }
        bool IsMultiConnectionSession { get; }
        bool IsSendRequestByPoll { get; }
        bool DoesSupportFileTransfer { get; }
        bool IsSynchronousRequestReply { get; }
        FuncCreateMessage MessageFactory { get; }
        void SendMessage(IMessage message);
        void MoveOldProtocolState(IProtocol protocolOld);
        IMessage? PollReply(IList<IMessage> messages);
        void Subscribe(IList<string> subscribtions);
    }

    public delegate IProtocol FuncCreateProtocol(Variant? data);

}   // namespace finalmq
