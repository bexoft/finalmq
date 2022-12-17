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

    public interface IProtocolSession
    {
        IMessage CreateMessage();
        void SendMessage(IMessage msg, bool isReply = false);
        long SessionId { get; }
        ConnectionData ConnectionData { get; }
        int ContentType { get; }
        bool DoesSupportMetainfo();
        bool NeedsReply();
        bool IsMultiConnectionSession();
        bool IsSendRequestByPoll();
        bool DoesSupportFileTransfer();
        void Disconnect();
        void Connect(string endpoint, ConnectProperties? connectionProperties = null, int contentType = 0);
        IExecutor? Executor { get; }
        void Subscribe(IList<string> subscribtions);
    }


    public interface IProtocolSessionCallback
    {
        void Connected(IProtocolSession session);
        void Disconnected(IProtocolSession session);
        void DisconnectedVirtualSession(IProtocolSession session, string virtualSessionId);
        void Received(IProtocolSession session, IMessage message);
        void SocketConnected(IProtocolSession session);
        void SocketDisconnected(IProtocolSession session);
    }

}   // namespace finalmq
