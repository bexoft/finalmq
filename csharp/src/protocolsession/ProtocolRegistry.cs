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

    interface IProtocolRegistry
    {
        void RegisterProtocolFactory(string protocolName, uint protocolId, FuncCreateProtocol protocolFactory);
        FuncCreateProtocol GetProtocolFactory(string protocolName);
        FuncCreateProtocol GetProtocolFactory(uint protocolId);
    };

    class ProtocolRegistryImpl : IProtocolRegistry
    {
        public void RegisterProtocolFactory(string protocolName, uint protocolId, FuncCreateProtocol protocolFactory)
        {
            m_protocolNameToFactory[protocolName] = protocolFactory;
            m_protocolIdToFactory[protocolId] = protocolFactory;
        }

        public FuncCreateProtocol GetProtocolFactory(string protocolName)
        {
            FuncCreateProtocol? factory = null;
            m_protocolNameToFactory.TryGetValue(protocolName, out factory);
            if (factory != null)
            {
                return factory;
            }
            throw new System.Collections.Generic.KeyNotFoundException("Protocol name " + protocolName + " not found");
        }

        public FuncCreateProtocol GetProtocolFactory(uint protocolId)
        {
            FuncCreateProtocol? factory = null;
            m_protocolIdToFactory.TryGetValue(protocolId, out factory);
            if (factory != null)
            {
                return factory;
            }
            throw new System.Collections.Generic.KeyNotFoundException("Protocol ID " + protocolId + " not found");
        }

        private IDictionary<string, FuncCreateProtocol>   m_protocolNameToFactory = new Dictionary<string, FuncCreateProtocol>();
        private IDictionary<uint, FuncCreateProtocol>     m_protocolIdToFactory   = new Dictionary<uint, FuncCreateProtocol>();
    };


    class ProtocolRegistry
    {
        public static IProtocolRegistry Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }

        private ProtocolRegistry()
        {
        }

        private static IProtocolRegistry m_instance = new ProtocolRegistryImpl();
    }


}   // namespace finalmq
