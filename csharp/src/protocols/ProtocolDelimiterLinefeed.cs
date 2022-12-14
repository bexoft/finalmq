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

using System.Text;
using System.Runtime.CompilerServices;

namespace finalmq
{
    class ProtocolDelimiterLinefeed : ProtocolDelimiter
    {
        public static readonly uint PROTOCOL_ID = 3;
        public static readonly string PROTOCOL_NAME = "delimiter_lf";

        public ProtocolDelimiterLinefeed()
            : base(Encoding.UTF8.GetBytes("\n"))
        {
        }

        public override uint ProtocolId
        {
            get
            {
                return PROTOCOL_ID;
            }
        }
    };


    //---------------------------------------
    // register factory
    //---------------------------------------
    class RegisterProtocolDelimiterLinefeedFactory
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register()
        {
            ProtocolRegistry.Instance.RegisterProtocolFactory(ProtocolDelimiterLinefeed.PROTOCOL_NAME, ProtocolDelimiterLinefeed.PROTOCOL_ID, () => { return new ProtocolDelimiterLinefeed(); });
        }
    }

}
