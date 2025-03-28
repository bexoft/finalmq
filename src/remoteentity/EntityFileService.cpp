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

#include "finalmq/remoteentity/EntityFileService.h"

#include <fcntl.h>


namespace finalmq {




EntityFileServer::EntityFileServer(const std::string& baseDirectory)
    : m_baseDirectory(baseDirectory)
{
    if (m_baseDirectory.empty() || m_baseDirectory[m_baseDirectory.size() - 1] != '/')
    {
        m_baseDirectory += '/';
    }
    registerCommand<ConnectEntity>([](const RequestContextPtr& requestContext, const std::shared_ptr<ConnectEntity>& /*request*/) {
        requestContext->reply(Status::STATUS_ENTITY_NOT_FOUND);
    });

    registerCommandFunction("*tail*", "", [this](RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        bool handeled = false;
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            std::string filename = m_baseDirectory + *path;
            handeled = requestContext->replyFile(filename);
        }

        if (!handeled)
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });
}





}   // namespace finalmq
