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


#include "platformspecific.h"

#include "processes.h"
#include "watchprocesses.h"
#include "transactionbase.h"
#include "transactionstart.h"
#include "transactionstop.h"
#include "finalmq/interfaces/fmqprocess.fmq.h"

#include "finalmq/remoteentity/RemoteEntity.h"
#include "finalmq/helpers/PersistFile.h"
#include "finalmq/helpers/File.h"
#include "finalmq/serializejson/ParserJson.h"
#include "finalmq/serializejson/SerializerJson.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"
#include "finalmq/remoteentity/EntityFileService.h"



#include "defines.h"

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>
#include <iostream>


#define	TIMEINTERVAL_WATCHDOG	20			// 20s
#define	report_LIMIT			1000		// 1000 entries
#define SHUTDOWN_FILESIZE		(8*512)		// max 512 timestamps
#define	RECOVERSUPRESS_INTERVAL	(60 * 5)	// [s] 5 minutes
#define	RECOVERSUPRESS_MAXNUM	10000		// very high number to switch off supression


#define MODULENAME	"ProcessServer"

using namespace finalmq;
using namespace finalmq::fmqprocess;


class ProcessServer : public RemoteEntity
					, public IProcessServer
{
public:
	ProcessServer()
		: m_exitAtReboot(false)
        , m_rebootAtStartupError(true)
        , m_recover(true)
        , m_reportChanged(false)
		, m_supressRecover(false)
		, m_enableShutdownTimeMonitor(false)
		, m_intervalShutdownTime(60)
		, m_startTimeRecoverSupress(0)
		, m_timeLastShutdownTime(0)
		, m_timeShutdown(0)
		, m_timeWatchDog(0)
		, m_watchProcesses(self())
	{
		// register peer events to see when a remote entity connects or disconnects.
		registerPeerEvent([this](PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
			streamInfo << "peer event " << peerEvent.toString();

			if (peerEvent == PeerEvent::PEER_CONNECTED)
			{
				ProcessStatusChangedRequest request;
				getAllProcesses(request.status);
				sendEvent(peerId, "processstatuschanged", request);
			}
		});

		registerCommand<NoData>("processes", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			// send reply
			AllProcessesReply replyData;
			getAllProcesses(replyData.processes);
			requestContext->reply(std::move(replyData));
		});

		registerCommand<InstallRequest>("install", [this](const RequestContextPtr& requestContext, const std::shared_ptr<InstallRequest>& request) {
			install(request->process);
		});

		registerCommand<UninstallRequest>("uninstall", [this](const RequestContextPtr& requestContext, const std::shared_ptr<UninstallRequest>& request) {
			Status status = uninstall(request->id);
			requestContext->reply(status);
		});

		registerCommand<StartRequest>("start", [this](const RequestContextPtr& requestContext, const std::shared_ptr<StartRequest>& request) {
			start(request->id, requestContext);
		});

		registerCommand<StopRequest>("stop", [this](const RequestContextPtr& requestContext, const std::shared_ptr<StopRequest>& request) {
			stop(request->id, requestContext);
		});

		registerCommand<NoData>("report", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			ReportReply replyData;
			getReport(replyData.report);
			requestContext->reply(std::move(replyData));
		});

		registerCommand<NoData>("clearreport", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			clearReport();
		});

		registerCommand<NoData>("startshutdowntimemonitor", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			startShutdownTimeMonitor();
		});

		registerCommand<NoData>("shutdowntime", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			ShutdownTimeReply replyData;
			replyData.timeShutdown = getShutdownTime();
			requestContext->reply(std::move(replyData));
		});
	}
	virtual ~ProcessServer()
	{
	}
	void init(const std::string& persistFile, const std::string& persistreport, const std::string& fileForStoringShutdownTime, int intervalShutdownTime, bool exitAtReboot)
	{
		m_fileForStoringShutdownTime = fileForStoringShutdownTime;
		m_intervalShutdownTime = intervalShutdownTime;
		m_exitAtReboot = exitAtReboot;
		m_persistFile = persistFile;
		m_persistreport = persistreport;
		m_report.report.reserve(report_LIMIT + 200);
		loadShutdownTime();
		loadPersistency();
		loadReport();
		m_thread = std::thread([this]() {
			run();
		});
		m_watchStartedProcesses.start();
		m_watchProcesses.start(m_data.pollEntityTimeinterval, m_data.pollEntityTimeReaction);
	}

private:

	ProcessServer& self()
	{
		return *this;
	}

	void loadShutdownTime()
	{
		File file;
		int res = file.openForRead(m_fileForStoringShutdownTime.c_str());
		if (res >= 0)
		{
			int size = file.getFileSize();
			bool errFile = true;
			if (size > 0 && (size % sizeof(m_timeShutdown)) == 0)
			{
				res = file.seek(size - sizeof(m_timeShutdown));
				if (res == (size - (int)sizeof(m_timeShutdown)))
				{
					time_t tim = -1;
					int res = file.read(reinterpret_cast<char*>(&tim), sizeof(tim));
					if (res == sizeof(tim) && tim > 0)
					{
						errFile = false;
						m_timeShutdown = tim;
					}
				}
			}
			if (errFile)
			{
				file.unlink();
			}
			file.close();
		}
	}

	void loadPersistency()
	{
		PersistFile file(m_persistFile.c_str());
		std::vector<char> buffer;
		bool ok = file.read(buffer);
		if (ok)
		{
			// deserialize
			SerializerStruct serializer(m_data);
			ParserJson parser(serializer, buffer.data(), buffer.size());
			parser.parseStruct(m_data.getStructInfo().getTypeName());
            m_rebootAtStartupError = m_data.rebootAtStartupError;
            m_recover = m_data.recover;
            for (size_t i=0; i<m_data.config.size(); i++)
			{
				m_processes.addProcess(m_data.config[i]);
			}
			std::vector<std::string> processNamesToStart;
			m_processes.getStartupProcessNames(processNamesToStart);
			streamInfo << "number of processes: " << processNamesToStart.size();
			for (size_t i=0; i<processNamesToStart.size(); i++)
			{
				streamInfo << "start process: " << processNamesToStart[i];
				startProcessIntern(processNamesToStart[i], true);
			}
		}
	}

	void savePersistency()
	{
		ZeroCopyBuffer buffer;
		SerializerJson serializer(buffer, 512, true, false);
		ParserStruct parser(serializer, m_data);
		parser.parseStruct();
		std::string buf = buffer.getData();

		PersistFile file(m_persistFile.c_str());
		file.write(std::vector<char>(buf.c_str(), buf.c_str() + buf.size()), false);
	}

	void loadReport()
	{
		PersistFile file(m_persistreport.c_str());
		std::vector<char> buffer;
		bool ok = file.read(buffer);
		if (ok)
		{
			// deserialize
			SerializerStruct serializer(m_report);
			ParserJson parser(serializer, buffer.data(), buffer.size());
			parser.parseStruct(m_report.getStructInfo().getTypeName());
		}
	}

	void saveReport()
	{
		if (m_reportChanged)
		{
			ZeroCopyBuffer buffer;
			SerializerJson serializer(buffer, 512, true, false);
			ParserStruct parser(serializer, m_report);
			parser.parseStruct();
			std::string buf = buffer.getData();

			PersistFile file(m_persistreport.c_str());
			file.write(std::vector<char>(buf.c_str(), buf.c_str() + buf.size()));
			m_reportChanged = false;
		}
	}

	void startProcessIntern(const std::string& id, bool startup = false)
	{
		new TransactionStart(id, nullptr, m_processes, m_transactions, m_watchStartedProcesses, startup);
	}

	void stopProcessIntern(const std::string& id)
	{
		new TransactionStop(id, nullptr, m_processes, m_transactions);
	}

	// IProcessServer
	void getAllProcesses(std::vector<ProcessStatus>& processes ) const
	{
		std::unique_lock<std::mutex> lock(m_mtx);
        std::vector<ProcessStatus> processesAll = m_processes.getProcesses();
        for (size_t i=0; i<processesAll.size(); i++)
        {
            const ProcessStatus& process = processesAll[i];
            int ix = getProcessConfig(process.config.id);
            if (ix >= 0)
            {
                processes.push_back(process);
            }
        }
		lock.unlock();
	}

	void install(const ProcessConfig& process)
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		int ix = getProcessConfig(process.id);
		if (ix >= 0)
		{
			m_data.config[ix] = process;
		}
		else
		{
			m_data.config.push_back(process);
		}
		savePersistency();
		m_processes.addProcess(process);
		lock.unlock();
	}

	Status uninstall(const std::string& id)
	{
		Status status = Status::STATUS_OK;
		std::unique_lock<std::mutex> lock(m_mtx);
		ProcessState state = m_processes.getProcessState(id);
		if (state != ProcessState::STATE_ON &&
			state != ProcessState::STATE_STARTING)
		{
			int ix = getProcessConfig(id);
			if (ix >= 0)
			{
				m_data.config.erase(m_data.config.begin()+ix);
			}
			savePersistency();
			m_processes.removeProcess(id);
		}
		else
		{
			status = Status::STATUS_REQUEST_PROCESSING_ERROR;
		}
		lock.unlock();
		return status;
	}

	void start(const std::string& id, const RequestContextPtr& requestContext)
	{
		Status status = Status::STATUS_OK;
		std::unique_lock<std::mutex> lock(m_mtx);
		int ix = getProcessConfig(id);
        if (ix >= 0)
        {
            new TransactionStart(id, requestContext, m_processes, m_transactions, m_watchStartedProcesses, false);
        }
        else
        {
			status = Status::STATUS_REQUEST_PROCESSING_ERROR;
        }
        lock.unlock();
		if (status != Status::STATUS_OK)
		{
			requestContext->reply(status);
		}
	}

	void stop(const std::string& id, const RequestContextPtr& requestContext)
	{
		Status status = Status::STATUS_OK;
		std::unique_lock<std::mutex> lock(m_mtx);
		ProcessStatus* processStatus = m_processes.getProcess(id);
		if (processStatus)
		{
			new TransactionStop(id, requestContext, m_processes, m_transactions);
		}
		else
		{
			status = Status::STATUS_REQUEST_PROCESSING_ERROR;
		}
		lock.unlock();
		if (status != Status::STATUS_OK)
		{
			requestContext->reply(status);
		}
	}

	void getReport(std::vector<ReportEntry>& report) const
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		report = m_report.report;
		lock.unlock();
	}

	void clearReport()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_report.report.clear();
        m_reportChanged = true;
		saveReport();
		lock.unlock();
	}

	void startShutdownTimeMonitor()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		if (!m_enableShutdownTimeMonitor)
		{
			// if report has timestamps in future -> set timestamps to current time.
			time_t timCurrent = time(nullptr);
			std::vector<ReportEntry>& report = m_report.report;
			std::vector<ReportEntry>::reverse_iterator it = report.rbegin();
			while (it != report.rend())
			{
				std::vector<ReportEntry>::reverse_iterator itRun = it;
				it++;
				if (itRun->timestamp > timCurrent)
				{
					itRun->timestamp = timCurrent;
					
					// next line if remove of entry is desired.
					//report.erase(it.base());
				}
			}
			m_enableShutdownTimeMonitor = true;
		}
		lock.unlock();
	}

	time_t getShutdownTime() const
	{
		return m_timeShutdown;
	}

	// CThread
	void run()
	{
		bool recoverAllowed = isRecoverAllowed();
		int cnt = 0;
		while (!m_terminate.wait(500))
		{
			cnt++;
			std::vector<int> pidsEnded = m_watchStartedProcesses.getEndedPids();
			std::vector<WatchProcesses::FailedProcess> pidsFailed = m_watchProcesses.getFailedPids();
//			std::vector<ReportEntry> reportWatch;
//			m_watchProcesses.getreport(reportWatch);

			std::unique_lock<std::mutex> lock(m_mtx);
			//			putReport(reportWatch);
			bool startupError;
            std::vector<ReportProcessStatus> report;
			m_transactions.execute(report, startupError, recoverAllowed);
			putReport(report);
			refreshEndedProcesses(pidsEnded, startupError, recoverAllowed);
			refreshFailedProcesses(pidsFailed, recoverAllowed);
			if (!m_supressRecover)
			{
				if (!m_watchProcesses.loopOk())
				{
					putReport(MODULENAME, ReportId::REPORTID_WATCHTHREADBLOCKED);
					recoverProcess(ProcessStatus(), RecoverMode::RECOVERMODE_RESTART_SYSTEM, recoverAllowed);
				}
                if (m_rebootAtStartupError)
                {
                    if (startupError)
                    {
                        recoverProcess(ProcessStatus(), RecoverMode::RECOVERMODE_RESTART_SYSTEM, recoverAllowed);
                    }
                    ProcessStatus* processStartTimeout = m_processes.isStartTimeout();
                    if (processStartTimeout)
                    {
						putReport(*processStartTimeout, ReportId::REPORTID_STARTTIMEOUT);
                        recoverProcess(ProcessStatus(), RecoverMode::RECOVERMODE_RESTART_SYSTEM, recoverAllowed);
                    }
                }
			}
			refreshWatchDog();
			if (cnt % 10 == 0)
			{
				// saves only if report has changed
				saveReport();
				shutdownTimeStoring();
				recoverAllowed = isRecoverAllowed();
				// if m_supressRecover -> reset after RECOVERSUPRESS_INTERVAL
				if (m_supressRecover)
				{
					if (Helper::getClockSecond() > m_startTimeRecoverSupress + RECOVERSUPRESS_INTERVAL)
					{
						m_supressRecover = false;
						m_startTimeRecoverSupress = 0;
						recoverProcess(ProcessStatus(), RecoverMode::RECOVERMODE_RESTART_SYSTEM, true);
					}
				}
			}
			lock.unlock();
		}
	}

	// shutdown time recording
	void shutdownTimeStoring()
	{
		if (m_enableShutdownTimeMonitor)
		{
			time_t timCurrent = time(nullptr);
			if (timCurrent >= m_timeLastShutdownTime + m_intervalShutdownTime || timCurrent < m_timeLastShutdownTime)
			{
				m_timeLastShutdownTime = timCurrent;
				saveShutdownTime();
			}
		}
	}

	void saveShutdownTime()
	{
		time_t timCurrent = time(nullptr);
		if (File::getFileSize(m_fileForStoringShutdownTime.c_str()) >= SHUTDOWN_FILESIZE)
		{
			File::truncate(m_fileForStoringShutdownTime.c_str(), 0);
		}
		File file;
		int res = file.openForWrite(m_fileForStoringShutdownTime.c_str());
		if (res >= 0)
		{
			file.write(reinterpret_cast<char*>(&timCurrent), sizeof(timCurrent));
			file.sync();
			file.close();
		}
	}

    int getProcessConfig(const std::string& id) const
	{
		for (int i=0; i<static_cast<int>(m_data.config.size()); i++)
		{
			if (m_data.config[i].id == id)
			{
				return i;
			}
		}
		return -1;
	}

	void refreshWatchDog()
	{
		time_t timCurrent = time(nullptr);
		if (timCurrent < m_timeWatchDog ||
			timCurrent > m_timeWatchDog + TIMEINTERVAL_WATCHDOG)
		{
			m_timeWatchDog = timCurrent;
			// if supressRecover -> keep watchdog alive
			if (m_processes.isSystemOk() || m_supressRecover)
			{
				fireWatchdog();
			}
		}
	}

	void refreshEndedProcesses(const std::vector<int>& pidsEnded, bool& startupError, bool recoverAllowed)
	{
		for (size_t i=0; i<pidsEnded.size(); i++)
		{
			int pid = pidsEnded[i];
			ProcessStatus* process = m_processes.getProcess(pid);
			if (process)
			{
				streamInfo << "process ended " << process->config.id << " " << process->pid << " state: " << process->state.toString();
				// reregister ProcessServer
				if (process->state == ProcessState::STATE_STOPPING)
				{
					process->state = ProcessState::STATE_OFF;
                    process->timeDone = time(nullptr);
                    putReport(*process, ReportId::REPORTID_STATECHANGE);
                    process->pid = 0;
                }
				else if (process->state == ProcessState::STATE_ON)
				{
					process->state = ProcessState::STATE_ENDED;
                    putReport(*process, ReportId::REPORTID_STATECHANGE);
                    process->pid = 0;
                    recoverProcess(*process, process->config.recoverMode, recoverAllowed);
				}
				else if (process->state == ProcessState::STATE_STARTING)
				{
					process->state = ProcessState::STATE_ERROR_START;
                    putReport(*process, ReportId::REPORTID_STATECHANGE);
                    process->pid = 0;
                    if (process->startup)
					{
						startupError = true;
					}
					else
					{
						recoverProcess(*process, process->config.recoverMode, recoverAllowed);
					}
				}
                else
                {
                    streamWarning << "process " << process->config.id << " with state: " << process->state.toString() << " stopped.";
                    process->pid = 0;
                }
			}
		}
	}

	void refreshFailedProcesses(std::vector<WatchProcesses::FailedProcess>& pidsFailed, bool recoverAllowed)
	{
		for (size_t i=0; i<pidsFailed.size(); i++)
		{
			int pid = pidsFailed[i].m_pid;
			ProcessStatus* process = m_processes.getProcess(pid);
			if (process)
			{
				if (process->state == ProcessState::STATE_ON)
				{
					RecoverMode recoverMode = pidsFailed[i].m_recoverMode;
					// correct recover mode
					if (process->config.recoverMode > recoverMode)
					{
						recoverMode = process->config.recoverMode;
					}
                    process->errorReason = pidsFailed[i].m_errorReason;
                    streamInfo << "process failed " << process->config.id << " " << process->pid << "state: " << process->state.toString() << " error: " << process->errorReason;
                    process->state = ProcessState::STATE_ERROR;
                    putReport(*process, ReportId::REPORTID_FAIL, recoverMode);
					recoverProcess(*process, recoverMode, recoverAllowed);
				}
			}
		}
	}


	int getNumOfRecovers(int lastSeconds)
	{
		int cntRecover = 0;
		time_t timCurrent = time(nullptr);
		const std::vector<ReportEntry>& report = m_report.report;
		std::vector<ReportEntry>::const_reverse_iterator it = report.rbegin();
		bool ex = false;
		for ( ; it != report.rend() && !ex; it++)
		{
			if (it->timestamp > timCurrent - lastSeconds)
			{
				if (it->id == ReportId::REPORTID_RECOVER)
				{
					cntRecover++;
				}
			}
			else
			{
				ex = false;
			}
		}
		return cntRecover;
	}

	bool isRecoverAllowed()
	{
		int numOfRecovers = getNumOfRecovers(RECOVERSUPRESS_INTERVAL);
		return (numOfRecovers < RECOVERSUPRESS_MAXNUM);
	}

	void recoverProcess(const ProcessStatus& process, RecoverMode recoverMode, bool recoverAllowed)
	{
		if (recoverMode != RecoverMode::RECOVERMODE_NONE)
		{
            if (m_recover)
            {
                if (!m_supressRecover && recoverAllowed)
                {
                    putReport((!process.config.id.empty()) ? process.config.id : MODULENAME, ReportId::REPORTID_RECOVER, process.state, process.errorReason, process.pid, recoverMode);
                    switch (recoverMode)
                    {
                    case RecoverMode::RECOVERMODE_NONE:
                        break;
                    case RecoverMode::RECOVERMODE_RESTART_PROCESS:
						startProcessIntern(process.config.id);
                        break;
                    case RecoverMode::RECOVERMODE_RESTART_DEPENDANTS:
                        {
							std::vector<std::string> processNames;
                            m_processes.getStartedDependantProcessNames(process.config.id, processNames);
                            stopProcessIntern(process.config.id);
                            for (size_t i=0; i<processNames.size(); i++)
                            {
                                startProcessIntern(processNames[i]);
                            }
                        }
                        break;
                    case RecoverMode::RECOVERMODE_RESTART_ALL:
                        {
							std::vector<std::string> processNamesToRestart;
                            m_processes.getStartedProcessNames(processNamesToRestart);
                            for (size_t i=0; i<processNamesToRestart.size(); i++)
                            {
                                stopProcessIntern(processNamesToRestart[i]);
                            }
                            for (size_t i=0; i<processNamesToRestart.size(); i++)
                            {
                                startProcessIntern(processNamesToRestart[i]);
                            }
                        }
                        break;
                    case RecoverMode::RECOVERMODE_RESTART_SYSTEM:
                    default:
                        {
                            reboot();
                        }
                        break;
                    }
                }
                else
                {
                    if (!m_supressRecover)
                    {
                        m_supressRecover = true;
                        m_startTimeRecoverSupress = Helper::getClockSecond();
                        putReport(process, ReportId::REPORTID_SUPRESSRECOVER, recoverMode);
                        streamInfo << "supress recover " << process.config.id << " " << process.pid << " state: " << process.state.toString();
                    }
                }
            }
            else
            {
				streamInfo << "no recover of " << process.config.id << " " << process.pid << " state: " << process.state.toString();
            }
		}
	}

	void reboot()
	{
		//std::vector<std::string> processNamesToRestart;
		//m_processes.getStartedProcessNames(processNamesToRestart);
		//for (size_t i=0; i<processNamesToRestart.size(); i++)
		//{
		//	stopProcessIntern(processNamesToRestart[i]);
		//}
		putReport(MODULENAME, ReportId::REPORTID_REBOOT);
		saveReport();
		int res = system("sync");
		if (res != 0)
		{
			streamError << "sync failed";
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));	// give other processes little time to react on recover
		saveShutdownTime();
		// watchdog to keep reboot alive
		fireWatchdog();
		if (!m_exitAtReboot)
		{
			res = system("reboot");
			if (res != 0)
			{
				streamError << "reboot failed";
			}
		}
		exit(-1);
	}

    static bool isStateChange(ReportId id)
    {
        bool ret = false;
        if (id == ReportId::REPORTID_FAIL ||
            id == ReportId::REPORTID_STATECHANGE)
        {
            ret = true;
        }
        return ret;
    }

    void putReport(const std::string& processName, ReportId id)
    {
		putReport(processName, id, ProcessState::STATE_NONE, "", 0);
    }

    void putReport(const ProcessStatus& process, ReportId id)
    {
		ReportProcessStatus entry;
        entry.timestamp                     = time(nullptr);
        entry.process                       = process;
        entry.id                            = id;
		putReport(entry);
    }

    void putReport(const ProcessStatus& process, ReportId id, RecoverMode recoverMode)
    {
		ReportProcessStatus entry;
        entry.timestamp                     = time(nullptr);
        entry.process                       = process;
        entry.id                            = id;
        entry.process.config.recoverMode	= recoverMode;
		putReport(entry);
    }

    void putReport(const std::string& processName, ReportId id, ProcessState processState, const std::string& errorReason, int pid, RecoverMode recoverMode = RecoverMode::RECOVERMODE_NONE)
    {
        ReportProcessStatus entry;
        entry.timestamp                     = time(nullptr);
        entry.process.config.id             = processName;
        entry.id                            = id;
        entry.process.pid                   = pid;
        entry.process.config.recoverMode	= recoverMode;
        entry.process.state                 = processState;
        entry.process.errorReason           = errorReason;
        putReport(entry);
    }

    void processStatusToReport(const ReportProcessStatus& ReportProcessStatus, ReportEntry& reportEntry)
    {
		reportEntry.timestamp		= ReportProcessStatus.timestamp;
		reportEntry.processName	= ReportProcessStatus.process.config.id;
		reportEntry.id			= ReportProcessStatus.id;
		reportEntry.pid			= ReportProcessStatus.process.pid;
		reportEntry.processState	= ReportProcessStatus.process.state;
		reportEntry.errorReason	= ReportProcessStatus.process.errorReason;
		reportEntry.recoverMode	= ReportProcessStatus.process.config.recoverMode;
    }

    void putReport(const ReportProcessStatus& entry, RecoverMode recoverMode)
    {
		std::unique_lock<std::mutex> lock(m_mtxReport);
		m_report.report.resize(m_report.report.size() + 1);
		processStatusToReport(entry, m_report.report.back());
        m_report.report.back().recoverMode = recoverMode;
		keepReportSmall();
        m_reportChanged = true;
		std::vector<ReportEntry> protEntries;
		protEntries.push_back(m_report.report.back());
		fireProcessReportChanged(protEntries);
		if (isStateChange(entry.id))
        {
			std::vector<ProcessStatus> processes;
			processes.push_back(entry.process);
			fireProcessStatusChanged(processes);
		}
        lock.unlock();
    }
    void putReport(const ReportProcessStatus& entry)
    {
		std::unique_lock<std::mutex> lock(m_mtxReport);
		m_report.report.resize(m_report.report.size() + 1);
        processStatusToReport(entry, m_report.report.back());
		keepReportSmall();
        m_reportChanged = true;
		std::vector<ReportEntry> protEntries;
		protEntries.push_back(m_report.report.back());
		fireProcessReportChanged(protEntries);
		if (isStateChange(entry.id))
        {
			std::vector<ProcessStatus> processes;
			processes.push_back(entry.process);
			fireProcessStatusChanged(processes);
		}
        lock.unlock();
    }
    void putReport(const std::vector<ReportProcessStatus>& entries)
	{
		if (!entries.empty())
		{
			std::unique_lock<std::mutex> lock(m_mtxReport);
			std::vector<ReportEntry> protEntries;
			protEntries.resize(entries.size());
			for (size_t i=0; i<entries.size(); i++)
            {
				processStatusToReport(entries[i], protEntries[i]);
            }
            m_report.report.insert(m_report.report.end(), protEntries.begin(), protEntries.end());
			keepReportSmall();
			m_reportChanged = true;
            std::vector<ProcessStatus> processes;
            for (size_t i=0; i<entries.size(); i++)
            {
                if (isStateChange(entries[i].id))
                {
                    processes.push_back(entries[i].process);
                }
            }
			fireProcessReportChanged(protEntries);
            if (!processes.empty())
            {
				fireProcessStatusChanged(processes);
            }
            lock.unlock();
        }
	}

	void fireProcessReportChanged(const std::vector<ReportEntry>& entries)
	{
		ProcessReportChangedRequest request;
		request.entries = entries;
		sendEventToAllPeers("processstatuschanged", request);
	}

	void fireProcessStatusChanged(const std::vector<ProcessStatus>& status)
	{
		ProcessStatusChangedRequest request;
		request.status = status;
		sendEventToAllPeers("processstatuschanged", request);
	}

	void fireWatchdog()
	{
		WatchdogRequest request;
		request.ok = true;
		sendEventToAllPeers("watchdog", request);
	}

	void keepReportSmall()
	{
		if (m_report.report.size() > report_LIMIT)
		{
			std::vector<ReportEntry> tmp;
			tmp.insert(tmp.end(), m_report.report.begin() + m_report.report.size() - (report_LIMIT/4), m_report.report.end());
			m_report.report = tmp;
		}
	}

    bool								m_exitAtReboot;
    bool								m_rebootAtStartupError;
    bool								m_recover;
    bool								m_reportChanged;
	bool								m_supressRecover;
	bool								m_enableShutdownTimeMonitor;
	int									m_intervalShutdownTime;
	time_t								m_startTimeRecoverSupress;	// monotonic clock
	std::string							m_fileForStoringShutdownTime;
	time_t								m_timeLastShutdownTime;
	time_t								m_timeShutdown;
	time_t								m_timeWatchDog;
	std::string							m_persistFile;
	std::string							m_persistreport;
	PersistData							m_data;
	PersistReport						m_report;
	Processes							m_processes;
	TransactionList						m_transactions;
	WatchStartedProcessesForTermination	m_watchStartedProcesses;
	WatchProcesses						m_watchProcesses;
	std::thread							m_thread;
	mutable std::mutex					m_mtx;
	mutable std::mutex					m_mtxReport;
	CondVar								m_terminate;
};






int main(int argc, char* argv[])
{
	// display log traces
	Logger::instance().registerConsumer([](const LogContext& context, const char* text) {
		std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
	});

	std::string persistFile = "./processServer.cfg";
	std::string persistReport = "./processreport.cfg";
#ifdef WIN32
	std::string fileForStoringShutdownTime = "./file_for_shutdown_time";
#else
	std::string fileForStoringShutdownTime = "/data/file_for_shutdown_time";
#endif
	int intervalShutdownTime = 60;
	bool exitAtReboot = false;

	if (argc >= 2)
	{
		persistFile = argv[1];
	}
	if (argc >= 3)
	{
		persistReport = argv[2];
	}
	if (argc >= 4)
	{
		fileForStoringShutdownTime = argv[3];
	}
	if (argc >= 5)
	{
		intervalShutdownTime = atoi(argv[4]);
		if (intervalShutdownTime == 0)
		{
			intervalShutdownTime = 60;
		}
	}
	if (argc >= 6)
	{
		exitAtReboot = atoi(argv[5]) != 0 ? true : false;
	}

	{
		RemoteEntityContainer entityContainer;
		entityContainer.init();

		// register lambda for connection events to see when a network node connects or disconnects.
		entityContainer.registerConnectionEvent([](const SessionInfo& session, ConnectionEvent connectionEvent) {
			const ConnectionData connectionData = session.getConnectionData();
			streamInfo << "connection event at " << connectionData.endpoint
				<< " remote: " << connectionData.endpointPeer
				<< " event: " << connectionEvent.toString();
		});

		ProcessServer processServer;
		processServer.init(persistFile, persistReport, fileForStoringShutdownTime, intervalShutdownTime, exitAtReboot);
		entityContainer.registerEntity(&processServer, "ProcessServer");

		EntityFileServer entityFileServer("htdocs");
		entityContainer.registerEntity(&entityFileServer, "*");

		entityContainer.bind("tcp://*:9100:headersize:protobuf");
		entityContainer.bind("tcp://*:9101:delimiter_lf:json");
		entityContainer.bind("tcp://*:9180:httpserver:json");

		entityContainer.run();
	}

	return 0;
}
