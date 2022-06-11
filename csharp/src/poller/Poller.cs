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


using System;
using System.Net.Sockets;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace finalmq
{
    public class DescriptorInfo
    {
        public void Clear()
        {
            socket = null;
            readable = false;
            writable = false;
            disconnected = false;
            bytesToRead = 0;
        }
        public Socket? socket = null;
        public bool readable = false;
        public bool writable = false;
        public bool disconnected = false;
        public int bytesToRead = 0;
    };




    public class PollerResult
    {
        bool m_timeout = false;
        int m_releaseWait = 0;
        IDictionary<Socket, DescriptorInfo> m_descriptorInfos = new Dictionary<Socket, DescriptorInfo>();

        public bool Timeout
        {
            get => m_timeout;
            set
            {
                m_timeout = value;
            }
        }

        public int ReleaseWait
        {
            get => m_releaseWait;
            set
            {
                m_releaseWait = value;
            }
        }

        public IDictionary<Socket, DescriptorInfo> DescriptorInfos
        {
            get => m_descriptorInfos;
        }

        public void Clear()
        {
            m_timeout = false;
            m_releaseWait = 0;
            m_descriptorInfos.Clear();
        }
    }



    public interface IPoller
    {
        void Init();
        void AddSocket(Socket socket);
        void AddSocketEnableRead(Socket socket);
        void RemoveSocket(Socket socket);
        void EnableRead(Socket socket);
        void DisableRead(Socket socket);
        void EnableWrite(Socket socket);
        void DisableWrite(Socket socket);
        PollerResult Wait(int timeout);
        void ReleaseWait(int info);
    }

    public class PollerImplSelect : IPoller
    {
        Socket? m_controlSocketRead = null;
        Socket? m_controlSocketWrite = null;

        PollerResult m_result = new PollerResult();

        HashSet<Socket> m_readfdsCached = new HashSet<Socket>();
        HashSet<Socket> m_writefdsCached = new HashSet<Socket>();
        HashSet<Socket> m_socketDescriptors = new HashSet<Socket>();
        IList<Socket> m_readfdsOriginal = new List<Socket>();
        IList<Socket> m_writefdsOriginal = new List<Socket>();
        IList<Socket> m_errorfdsOriginal = new List<Socket>();
        IList<Socket> m_readfds = new List<Socket>();
        IList<Socket> m_writefds = new List<Socket>();
        IList<Socket> m_errorfds = new List<Socket>();
        int m_socketDescriptorsStable = 1; // atomic
        int m_fdsReadStable = 1; // atomic;
        int m_fdsWriteStable = 1; // atomic;
        int m_releaseFlags = 0; // atomic

        private Object m_mutex = new Object();

        public PollerImplSelect()
        {
        }

        public void Init()
        {
            Platform.Instance.MakeSocketPair(out m_controlSocketWrite, out m_controlSocketRead);
            if (m_controlSocketRead != null)
            {
                AddSocketEnableRead(m_controlSocketRead);
            }
            else
            {
                throw new InvalidOperationException("MakeSocketPair failed");
            }
        }

        public void AddSocket(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                bool result = m_socketDescriptors.Add(socket);
                if (result)
                {
                    changed = true;
                }
                else
                {
                    // socket already added
                }
            }
            if (changed)
            {
                SockedDescriptorsHaveChanged();
            }
        }

        public void AddSocketEnableRead(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                bool result = m_socketDescriptors.Add(socket);
                if (result)
                {
                    m_readfdsCached.Add(socket);
                    changed = true;
                }
                else
                {
                    // socket already added
                }
            }
            if (changed)
            {
                SockedDescriptorsAndFdsReadHaveChanged();
            }
        }

        public void RemoveSocket(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                if (m_socketDescriptors.Contains(socket))
                {
                    m_readfdsCached.Remove(socket);
                    m_writefdsCached.Remove(socket);
                    m_socketDescriptors.Remove(socket);
                    changed = true;
                }
                else
                {
                    // socket not added
                }
            }
            if (changed)
            {
                SockedDescriptorsAndAllFdsHaveChanged();
            }
        }

        public void EnableRead(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                if (m_socketDescriptors.Contains(socket))
                {
                    changed = m_readfdsCached.Add(socket);
                }
                else
                {
                    // socket not added
                }
            }
            if (changed)
            {
                FdsReadHaveChanged();
            }
        }

        public void DisableRead(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                if (m_socketDescriptors.Contains(socket))
                {
                    changed = m_readfdsCached.Remove(socket);
                }
                else
                {
                    // socket not added
                }
            }
            if (changed)
            {
                FdsReadHaveChanged();
            }
        }

        public void EnableWrite(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                if (m_socketDescriptors.Contains(socket))
                {
                    changed = m_writefdsCached.Add(socket);
                }
                else
                {
                    // socket not added
                }
            }
            if (changed)
            {
                FdsWriteHaveChanged();
            }
        }

        public void DisableWrite(Socket socket)
        {
            bool changed = false;
            lock (m_mutex)
            {
                if (m_socketDescriptors.Contains(socket))
                {
                    changed = m_writefdsCached.Remove(socket);
                }
                else
                {
                    // socket not added
                }
            }
            if (changed)
            {
                FdsWriteHaveChanged();
            }
        }

        private void SockedDescriptorsHaveChanged()
        {
            Interlocked.Exchange(ref m_socketDescriptorsStable, 0);
            ReleaseWait(0);
        }

        private void SockedDescriptorsAndFdsReadHaveChanged()
        {
            Interlocked.Exchange(ref m_socketDescriptorsStable, 0);
            Interlocked.Exchange(ref m_fdsReadStable, 0);
            ReleaseWait(0);
        }

        private void SockedDescriptorsAndAllFdsHaveChanged()
        {
            Interlocked.Exchange(ref m_socketDescriptorsStable, 0);
            Interlocked.Exchange(ref m_fdsReadStable, 0);
            Interlocked.Exchange(ref m_fdsWriteStable, 0);
            ReleaseWait(0);
        }

        private void FdsReadHaveChanged()
        {
            Interlocked.Exchange(ref m_fdsReadStable, 0);
            ReleaseWait(0);
        }

        private void FdsWriteHaveChanged()
        {
            Interlocked.Exchange(ref m_fdsWriteStable, 0);
            ReleaseWait(0);
        }


        void UpdateSocketDescriptors()
        {
            m_errorfdsOriginal.Clear();
            foreach (Socket desc in m_socketDescriptors)
            {
                m_errorfdsOriginal.Add(desc);
            }
        }

        void UpdateFdsRead()
        {
            m_readfdsOriginal.Clear();
            foreach (Socket desc in m_readfdsCached)
            {
                m_readfdsOriginal.Add(desc);
            }
        }

        void UpdateFdsWrite()
        {
            m_writefdsOriginal.Clear();
            foreach (Socket desc in m_writefdsCached)
            {
                m_writefdsOriginal.Add(desc);
            }
        }

        private DescriptorInfo getDescriptorInfo(Socket socket)
        {
            var desciptorInfos = m_result.DescriptorInfos;
            DescriptorInfo? descriptorInfo = null;
            try
            {
                descriptorInfo = desciptorInfos[socket];
            }
            catch (KeyNotFoundException)
            {
                descriptorInfo = new DescriptorInfo();
                desciptorInfos[socket] = descriptorInfo;
                descriptorInfo.socket = socket;
            }
            return descriptorInfo;
        }

        private void CollectSockets()
        {
            m_result.Clear();

            if (m_readfds.Count == 0 &&
                m_writefds.Count == 0 &&
                m_errorfds.Count == 0)
            {
                m_result.Timeout = true;
            }
            else
            {
                var desciptorInfos = m_result.DescriptorInfos;

                foreach (Socket socket in m_readfds)
                {
                    int countRead = 0;

                    try
                    {
                        countRead = Platform.Instance.GetAvailable(socket);
                    }
                    catch (Exception)
                    {
                    }

                    if (socket == m_controlSocketRead)
                    {
                        bool controlSocketBroken = false;
                        if (countRead > 0)
                        {
                            // read pending bytes from control socket
                            byte[] buffer = new byte[countRead];
                            try
                            {
                                Platform.Instance.Receive(socket, buffer, 0, countRead);
                            }
                            catch (Exception)
                            {
                                // control socket broken
                                controlSocketBroken = true;
                            }
                            m_result.ReleaseWait = Interlocked.Exchange(ref m_releaseFlags, 0);
                        }
                        else
                        {
                            // control socket broken
                            controlSocketBroken = true;
                        }
                        if (controlSocketBroken)
                        {
                            //todo: handle broken control socket (should never happen)
                        }
                    }
                    else
                    {
                        DescriptorInfo descriptorInfo = getDescriptorInfo(socket);
                        descriptorInfo.readable = true;
                        descriptorInfo.bytesToRead = countRead;
                    }
                }
                foreach (Socket socket in m_writefds)
                {
                    DescriptorInfo descriptorInfo = getDescriptorInfo(socket);
                    descriptorInfo.writable = true;
                }
                foreach (Socket socket in m_errorfds)
                {
                    if (socket == m_controlSocketRead)
                    {
                        // control socket broken
                        //todo: handle broken control socket (should never happen)
                    }
                    else
                    {
                        DescriptorInfo descriptorInfo = getDescriptorInfo(socket);
                        descriptorInfo.disconnected = true;
                    }
                }
            }
        }

        private static int MILLITOMICRO = 1000;
        public PollerResult Wait(int timeout)
        {
            do
            {
                int socketDescriptorsStable = Interlocked.Exchange(ref m_socketDescriptorsStable, 1);
                int fdsReadStable = Interlocked.Exchange(ref m_fdsReadStable, 1);
                int fdsWriteStable = Interlocked.Exchange(ref m_fdsWriteStable, 1);

                if (socketDescriptorsStable == 0 || fdsReadStable == 0 || fdsWriteStable == 0)
                {
                    lock (m_mutex)
                    {
                        if (socketDescriptorsStable == 0)
                        {
                            UpdateSocketDescriptors();
                        }

                        if (fdsReadStable == 0)
                        {
                            UpdateFdsRead();
                        }
                        if (fdsWriteStable == 0)
                        {
                            UpdateFdsWrite();
                        }
                    }
                }

                // copy fds
                m_readfds = new List<Socket>(m_readfdsOriginal);
                m_writefds = new List<Socket>(m_writefdsOriginal);
                m_errorfds = new List<Socket>(m_errorfdsOriginal);

                int microSeconds = -1;
                if (timeout >= 0)
                {
                    microSeconds = timeout * MILLITOMICRO;
                }

                Platform.Instance.Select(m_readfds, m_writefds, m_errorfds, microSeconds);

                CollectSockets();

            } while (!m_result.Timeout && m_result.ReleaseWait == 0 && (m_result.DescriptorInfos.Count == 0));

            return m_result;
        }

        public void ReleaseWait(int info)
        {
            Interlocked.Add(ref m_releaseFlags, info);
            if (m_controlSocketWrite != null)
            {
                byte[] buffer = { 0 };
                Platform.Instance.Send(m_controlSocketWrite, buffer, 0, 1, SocketFlags.None);
            }
        }

    }


}