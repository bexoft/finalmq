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


#include "platformspecific.h"
#include "processes.h"
#include "finalmq/interfaces/fmqprocess.fmq.h"
#include "finalmq/remoteentity/RemoteEntity.h"

#include <string>
#include <vector>

using namespace finalmq;
using namespace finalmq::fmqprocess;


class TransactionList;

enum TransactionMode
{
	TRANSACTIONMODE_NON		= 0,
	TRANSACTIONMODE_START	= 1,
	TRANSACTIONMODE_STOP	= 2,
};

struct ReportProcessStatus
{
    long long       timestamp;
    ReportId        id;
    ProcessStatus   process;
};

class TransactionBase
{
public:
	TransactionBase(TransactionMode	mode, const std::string& owner, const RequestContextPtr& requestContext, Processes& processes, TransactionList& transactions, WatchStartedProcessesForTermination* watchStartedProcesses);
	virtual ~TransactionBase();
	const std::string& getOwner() const;
	TransactionMode getMode() const;
	const std::vector<std::string>& getProcessNamesForTransaction() const;
    bool executeIntern(std::vector<ReportProcessStatus>& report, bool& startupError);
//	void abortTransaction();
//	bool isAborted() const;
    virtual void init(std::vector<ReportProcessStatus>& report) = 0;
    virtual bool execute(std::vector<ReportProcessStatus>& report, bool& startupError) = 0;
//	virtual void abort() = 0;
protected:
    void putReport(std::vector<ReportProcessStatus>& report, const ProcessStatus& process, ReportId id);

	bool									m_init;
	TransactionMode							m_mode;
	std::string								m_owner;	// id to process
	std::vector<std::string>				m_processNamesForTransaction;
	Processes&								m_processes;
	TransactionList&						m_transactions;
	WatchStartedProcessesForTermination*	m_watchStartedProcesses;
	RequestContextPtr						m_requestContext;
//	bool									m_aborted;
};


class TransactionList
{
public:
	typedef std::list<TransactionBase*>::iterator iterator;
	~TransactionList();
	void add(TransactionBase* ta);
	void clear();
	int size() const;
	bool empty() const;
	iterator begin();
	iterator end();
	void erase(iterator& it);
//	void getTransactionsOfProcess(const std::string& id, std::vector<TransactionBase*>& tas);
	bool isStarting();
    void execute(std::vector<ReportProcessStatus>& report, bool& startupError, bool restartAllowed);
private:
	std::list<TransactionBase*>	m_list;
};


