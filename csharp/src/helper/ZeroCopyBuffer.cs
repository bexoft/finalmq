// MIT License

// Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


using System.Diagnostics;

namespace finalmq
{

    public class ZeroCopyBuffer : IZeroCopyBuffer
    {
        public BufferRef AddBuffer(int size, int reserve = 0)
        {
            Debug.Assert(size > 0);
            byte[] buffer = new byte[size];
            m_buffers.Add(new BufferRef(buffer));
            return m_buffers.Last();
        }
        public void DownsizeLastBuffer(int newSize)
        {
            if (m_buffers.Count == 0 && newSize == 0)
            {
                return;
            }

            Debug.Assert(m_buffers.Count != 0);

            if (newSize == 0)
            {
                m_buffers.RemoveAt(m_buffers.Count - 1);
            }
            else
            {
                m_buffers.Last().Length = newSize;
            }
        }
        public int RemainingSize 
        { 
            get
            { 
                return 0; 
            }
        }

        public byte[] GetData()
        {
            int size = 0;
            foreach (var buffer in m_buffers)
            {
                size += buffer.Length;
            };
            byte[] data = new byte[size];
            int offset = 0;
            foreach (var buffer in m_buffers)
            {
                Array.Copy(buffer.Buffer, 0, data, offset, buffer.Length);
                offset += buffer.Length;
            }

            return data;
        }

        private readonly IList<BufferRef> m_buffers = new List<BufferRef>();
    }


}   // namespace finalmq
