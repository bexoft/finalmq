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

#pragma once


#include "finalmq/helpers/BexDefines.h"
#include "finalmq/helpers/Executor.h"
#include "finalmq/streamconnection/IMessage.h"

#include <memory>
#include <string>
#include <vector>
#include <thread>


namespace finalmq {

class ReplyContext;
typedef std::shared_ptr<ReplyContext> ReplyContextPtr;

class SYMBOLEXP FileTransferReply
{
public:
    FileTransferReply(int numberOfWorkerThreads = 2);
    ~FileTransferReply();

    bool replyFile(const ReplyContextPtr& replyContext, const std::string& filename, IMessage::Metainfo* metainfo = nullptr);

private:
    std::string                 m_baseDirectory;
    std::unique_ptr<IExecutor>  m_executor;
    std::vector<std::thread>    m_threads;
};



}   // namespace finalmq
