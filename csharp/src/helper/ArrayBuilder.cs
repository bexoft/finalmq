
using System.Diagnostics;

namespace finalmq
{
    public class ArrayBuilder<T>
    {
        public ArrayBuilder(int blocksize = 1024)
        {
            blocksize = Math.Max(blocksize, 128);
            m_blocksize = blocksize;
        }

        public void Add(T element)
        {
            Debug.Assert(m_indexBlocks < m_blocks.Count);
            if (m_indexBlocks == -1)
            {
                m_blocks.Add(new T[m_blocksize]);
                m_indexBlocks = 0;
                m_indexElement = 0;
            }
            Debug.Assert(m_indexElement <= m_blocksize);
            if (m_indexElement == m_blocksize)
            {
                Debug.Assert(m_indexBlocks < m_blocks.Count);
                if (m_indexBlocks == m_blocks.Count - 1)
                {
                    m_blocks.Add(new T[m_blocksize]);
                }
                ++m_indexBlocks;
                m_indexElement = 0;
            }
            T[] array = m_blocks[m_indexBlocks];
            array[m_indexElement] = element;
            ++m_indexElement;
            ++m_totalElements;
        }

        public void Clear()
        {
            m_totalElements = 0;
            m_indexElement = 0;
            m_indexBlocks = -1;
        }

        public int Length
        {
            get
            {
                return m_totalElements;
            }
        }

        public T[] ToArray()
        {
            T[] array = new T[m_totalElements];
            if (m_blocks.Count > 0)
            {
                Debug.Assert(m_indexBlocks < m_blocks.Count);
                var blockLast = m_blocks[m_indexBlocks];
                int index = 0;
                foreach (var block in m_blocks)
                {
                    if (block != blockLast)
                    {
                        block.CopyTo(array, index);
                        index += block.Length;
                    }
                    else
                    {
                        Array.Copy(block, 0, array, index, m_indexElement);
                        Debug.Assert(m_totalElements == index + m_indexElement);
                        break;
                    }
                }
            }
            return array;
        }

        int m_totalElements = 0;
        int m_indexElement = 0;
        int m_indexBlocks = -1;
        readonly IList<T[]> m_blocks = new List<T[]>();
        readonly int m_blocksize;
    }
}
