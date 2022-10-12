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

    internal interface IProtocolSessionList
    {
        long AddProtocolSession(IProtocolSessionPrivate protocolSession, bool verified);
        void RemoveProtocolSession(long sessionId);
        IList<IProtocolSessionPrivate> GetAllSessions();
        IProtocolSession GetSession(long sessionId);
        IProtocolSessionPrivate? FindSessionByName(string sessionName);
        bool SetSessionName(long sessionId, string sessionName);
    }

    internal class ProtocolSessionList : IProtocolSessionList
    {
        public ProtocolSessionList()
        {
        }

        public long AddProtocolSession(IProtocolSessionPrivate protocolSession, bool verified)
        {
            long sessionId = 0;
            lock (m_mutex)
            {
                sessionId = Interlocked.Increment(ref m_nextSessionId);
                m_connectionId2ProtocolSession[sessionId] = new SessionData(protocolSession, verified, "");
            }
            return sessionId;
        }
        public void RemoveProtocolSession(long sessionId)
        {
            lock (m_mutex)
            {
                m_connectionId2ProtocolSession.Remove(sessionId);
            }
        }
        public IList<IProtocolSessionPrivate> GetAllSessions()
        {
            IList<IProtocolSessionPrivate> protocolSessions = new List<IProtocolSessionPrivate>();
            lock (m_mutex)
            {
                foreach (var entry in m_connectionId2ProtocolSession)
                {
                    SessionData sessionData = entry.Value;
                    if (sessionData.verified)
                    {
                        protocolSessions.Add(sessionData.session);
                    }
                }
            }
            return protocolSessions;
        }
        public IProtocolSession GetSession(long sessionId)
        {
            lock (m_mutex)
            {
                SessionData? sessionData = null;
                m_connectionId2ProtocolSession.TryGetValue(sessionId, out sessionData);
                if (sessionData != null)
                {
                    return sessionData.session;
                }
            }
            throw new System.Collections.Generic.KeyNotFoundException("Session ID " + sessionId + " not found");
        }
        public IProtocolSessionPrivate? FindSessionByName(string sessionName)
        {
            lock (m_mutex)
            {
                foreach (var entry in m_connectionId2ProtocolSession)
                {
                    if (entry.Value.verified && entry.Value.name == sessionName)
                    {
                        return entry.Value.session;
                    }
                }
            }
            return null;
        }

        public bool SetSessionName(long sessionId, string sessionName)
        {
            bool ok = false;
            lock (m_mutex)
            {
                SessionData? sessionData = null;
                m_connectionId2ProtocolSession.TryGetValue(sessionId, out sessionData);
                if (sessionData != null && !sessionData.verified)
                {
                    ok = true;
                    sessionData.name = sessionName;
                    sessionData.verified = true;
                }
            }
            return ok;
        }

        class SessionData
        {
            public SessionData(IProtocolSessionPrivate session, bool verified, string name)
            {
                this.session = session;
                this.verified = verified;
                this.name = name;
            }
            public IProtocolSessionPrivate session;
            public bool verified = false;
            public string name;
        };

        private IDictionary<long, SessionData>  m_connectionId2ProtocolSession = new Dictionary<long, SessionData>();
        private static long                     m_nextSessionId = 1;
        private readonly object                 m_mutex = new object();
    }

}   // namespace finalmq
