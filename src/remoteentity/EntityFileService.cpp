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
#include "finalmq/helpers/Executor.h"

#include <fcntl.h>


namespace finalmq {


FileTransferReply::FileTransferReply(const std::string& baseDirectory, int numberOfWorkerThreads)
    : m_baseDirectory(baseDirectory)
    , m_executor(std::make_unique<Executor>())
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


template<class T>
class MoveObjectByCopy
{
public:
    MoveObjectByCopy(T& moveObject)
    {
        m_moveObject = std::move(moveObject);
    }
    MoveObjectByCopy(const MoveObjectByCopy& rhs)
    {
        m_moveObject = std::move(const_cast<MoveObjectByCopy&>(rhs).m_moveObject);
    }
    const T& operator *() const
    {
        return m_moveObject;
    }
private:
    T   m_moveObject;
};


bool FileTransferReply::replyFile(ReplyContextUPtr& replyContext)
{
    bool handeled = false;
    std::string* path = replyContext->getMetainfo("fmq_path");
    if (path && !path->empty())
    {
        std::string filename = m_baseDirectory + *path;

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

                MoveObjectByCopy<ReplyContextUPtr> replyContextShell(replyContext);
                m_executor->addAction([filename, sizeFile, replyContextShell]() {
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
                        (*replyContextShell)->reply(reply);
                    }
                    else
                    {
                        // not found
                        (*replyContextShell)->reply(finalmq::remoteentity::Status::STATUS_ENTITY_NOT_FOUND);
                    }
                });
//            }
        }
    }
    return handeled;
}


///////////////////////////////////////////////////////



EntityFileServer::EntityFileServer(const std::string& baseDirectory, int numberOfWorkerThreads)
    : m_fileTransfer(baseDirectory, numberOfWorkerThreads)
{
    registerCommandFunction("*", [this](ReplyContextUPtr& replyContext, const StructBasePtr& /*structBase*/) {
        bool handeled = m_fileTransfer.replyFile(replyContext);
        if (!handeled)
        {
            // not found
            replyContext->reply(finalmq::remoteentity::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });
}





}   // namespace finalmq
