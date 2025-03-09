#include "transactionbase.h"
#include <time.h>


TransactionBase::TransactionBase(TransactionMode mode, const std::string& owner, const RequestContextPtr& requestContext, Processes& processes, TransactionList& transactions, WatchStartedProcessesForTermination* watchStartedProcesses)
	: m_init(false)
	, m_mode(mode)
	, m_owner(owner)
	, m_processes(processes)
	, m_transactions(transactions)
	, m_watchStartedProcesses(watchStartedProcesses)
	, m_requestContext(requestContext)
//	, m_aborted(false)
{
	transactions.add(this);
}
TransactionBase::~TransactionBase()
{
}
const std::string& TransactionBase::getOwner() const
{
	return m_owner;
}
TransactionMode TransactionBase::getMode() const
{
	return m_mode;
}


const std::vector<std::string>& TransactionBase::getProcessNamesForTransaction() const
{
	return m_processNamesForTransaction;
}

bool TransactionBase::executeIntern(std::vector<ReportProcessStatus>& report, bool& startupError)
{
	if (!m_init)
	{
		init(report);
		m_init = true;
	}
	return execute(report, startupError);
}



void TransactionBase::putReport(std::vector<ReportProcessStatus>& report, const ProcessStatus& process, ReportId id)
{
	ReportProcessStatus entry;
    entry.timestamp		= time(nullptr);
    entry.process   	= process;
    entry.id			= id;
    report.push_back(entry);
}


//void TransactionBase::abortTransaction()
//{
//	if (!m_aborted)
//	{
//		m_aborted = true;
//		abort();
//	}
//}
//bool TransactionBase::isAborted() const
//{
//	return m_aborted;
//}


TransactionList::~TransactionList()
{
	clear();
}
void TransactionList::add(TransactionBase* ta)
{
	m_list.push_back(ta);
}
void TransactionList::clear()
{
	while (!m_list.empty())
	{
		delete m_list.front();
		m_list.pop_front();
	}
}
int TransactionList::size() const
{
	return static_cast<int>(m_list.size());
}
bool TransactionList::empty() const
{
	return m_list.empty();
}
TransactionList::iterator TransactionList::begin()
{
	return m_list.begin();
}
TransactionList::iterator TransactionList::end()
{
	return m_list.end();
}
void TransactionList::erase(iterator& it)
{
	delete *it;
	m_list.erase(it);
}
//void TransactionList::getTransactionsOfProcess(const std::string& id, std::vector<TransactionBase*>& tas)
//{
//	std::list<TransactionBase*>::iterator it = m_list.begin();
//	for ( ; it != m_list.end(); it++)
//	{
//		if ((*it)->getOwner() == id)
//		{
//			tas.push_back(*it);
//		}
//	}
//}
bool TransactionList::isStarting()
{
	std::list<TransactionBase*>::iterator it = m_list.begin();
	for ( ; it != m_list.end(); it++)
	{
		if ((*it)->getMode() == TRANSACTIONMODE_START)
		{
			return true;
		}
	}
	return false;
}

bool isTransactionAllowed(const TransactionBase& transaction, std::unordered_map< std::string, int >& modesForProcessNames)
{
	bool allowed = true;
	int mymode = transaction.getMode();
	const std::vector<std::string>& names = transaction.getProcessNamesForTransaction();
	for (size_t i=0; i<names.size(); i++)
	{
		const std::string& name = names[i];
		int& mode = modesForProcessNames[name];
		if (mode != 0 && mode != mymode)
		{
			allowed = false;
		}
		mode |= mymode;
	}
	return allowed;
}


void TransactionList::execute(std::vector<ReportProcessStatus>& report, bool& startupError, bool restartAllowed)
{
	startupError = false;
	std::unordered_map< std::string, int > modesForProcessNames;
	TransactionList::iterator it = begin();
	while (it != end() && (!startupError || !restartAllowed))
	{
		TransactionList::iterator itRun = it;
		TransactionBase& transaction = **itRun;
		it++;

		if (isTransactionAllowed(transaction, modesForProcessNames))
		{
			bool startupErrorLocal;
			bool cont = transaction.executeIntern(report, startupErrorLocal);
			if (!startupError)
			{
				startupError = startupErrorLocal;
			}
			if (!cont)
			{
				erase(itRun);
			}
		}
	}
}

