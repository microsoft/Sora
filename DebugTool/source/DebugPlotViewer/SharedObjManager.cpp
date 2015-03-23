#include "StdAfx.h"
#include "AppSettings.h"
#include "SharedObjManager.h"
#include "SeriesObj.h"
#include "SeriesLine.h"
#include "SeriesText.h"
#include "SeriesDots.h"
#include "SeriesLog.h"
#include "SeriesSpectrum.h"
#include "SharedStruct.h"
#include "Logger.h"

SharedObjManager * SharedObjManager::instance = 0;

SharedObjManager * SharedObjManager::Instance()
{
	if (!SharedObjManager::instance)
	{
		SharedObjManager::instance = new SharedObjManager();
	}

	return SharedObjManager::instance;
}

SharedObjManager::SharedObjManager(void) : lastUpdateIndex(0)
{
	::InitializeCriticalSection(&cs);
}

SharedObjManager::~SharedObjManager(void)
{
	LockAndClear();

	::DeleteCriticalSection(&cs);
}

void SharedObjManager::Clean()
{
	if (SharedObjManager::instance)
	{
		delete SharedObjManager::instance;
		SharedObjManager::instance = 0;
	}
}

// update node structure
void SharedObjManager::RemoveDeadProcess()
{
	SharedGlobalData * sharedGlobalData = SharedGlobalData::Instance();
	sharedGlobalData->Lock(INFINITE);
	sharedGlobalData->GarbageCollect();
	sharedGlobalData->Unlock();
}

// update local object
void SharedObjManager::MarkAllObjsCheck()
{
	std::map<int, DebuggingProcessProp *>::iterator iterProcess;
	for (iterProcess = this->allActiveProcessObjs.begin();
		iterProcess != this->allActiveProcessObjs.end();
		iterProcess++)
	{
		DebuggingProcessProp * prop = (DebuggingProcessProp *)(*iterProcess).second;
		prop->alive = false;
	}

	std::map<int, SeriesProp *>::iterator iterSeries;
	for (iterSeries = this->allActiveSeriesObjs.begin();
		iterSeries != this->allActiveSeriesObjs.end();
		iterSeries++)
	{
		(*iterSeries).second->alive = false;
	}
}

void SharedObjManager::RemoveClosedObj()
{
	std::map<int, DebuggingProcessProp *>::iterator iterActiveProcess;
	for (iterActiveProcess = this->allActiveProcessObjs.begin();
		iterActiveProcess != this->allActiveProcessObjs.end();)
	{
		DebuggingProcessProp * processProp = (*iterActiveProcess).second;
		if (processProp->alive == false)
		{
			processProp->isActive = false;
			processProp->pid = 0;
			processProp->FreeSharedMem();
			this->allActiveProcessObjs.erase(iterActiveProcess++);
			processProp->DecRefCount();
		}
		else
			iterActiveProcess++;
	}

	std::map<int, SeriesProp *>::iterator iterActiveSeries;
	for (iterActiveSeries = this->allActiveSeriesObjs.begin();
		iterActiveSeries != this->allActiveSeriesObjs.end();)
	{
		SeriesProp * seriesProp = (*iterActiveSeries).second;
		if (seriesProp->alive == false)
		{
			seriesProp->isActive = false;
			if (seriesProp->SerialNum() > 0)
			{
				this->seriesMapForReading.erase(
					this->seriesMapForReading.find(
					seriesProp->SerialNum()
					)
					);
			}
			seriesProp->FreeSharedMem();
			this->allActiveSeriesObjs.erase(iterActiveSeries++);

			seriesProp->DecRefCount();
		}
		else
			iterActiveSeries++;
	}

	std::multimap<wstring, SeriesProp *>::iterator iterSeries;
	for (iterSeries = this->allSeriesObjs.begin();
		iterSeries != this->allSeriesObjs.end();)
	{
		SeriesProp * seriesProp = (*iterSeries).second;
		if (seriesProp->GetRefCount() == 1)
		{
			seriesProp->EraseFromProcess();
			seriesProp->DecRefCount();

			this->allSeriesObjs.erase(iterSeries++);

			this->shouldUpdate = true;
		}
		else
			iterSeries++;
	}

	std::multimap<wstring, DebuggingProcessProp *>::iterator iterProcess;
	for (iterProcess = this->allProcessObjs.begin();
		iterProcess != this->allProcessObjs.end();)
	{
		DebuggingProcessProp * processProp = (*iterProcess).second;
		if (processProp->GetRefCount() == 1)
		{
			processProp->DecRefCount();
			this->allProcessObjs.erase(iterProcess++);
			this->shouldUpdate = true;
		}
		else
			iterProcess++;
	}
}

void SharedObjManager::AddNewObjs()
{
	SharedGlobalData * sharedGlobalData = SharedGlobalData::Instance();
	if (!sharedGlobalData)
		return;

	if (sharedGlobalData->sharedStruct->updateIndex == this->lastUpdateIndex)
	{
		this->shouldUpdate = false;
		return;
	}
	else
		this->lastUpdateIndex = sharedGlobalData->sharedStruct->updateIndex;

	this->shouldUpdate = true;

	MarkAllObjsCheck();

	sharedGlobalData->Lock(INFINITE);

	// scan processes
	for (int iProcess = 1; iProcess != MAX_PROCESS; iProcess++)
	{
		ProcessNode * processNode = sharedGlobalData->sharedStruct->processNodes + iProcess;
		if (processNode->used)		// find a process
		{
			int key = processNode->pid;

			std::map<int, DebuggingProcessProp *>::iterator iterProcess;
			iterProcess = allActiveProcessObjs.find(key);

			if (iterProcess != allActiveProcessObjs.end())	// find it, unset the check flag and free opened shared memory
			{
				DebuggingProcessProp * debuggingProcessProp = (DebuggingProcessProp *)(*iterProcess).second;
				debuggingProcessProp->alive = true;
			}
			else								// doesn't find it, create new one
			{
				ShareMem * smProcess = sharedGlobalData->GetProcessInfo(processNode);
				SharedProcessInfo * sharedProcessInfo = (SharedProcessInfo *)smProcess->GetAddress();

				DebuggingProcessProp * debuggingProcessProp = MatchProcess(sharedProcessInfo);
				if (debuggingProcessProp)
				{
					;
				}
				else
				{
					debuggingProcessProp = new DebuggingProcessProp;
					debuggingProcessProp->moduleName.SetString(sharedProcessInfo->name);

					std::wstring processName;
					processName.append(sharedProcessInfo->name);
					
					debuggingProcessProp->IncRefCount();
					this->allProcessObjs.insert(std::pair<wstring, DebuggingProcessProp *>(processName,debuggingProcessProp));
				}

				//sharedProcessInfo->threshold = debuggingProcessProp->threshold;

				debuggingProcessProp->alive = true;
				debuggingProcessProp->pid = sharedProcessInfo->pid;
				debuggingProcessProp->smProcess = smProcess;
				debuggingProcessProp->eventViewerPlotterSync = sharedGlobalData->NewProcessEvent(sharedProcessInfo);
				debuggingProcessProp->smSourceInfo = sharedGlobalData->NewSourceInfo(processNode);
				debuggingProcessProp->smSourceData = sharedGlobalData->NewSourceData(processNode, ::SettingGetSourceBufferSize());
				debuggingProcessProp->isActive = true;

				debuggingProcessProp->IncRefCount();
				this->allActiveProcessObjs.insert(std::pair<int, DebuggingProcessProp *>(key, debuggingProcessProp));
			}
		}
	}

	//std::map<int, SeriesProp *>::iterator iterSeries;
	for (int iSeries = 1; iSeries != MAX_SERIES; iSeries++)
	{
		SeriesNode * seriesNode = sharedGlobalData->sharedStruct->seriesNodes + iSeries;
		if (!seriesNode->used)
			continue;

		ProcessNode * processNode = sharedGlobalData->sharedStruct->processNodes + seriesNode->processIndex;
		ASSERT(processNode->used);

		int key = processNode->pid * MAX_SERIES + seriesNode->sid;

		std::map<int, SeriesProp *>::iterator iterSeries;

		iterSeries = this->allActiveSeriesObjs.find(key);
		if (iterSeries != allActiveSeriesObjs.end())
		{
			SeriesProp * seriesProp = (SeriesProp *)(*iterSeries).second;
			seriesProp->alive = true;
		}
		else
		{
			ShareMem * smSeries = sharedGlobalData->GetSeriesInfo(seriesNode);
			SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)smSeries->GetAddress();

			//SeriesProp * seriesProp = GetOpendSeries(sharedSeriesInfo);

			SeriesProp * seriesProp = MatchSeries(sharedSeriesInfo);

			bool addNewObj = false;

			if (seriesProp)
			{
				if (sharedSeriesInfo->type == TYPE_TEXT)
				{
					sharedSeriesInfo->replace = ((TextSeriesProp *)seriesProp)->replaceMode;
				}
			}
			else
			{
				switch(sharedSeriesInfo->type)
				{
				case TYPE_TEXT:
					seriesProp = new TextSeriesProp;
					sharedSeriesInfo->replace = ((TextSeriesProp *)seriesProp)->replaceMode;
					break;
				case TYPE_LOG:
					seriesProp = new LogSeriesProp;
					break;
				case TYPE_LINE:
					seriesProp = new LineSeriesProp;
					break;
				case TYPE_DOTS:
					seriesProp = new DotsSeriesProp;
					break;
				case TYPE_SPECTRUM:
					seriesProp = new SpectrumSeriesProp;
					break;
				}
				//ASSERT(seriesProp);	// Bug
				if (!seriesProp)
				{
					this->lastUpdateIndex--;
					continue;
				}

				// check if process id exist
				std::map<int, DebuggingProcessProp *>::iterator iterProcess;
				iterProcess = this->allActiveProcessObjs.find(processNode->pid);
				ASSERT(iterProcess != this->allActiveProcessObjs.end());

				DebuggingProcessProp * processProp = (DebuggingProcessProp *)(*iterProcess).second;
				seriesProp->SetProcess(processProp);
				processProp->AddSeries(seriesProp);

				wstring keyName;
				keyName.append(processProp->moduleName);
				keyName.append(L"|");
				keyName.append(sharedSeriesInfo->name);
				//seriesName.append(sharedSeriesInfo->name);
				seriesProp->IncRefCount();
				this->allSeriesObjs.insert(std::pair<wstring, SeriesProp *>(keyName, seriesProp));
				addNewObj = true;
			}

			seriesProp->alive = true;
			seriesProp->isActive = true;
			seriesProp->name.SetString(sharedSeriesInfo->name);	// TODO
			//seriesProp->type = sharedSeriesInfo->type;			// TODO
			//seriesProp->smSeriesInfo = smSeries;
			seriesProp->SetSmInfo(smSeries);

			//seriesProp->smSeriesData = SharedGlobalData::Instance()->NewSeriesData((SharedSeriesInfo *)smSeries->GetAddress());

			if (seriesProp->SerialNum() > 0)
				this->seriesMapForReading.insert(std::pair<int, SeriesProp *>(seriesProp->SerialNum(), seriesProp));

			seriesProp->IncRefCount();
			this->allActiveSeriesObjs.insert(std::pair<int, SeriesProp *>(key, seriesProp));


		}
	}

	sharedGlobalData->Unlock();
}


void SharedObjManager::LockAndClear()
{
	Lock();
	std::map<int, DebuggingProcessProp *>::iterator iterActiveProcess;
	for (iterActiveProcess = this->allActiveProcessObjs.begin();
		iterActiveProcess != this->allActiveProcessObjs.end();)
	{
		DebuggingProcessProp * processProp = (*iterActiveProcess).second;
		if (1/*processProp->alive == false*/)
		{
			processProp->isActive = false;
			processProp->pid = 0;
			processProp->FreeSharedMem();
			this->allActiveProcessObjs.erase(iterActiveProcess++);
			processProp->DecRefCount();
		}
		else
			iterActiveProcess++;
	}

	std::map<int, SeriesProp *>::iterator iterActiveSeries;
	for (iterActiveSeries = this->allActiveSeriesObjs.begin();
		iterActiveSeries != this->allActiveSeriesObjs.end();)
	{
		SeriesProp * seriesProp = (*iterActiveSeries).second;
		if (1/*seriesProp->alive == false*/)
		{
			seriesProp->isActive = false;
			seriesProp->FreeSharedMem();
			this->allActiveSeriesObjs.erase(iterActiveSeries++);
			seriesProp->DecRefCount();
		}
		else
			iterActiveSeries++;
	}

	std::multimap<wstring, SeriesProp *>::iterator iterSeries;
	for (iterSeries = this->allSeriesObjs.begin();
		iterSeries != this->allSeriesObjs.end();)
	{
		SeriesProp * seriesProp = (*iterSeries).second;
		if (seriesProp->GetRefCount() == 1)
		{
			// TODO
			if (iterSeries->second->SerialNum() > 0)
			{
				this->seriesMapForReading.erase(
					this->seriesMapForReading.find(
						iterSeries->second->SerialNum()
					)
				);
			}

			seriesProp->EraseFromProcess();
			seriesProp->DecRefCount();

			this->allSeriesObjs.erase(iterSeries++);
			this->shouldUpdate = true;
		}
		else
			iterSeries++;
	}

	std::multimap<wstring, DebuggingProcessProp *>::iterator iterProcess;
	for (iterProcess = this->allProcessObjs.begin();
		iterProcess != this->allProcessObjs.end();)
	{
		DebuggingProcessProp * processProp = (*iterProcess).second;
		if (processProp->GetRefCount() == 1)
		{
			processProp->DecRefCount();
			this->allProcessObjs.erase(iterProcess++);
			this->shouldUpdate = true;
		}
		else
			iterProcess++;
	}

	this->shouldUpdate = true;
	this->lastUpdateIndex = 0;

	Unlock();
}

void SharedObjManager::Lock()
{
	::EnterCriticalSection(&cs);
}

void SharedObjManager::Unlock()
{
	::LeaveCriticalSection(&cs);
}

void SharedObjManager::LockAndUpdate()
{
	Lock();
	RemoveDeadProcess();
	AddNewObjs();
	RemoveClosedObj();
	Unlock();
}

bool SharedObjManager::IsChanged()
{
	return this->shouldUpdate;
}

DebuggingProcessProp * SharedObjManager::MatchProcess(SharedProcessInfo * sharedProcessInfo)
{
	wstring key;
	key.append(sharedProcessInfo->name);
	std::pair<std::multimap<wstring, DebuggingProcessProp *>::iterator, std::multimap<wstring, DebuggingProcessProp *>::iterator> range;  
	range = this->allProcessObjs.equal_range(key);
	std::multimap<wstring, DebuggingProcessProp *>::iterator iter;
	for (iter = range.first; iter != range.second; iter++)
	{
		DebuggingProcessProp * process = (*iter).second;
		
		if ( (process->moduleName.Compare(sharedProcessInfo->name) == 0) &&
			( process->isActive == false ) )
		{
			return process;
		}
	}

	return 0;
}

SeriesProp * SharedObjManager::MatchSeries(SharedSeriesInfo * sharedSeriesInfo)
{
	// TODO
	wstring keyName;

	SharedGlobalData * sharedGlobalData = SharedGlobalData::Instance();
	int seriesNodeIdx = sharedSeriesInfo->nodePtr;
	int processNodeIdx = sharedGlobalData->sharedStruct->seriesNodes[seriesNodeIdx].processIndex;
	int pid = sharedGlobalData->sharedStruct->processNodes[processNodeIdx].pid;

	SharedObjManager * sharedObjManager = SharedObjManager::Instance();
	std::map<int, DebuggingProcessProp *>::iterator iterActiveProcess;
	iterActiveProcess = sharedObjManager->allActiveProcessObjs.find(pid);
	ASSERT(iterActiveProcess != sharedObjManager->allActiveProcessObjs.end());
	DebuggingProcessProp * processProp = (*iterActiveProcess).second;

	keyName.append(processProp->moduleName);
	keyName.append(L"|");
	keyName.append(sharedSeriesInfo->name);

	std::pair<std::multimap<wstring, SeriesProp *>::iterator, std::multimap<wstring, SeriesProp *>::iterator> range;
	range = sharedObjManager->allSeriesObjs.equal_range(keyName);

	std::multimap<wstring, SeriesProp *>::iterator iter;
	for (iter = range.first; iter != range.second; iter++)
	{
		SeriesProp * series = (*iter).second;
		if (!series->isActive)
		{
			if (processProp == series->GetProcess())
				return series;
		}
	}

	return 0;
}
