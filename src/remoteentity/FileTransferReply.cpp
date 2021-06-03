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

#include "finalmq/remoteentity/FileTransferReply.h"
#include "finalmq/remoteentity/RemoteEntity.h"

#include <fcntl.h>


namespace finalmq {



FileTransferReply::FileTransferReply(int numberOfWorkerThreads)
    : m_executor(std::make_unique<Executor>())
{
    for (int i = 0; i < numberOfWorkerThreads; ++i)
    {
        m_threads.emplace_back(std::thread([this]() {
            m_executor->run();
        }));
    }
}


FileTransferReply::~FileTransferReply()
{
    m_executor->terminate();
    for (size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i].join();
    }
}



bool FileTransferReply::replyFile(ReplyContextPtr& replyContext, const std::string& filename, IMessage::Metainfo* metainfo)
{
    bool handeled = false;

    struct stat statdata;
    memset(&statdata, 0, sizeof(statdata));
    int res = OperatingSystem::instance().stat(filename.c_str(), &statdata);

    if (res == 0)
    {
        handeled = true;

//            if (replyContext->doesSupportFileTransfer())
//            {
//                replyContext->reply(filename);
//            }
//            else
//            {
            int sizeFile = statdata.st_size;

            std::shared_ptr<IMessage::Metainfo> mi;
            if (metainfo)
            {
                mi = std::make_shared<IMessage::Metainfo>(std::move(*metainfo));
                metainfo->clear();
            }

            m_executor->addAction([filename, sizeFile, replyContext, mi]() {
                int flags = O_RDONLY;
#ifdef WIN32
                flags |= O_BINARY;
#endif
                int fd = OperatingSystem::instance().open(filename.c_str(), flags);
                if (fd != -1)
                {
                    remoteentity::Bytes reply;
                    reply.data.resize(sizeFile);

                    char* buf = reply.data.data();
                    int len = static_cast<int>(reply.data.size());
                    int err = 0;
                    int lenReceived = 0;
                    bool ex = false;
                    while (!ex)
                    {
                        do
                        {
                            err = OperatingSystem::instance().read(fd, buf, len);
                        } while (err == -1 && OperatingSystem::instance().getLastError() == SOCKETERROR(EINTR));

                        if (err > 0)
                        {
                            buf += err;
                            len -= err;
                            lenReceived += err;
                            err = 0;
                            assert(len >= 0);
                            if (len == 0)
                            {
                                ex = true;
                            }
                        }
                        else
                        {
                            ex = true;
                        }
                    }
                    if (lenReceived < static_cast<int>(reply.data.size()))
                    {
                        reply.data.resize(lenReceived);
                    }
                    replyContext->reply(reply, mi.get());
                }
                else
                {
                    // not found
                    replyContext->reply(finalmq::remoteentity::Status::STATUS_ENTITY_NOT_FOUND);
                }
            });
//      }
    }
    return handeled;
}


}   // namespace finalmq
