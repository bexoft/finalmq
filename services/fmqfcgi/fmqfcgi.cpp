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

#include "remoteentity/RemoteEntityContainer.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"
#include "logger/Logger.h"

#include <thread>
#include <iostream>

#include <fcgiapp.h>


//using finalmq::RemoteEntity;
//using finalmq::RemoteEntityContainer;
//using finalmq::IRemoteEntityContainer;
//using finalmq::PeerId;
//using finalmq::PeerEvent;
//using finalmq::ReplyContextUPtr;
//using finalmq::ProtocolHeaderBinarySizeFactory;
//using finalmq::ProtocolDelimiterFactory;
//using finalmq::RemoteEntityContentType;
//using finalmq::IProtocolSessionPtr;
//using finalmq::ConnectionData;
//using finalmq::ConnectionEvent;
using finalmq::Logger;
using finalmq::LogContext;
//using finalmq::fmqreg::RegisterService;
//using finalmq::fmqreg::GetService;
//using finalmq::fmqreg::GetServiceReply;
//using finalmq::fmqreg::Service;




int main(void)
{

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {

        std::string outstr("Content-type: text/html\r\n"
            "\r\n"
            "<html>\n"
            "  <head>\n"
            "    <title>Hello, World!</title>\n"
            "  </head>\n"
            "  <body>\n"
            "    <h1>Hello, World</h1>\n"
            "  </body>\n"
            "</html>\n");
        FCGX_PutStr(outstr.c_str(), outstr.size(), request.out);
    }


    return 0;
}

