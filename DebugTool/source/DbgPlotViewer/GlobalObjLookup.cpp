#include "stdafx.h"
#include <assert.h>
#include <memory>
#include <Psapi.h>
#include "GlobalObjLookup.h"
#include "SharedChannel.h"
#include "SharedProcess.h"
#include "ChannelOpened.h"
#include "ChannelOpenedText.h"
#include "ChannelOpenedLog.h"
#include "ProcessOpened.h"
#include "TaskSimple.h"
#include "TaskCoordinator.h"
#include "ChannelOpenedLine.h"
#include "ChannelOpenedSpectrum.h"
#include "ChannelOpenedDots.h"
#include "PlotWindowOpened.h"
#include "PlotWindowOpenedLine.h"
#include "PlotWindowOpenedSpectrum.h"
#include "PlotWindowOpenedDots.h"
#include "PlotWindowOpenedText.h"
#include "PlotWindowOpenedLog.h"
#include "ChannelOpenedLine.h"
#include "ChannelOpenedSpectrum.h"
#include "ChannelOpenedDots.h"
#include "ChannelOpenedText.h"
#include "ChannelOpenedLog.h"
#include "HelperFunc.h"

#import <msxml6.dll>

using namespace std;
using namespace SoraDbgPlot::SharedObj;
using namespace SoraDbgPlot::Task;

ObjLookup::ObjLookup()
{
	_pollPauseEventToken = 0;
	_taskQueue = make_shared<TaskQueue>();
}

ObjLookup::~ObjLookup()
{
}

void ObjLookup::AddSharedProcess(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> > & vec)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, vec](){
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			auto sharedProcess = (*iter);
			SThis->AddSharedProcess(sharedProcess);
		}

		SThis->RaiseProcessChannelUpdatedEvent();
	});
}

void ObjLookup::AddSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	auto processOpened = MatchProcess(sharedProcess);
	if (processOpened == 0)
	{
		processOpened = make_shared<ProcessOpened>();
		processOpened->AttatchSharedProcessSync(sharedProcess);
		AddProcess(processOpened);
		EventNewProcessOpened.Raise(this, processOpened);
		EventProcessOpenedAttatched.Raise(this, processOpened);
	}
	else
	{
		processOpened->AttatchSharedProcessSync(sharedProcess);
		EventProcessOpenedAttatched.Raise(this, processOpened);
	}
	
	AttatchShared2Process(sharedProcess, processOpened);
}

void ObjLookup::RemoveSharedProcess(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> > & vec)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, vec](){
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			auto sharedProcess = (*iter);
			SThis->RemoveSharedProcess(sharedProcess);
		}

		SThis->RaiseProcessChannelUpdatedEvent();
	});
}

void ObjLookup::RemoveSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	auto process = GetProcessByShared(sharedProcess);
	UnattatchSharedProcess(sharedProcess);
	process->DeattatchSharedProcessSync();
	EventProcessOpenedDeattatched.Raise(this, process);

	auto iterProcess = _processChannelMap.find(process);
	if(iterProcess != _processChannelMap.end())
	{
		if (iterProcess->second.size() == 0)
			RemoveProcess(process);
	}
}

void ObjLookup::AddSharedChannel(const std::vector<std::shared_ptr<SharedChannel> > & vec)
{
	auto SThis = shared_from_this();

	_taskQueue->QueueTask([SThis, vec](){

		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			auto sharedChannel = *iter;
			SThis->AddSharedChannel(sharedChannel);
		}

		SThis->RaiseProcessChannelUpdatedEvent();
	});
}

void ObjLookup::AddSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel)
{
	shared_ptr<ProcessOpened> processOpened;
	auto channelOpened = this->MatchChannel(sharedChannel, processOpened);

	if (processOpened == 0)
		return;

	if (channelOpened == 0)
	{
		channelOpened = this->MakeChannel(sharedChannel);
		channelOpened->AttatchSharedChannelSync(sharedChannel);	
		this->AddChannel(channelOpened, processOpened);
	}
	else
	{
		channelOpened->AttatchSharedChannelSync(sharedChannel);	
	}

	AttatchShared2Channel(sharedChannel, channelOpened);
}

void ObjLookup::RemoveSharedChannel(const std::vector<std::shared_ptr<SharedChannel> > & vec)
{
	auto SThis = shared_from_this();

	_taskQueue->QueueTask([SThis, vec](){		
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			auto sharedChannel = *iter;
			SThis->RemoveSharedChannel(sharedChannel);
		}

		SThis->RaiseProcessChannelUpdatedEvent();
	});
}

void ObjLookup::RemoveSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel)
{
	auto channelOpened = GetChannelByShared(sharedChannel);
	if (channelOpened == 0)
		return;

	if (!IsOpened(channelOpened))
		RemoveChannel(channelOpened);

	UnattatchSharedChannel(sharedChannel);
	channelOpened->DeattatchSharedChannelSync();
}

std::shared_ptr<ProcessOpened> ObjLookup::MatchProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	auto moduleName = sharedProcess->ModuleName();
	auto range = _allProcessMap.equal_range(moduleName);
	for (auto iter = range.first; iter != range.second; ++iter)
	{
		auto processOpened = iter->second;
		if (!IsAttatched(processOpened))
		{
			return processOpened;
		}
	}

	return 0;
}


std::shared_ptr<ProcessOpened> ObjLookup::MatchProcess(const CString & name)
{
	for (auto iter = _allProcessMap.begin(); iter != _allProcessMap.end(); ++iter)
	{
		auto process = iter->second;
		if (!IsOpened(process))
		{
			if (wcscmp(process->Name().c_str(), name) == 0)
			{
				if (_loadLookupProcessSet.find(process) == _loadLookupProcessSet.end())
					return process;
			}
		}
	}

	return 0;
}

std::shared_ptr<ChannelOpened> ObjLookup::MatchChannel(const CString & name, std::shared_ptr<ProcessOpened> process, const CString & type)
{
	auto iter = _processChannelMap.find(process);
	assert(iter != _processChannelMap.end());
	for (auto iterCh = iter->second.begin(); iterCh != iter->second.end(); ++iterCh)
	{
		auto channel = *iterCh;
		if (this->IsOpened(channel))
			continue;

		CString typeCmp;
		typeCmp.Format(_T("%S"),typeid(*channel).name());

		if (wcscmp(typeCmp, type) != 0)
			continue;
		if (wcscmp(channel->Name().c_str(), name) != 0)
			continue;
		
		return channel;
	}

	return 0;
}


std::shared_ptr<ProcessOpened> ObjLookup::AddProcessUnattached(const CString & name)
{
	wstring stdName = name;
	auto process = make_shared<ProcessOpened>(stdName);
	this->AddProcess(process);
	this->MarkOpened(process, true);
	return process;
}

void ObjLookup::AddChannelUnattached(std::shared_ptr<ChannelOpened> channel, std::shared_ptr<ProcessOpened> process)
{
	this->AddChannel(channel, process);
	this->MarkOpened(channel, true);
}

std::shared_ptr<ChannelOpened> ObjLookup::MatchChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel, shared_ptr<ProcessOpened> & sptr)
{
	auto keyName = sharedChannel->Name();
	auto range = _allChannelMap.equal_range(keyName);
	for (auto iter = range.first; iter != range.second; ++iter)
	{
		auto channelOpened = iter->second;
		
		if (IsAttatched(channelOpened))
			continue;

		auto iterProcess = this->_channelProcessMap.find(channelOpened);
		assert(iterProcess != this->_channelProcessMap.end());
		auto process = iterProcess->second;
		//auto process = this->_channelProcessMap.find(channelOpened)->second;

		if (!IsAttatched(process))
			continue;

		if (process->Pid() != sharedChannel->Pid())
			continue;

		sptr = process;
		return channelOpened;
	}

	auto moduleName = ModuleName(sharedChannel->Pid());

	sptr = 0;
	auto rangePs = _allProcessMap.equal_range(moduleName);
	for (auto iter = rangePs.first; iter != rangePs.second; ++iter)
	{
		if (iter->second->Pid() == sharedChannel->Pid())
		{
			sptr = iter->second;
			break;
		}
	}

	return 0;
}

void ObjLookup::AddProcess(std::shared_ptr<ProcessOpened> processOpened)
{
	auto name = processOpened->Name();
	
	// 1. add to _allProcessMap
	_allProcessMap.insert(make_pair(name, processOpened));

	// 2. add to _processChannelMap
	_processChannelMap.insert(
		make_pair(processOpened, set<shared_ptr<ChannelOpened> >())
		);
}

void ObjLookup::RemoveProcess(shared_ptr<ProcessOpened> processOpened)
{
	for( auto iter = _allProcessMap.begin(); iter != _allProcessMap.end(); ++iter)
	{
		if (iter->second == processOpened)
		{
			_allProcessMap.erase(iter);
			break;
		}
	}

	auto iterProcess = _processChannelMap.find(processOpened);
	//assert(iterProcess->second.size() == 0);	//TODO
	_processChannelMap.erase(iterProcess);
}

void ObjLookup::AddChannel(std::shared_ptr<ChannelOpened> channelOpened, std::shared_ptr<ProcessOpened> processOpened)
{
	auto name = channelOpened->Name();

	// 1. add to _allChannelMap
	_allChannelMap.insert(make_pair(name, channelOpened));

	// 2. add to _channelProcessMap
	_channelProcessMap.insert(make_pair(
			channelOpened, processOpened
		));

	// 3. add to _processChannelMap
	auto iter = _processChannelMap.find(processOpened);
	iter->second.insert(channelOpened);

	this->RaiseProcessChannelUpdatedEvent();
}

void ObjLookup::RemoveChannel(shared_ptr<ChannelOpened> channelOpened)
{
	for (auto iter = _allChannelMap.begin(); iter != _allChannelMap.end(); ++iter)
	{
		if (iter->second == channelOpened)
		{
			_allChannelMap.erase(iter);
			break;
		}
	}

	auto iterChannelProcess = _channelProcessMap.find(channelOpened);
	assert(iterChannelProcess != _channelProcessMap.end());
	auto processOpened = iterChannelProcess->second;
	_channelProcessMap.erase(iterChannelProcess);

	auto iterProcessChannel = _processChannelMap.find(processOpened);
	auto iterChannel = iterProcessChannel->second.find(channelOpened);
	iterProcessChannel->second.erase(iterChannel);

	if (iterProcessChannel->second.size() == 0)
	{
		RemoveProcess(processOpened);
	}

	RaiseProcessChannelUpdatedEvent();
}

std::wstring ObjLookup::ModuleName(int pid)
{
	auto iter = _moduleName.find(pid);
	if (iter != _moduleName.end())
	{
		return iter->second;
	}
	else
	{
		wstring name = GetProcessNameByPid(pid); 
		_moduleName.insert(make_pair(pid, name));
		return name;
	}
}

void ObjLookup::ReleaseModuleName(int pid)
{
	auto iterModuleName = _moduleName.find(pid);
	_moduleName.erase(iterModuleName);
}

std::wstring ObjLookup::GetProcessNameByPid(int pid)
{
	wstring ret = L"";

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ, FALSE, pid);

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		TCHAR nameBuffer[MAX_PATH];

		if(::EnumProcessModules(hProcess, &hMod, 
			sizeof(hMod), &cbNeeded))
		{
			::GetModuleBaseName(hProcess, hMod, nameBuffer, 
				sizeof(nameBuffer)/sizeof(TCHAR));

			ret = nameBuffer;
		}

		::CloseHandle(hProcess);
	}	

	return ret;
}

std::shared_ptr<ChannelOpened> ObjLookup::MakeChannel(shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel)
{
	// factory
	switch(sharedChannel->Type())
	{
	case CH_TYPE_LINE:
		return make_shared<ChannelOpenedLine>();
	case CH_TYPE_SPECTRUM:
		return make_shared<ChannelOpenedSpectrum>();
	case CH_TYPE_DOTS:
		return make_shared<ChannelOpenedDots>();
	case CH_TYPE_TEXT:
		return make_shared<ChannelOpenedText>();
	case CH_TYPE_LOG:
		return make_shared<ChannelOpenedLog>();
	default:
		assert(FALSE);
	}

	return 0;
}

void ObjLookup::OpenProcess(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, process](){
		SThis->MarkOpened(process, true);
	});
}

void ObjLookup::OpenChannel(std::shared_ptr<ChannelOpened> channel, const function<void(bool)> & fCont)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, channel, fCont](){
		bool succ;
		if (SThis->IsAttatched(channel))
		{
			SThis->MarkOpened(channel, true);
			succ = true;

			auto iterProcess = SThis->_channelProcessMap.find(channel);
			assert(iterProcess != SThis->_channelProcessMap.end());
			auto process = iterProcess->second;
			if (!SThis->IsOpened(process))
				SThis->MarkOpened(process, true);
		}
		else
			succ = false;

		fCont(succ);
	});
}

void ObjLookup::CloseProcess(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, process](){
		if (SThis->IsOpened(process))
			SThis->MarkOpened(process, false);
		if (!SThis->IsAttatched(process))
		{
			SThis->RemoveProcess(process);
		}
	});
}

void ObjLookup::CloseChannel(std::shared_ptr<ChannelOpened> channel)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, channel](){
		if (SThis->IsOpened(channel))
		{
			SThis->MarkOpened(channel, false);
			auto iterProcess = SThis->_channelProcessMap.find(channel);
			assert(iterProcess != SThis->_channelProcessMap.end());
			auto process = iterProcess->second;
			
			auto iterChannel = SThis->_processChannelMap.find(process);
			assert(iterChannel != SThis->_processChannelMap.end());
			bool bOpened = false;
			for (auto iter = iterChannel->second.begin();
				iter != iterChannel->second.end();
				++iter)
			{
				if (SThis->IsOpened(*iter))
				{
					bOpened = true;
					break;
				}
			}

			if (!bOpened)
				if (SThis->IsOpened(process))
					SThis->MarkOpened(process, false);
		}
		if (!SThis->IsAttatched(channel))
		{
			SThis->RemoveChannel(channel);
		}
	});
}

void ObjLookup::CloseAll()
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis](){

		for (auto iterChannel = SThis->_allChannelMap.begin();
			iterChannel != SThis->_allChannelMap.end();
			++iterChannel)
		{
			auto channel = iterChannel->second;
			if (SThis->IsOpened(channel))
				SThis->MarkOpened(channel, false);
			if (!SThis->IsAttatched(channel))
			{
				SThis->RemoveChannel(channel);

			}
		}

		for (auto iterProcess = SThis->_allProcessMap.begin();
			iterProcess != SThis->_allProcessMap.end();
			++iterProcess)
		{
			auto process = iterProcess->second;
			if (SThis->IsOpened(process))
			{
				SThis->MarkOpened(process, false);
				if (!SThis->IsAttatched(process))
				{
					SThis->RemoveProcess(process);
				}
			}
		}
	});
}

void ObjLookup::RaiseProcessChannelUpdatedEvent()
{
	this->EventProcessChannelTreeChanged.Raise(this, _processChannelMap);
}

std::shared_ptr<ProcessOpened> ObjLookup::GetProcessByShared(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	return _shared2ProcessMap.find(sharedProcess)->second;
}

void ObjLookup::AttatchShared2Process(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess, std::shared_ptr<ProcessOpened> processOpened)
{
	_shared2ProcessMap.insert(make_pair(sharedProcess, processOpened));
	_processAttatchedSet.insert(processOpened);
}

void ObjLookup::UnattatchSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> sharedProcess)
{
	auto iter = _shared2ProcessMap.find(sharedProcess);
	_processAttatchedSet.erase(_processAttatchedSet.find(iter->second));
	_shared2ProcessMap.erase(iter);
}

std::shared_ptr<ChannelOpened> ObjLookup::GetChannelByShared(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel)
{
	auto iter = _shared2ChannelMap.find(sharedChannel);
	if (iter == _shared2ChannelMap.end())
		return 0;

	else
		return iter->second;
}

void ObjLookup::AttatchShared2Channel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel, std::shared_ptr<ChannelOpened> channelOpened)
{
	_shared2ChannelMap.insert(make_pair(sharedChannel, channelOpened));
	_channelAttatchedSet.insert(channelOpened);
}

void ObjLookup::UnattatchSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> sharedChannel)
{
	auto iter = _shared2ChannelMap.find(sharedChannel);
	_channelAttatchedSet.erase(_channelAttatchedSet.find(iter->second));
	_shared2ChannelMap.erase(iter);
}

bool ObjLookup::IsOpened(std::shared_ptr<ProcessOpened> processOpened)
{
	return _processOpenedSet.find(processOpened) != _processOpenedSet.end();
}

void ObjLookup::MarkOpened(std::shared_ptr<ProcessOpened> processOpened, bool isOpen)
{
	if (isOpen)
		_processOpenedSet.insert(processOpened);
	else
		_processOpenedSet.erase(_processOpenedSet.find(processOpened));
}


bool ObjLookup::IsOpened(std::shared_ptr<ChannelOpened> channelOpened)
{
	return _channelOpenedSet.find(channelOpened) != _channelOpenedSet.end();
}

void ObjLookup::MarkOpened(std::shared_ptr<ChannelOpened> channelOpened, bool isOpen)
{
	if (isOpen)
		_channelOpenedSet.insert(channelOpened);
	else
		_channelOpenedSet.erase(_channelOpenedSet.find(channelOpened));
}

bool ObjLookup::IsAttatched(std::shared_ptr<ProcessOpened> processOpened)
{
	return _processAttatchedSet.find(processOpened) != _processAttatchedSet.end();
}

bool ObjLookup::IsAttatched(std::shared_ptr<ChannelOpened> channelOpened)
{
	return _channelAttatchedSet.find(channelOpened) != _channelAttatchedSet.end();
}

void ObjLookup::Clear()
{
	auto SThis = shared_from_this();

	auto clearTask = [SThis](){
		SThis->EventPauseProcess.Reset();
		SThis->EventProcessChannelTreeChanged.Reset();
		SThis->EventNewProcessOpened.Reset();
		SThis->EventProcessOpenedAttatched.Reset();
		SThis->EventProcessOpenedDeattatched.Reset();
		SThis->EventPlotWindowLoaded.Reset();
		SThis->EventChannelLoaded.Reset();
	};

	_taskQueue->QueueTask(clearTask, clearTask);
}

//-------------------------------------------------------

void ObjLookup::Load(const CString & filepath)
{
	auto SThis = shared_from_this();

	this->_taskQueue->DoTask([SThis, filepath](){
		
		::CoInitialize(NULL);

		// XML TODO
		//...
		VARIANT var;
		VariantInit(&var);
		BSTR bstr = SysAllocString(filepath);
		V_VT(&var)   = VT_BSTR;
		V_BSTR(&var) = bstr;
		MSXML2::IXMLDOMDocumentPtr pXMLDom;

		HRESULT hr = pXMLDom.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

		pXMLDom->async = VARIANT_FALSE;
		pXMLDom->validateOnParse = VARIANT_FALSE;
		pXMLDom->resolveExternals = VARIANT_FALSE;
		pXMLDom->load(var);
		
		// v1.0 xml file loading
		MSXML2::IXMLDOMNodePtr versionNode = pXMLDom->selectSingleNode(L"//Version");
		if (versionNode == 0 || versionNode->text != _bstr_t("2.0"))
		{
			::AfxMessageBox(L"Incorrect file version.");
			CoUninitialize();
			return;
		}

		MSXML2::IXMLDOMNodeListPtr plotWndNodes = pXMLDom->selectNodes(L"//PlotWnd");

		if (plotWndNodes == 0)
		{
			::CoUninitialize();
			return;
		}
		
		std::map<int, shared_ptr<ProcessOpened> > processMap;

		if (plotWndNodes)
		{
			for (long i = 0; i < plotWndNodes->length; i++)
			{
				shared_ptr<ProcessOpened> processOpened;

				auto pNodeWnd = plotWndNodes->item[i];
				auto pNodeProcess = pNodeWnd->selectSingleNode(L"Process");
				if (pNodeProcess == 0)
					continue;

				BSTR processName = pNodeProcess->text;
				CString csProcessname;
				csProcessname.Format(_T("%s"),(LPCTSTR)processName);

				auto processNameNode = pNodeWnd->selectSingleNode(L"ProcessName");
				if (processNameNode == 0)
					continue;

				auto pWndTypeNode = pNodeWnd->selectSingleNode(L"WndType");
				if (pWndTypeNode == 0)
					continue;

				int processSid = atoi((LPCSTR)(processNameNode->text));
				auto itermap = processMap.find(processSid);
				if(itermap == processMap.end())
				{
					auto matchedProcess = SThis->MatchProcess(csProcessname);
					if (matchedProcess != 0)
					{
						processOpened = matchedProcess;
						SThis->MarkOpened(processOpened, true);
					}
					else
					{
						processOpened = SThis->AddProcessUnattached(csProcessname);
					}

					processMap.insert(make_pair(processSid, processOpened));
					SThis->_loadLookupProcessSet.insert(processOpened);
				}
				else
				{
					processOpened = itermap->second;
				}

				shared_ptr<PlotWindowOpened> plotWindowOpened;
				
				if(pWndTypeNode->text == _bstr_t(typeid(PlotWindowOpenedLine).name()))
				{
					plotWindowOpened = make_shared<PlotWindowOpenedLine>(processOpened);
				}
				else if (pWndTypeNode->text == _bstr_t(typeid(PlotWindowOpenedSpectrum).name())) 
				{
					plotWindowOpened = make_shared<PlotWindowOpenedSpectrum>(processOpened);
				}
				else if (pWndTypeNode->text == _bstr_t(typeid(PlotWindowOpenedDots).name())) 
				{
					plotWindowOpened = make_shared<PlotWindowOpenedDots>(processOpened);
				}
				else if (pWndTypeNode->text == _bstr_t(typeid(PlotWindowOpenedText).name())) 
				{
					plotWindowOpened = make_shared<PlotWindowOpenedText>(processOpened);
				}
				else if (pWndTypeNode->text == _bstr_t(typeid(PlotWindowOpenedLog).name())) 
				{
					plotWindowOpened = make_shared<PlotWindowOpenedLog>(processOpened);
				}
				else
				{
					::AfxMessageBox(L"File format error.");
					continue;
				}

				plotWindowOpened->LoadXmlElement(pNodeWnd);
				PlotWindowLoadedEvent e;
				e._plotWnd = plotWindowOpened;
				e._process = processOpened;
				plotWindowOpened->GetRect(e._rect);

				SThis->_mapProcessPlotwnd.insert(make_pair(processOpened, plotWindowOpened));				

				SThis->EventPlotWindowLoaded.Raise(SThis.get(), e);

				// load channels

				MSXML2::IXMLDOMNodeListPtr pChannelNodes = pNodeWnd->selectNodes(L"./Channel");

				if (pChannelNodes == 0)
					continue;

				for (long node = 0; node < pChannelNodes->length; ++node)
				{
					auto pChannelNode = pChannelNodes->item[node];
					auto pTypeNode = pChannelNode->selectSingleNode(L"SType");
					if (pTypeNode == 0)
						continue;

					CString csType;
					csType.Format(_T("%S"),(LPCSTR)pTypeNode->text);

					auto pNameNode = pChannelNode->selectSingleNode(L"SName");
					if (pNameNode == 0)
						continue;

					CString csName;
					csName.Format(_T("%S"),(LPCSTR)pNameNode->text);

					shared_ptr<ChannelOpened> channelOpened;

					auto matchedChannel = SThis->MatchChannel(csName, processOpened, csType);

					if (matchedChannel != 0)
					{
						channelOpened = matchedChannel;
						SThis->MarkOpened(channelOpened, true);
					}
					else
					{
						if(pTypeNode->text == _bstr_t(typeid(ChannelOpenedLine).name()))
						{
							channelOpened = make_shared<ChannelOpenedLine>();
						}
						else if(pTypeNode->text == _bstr_t(typeid(ChannelOpenedSpectrum).name()))
						{
							channelOpened = make_shared<ChannelOpenedSpectrum>();
						}
						else if(pTypeNode->text == _bstr_t(typeid(ChannelOpenedDots).name()))
						{
							channelOpened = make_shared<ChannelOpenedDots>();
						}
						else if(pTypeNode->text == _bstr_t(typeid(ChannelOpenedText).name()))
						{
							channelOpened = make_shared<ChannelOpenedText>();
						}
						else if(pTypeNode->text == _bstr_t(typeid(ChannelOpenedLog).name()))
						{
							channelOpened = make_shared<ChannelOpenedLog>();
						}
						else
						{
							::AfxMessageBox(L"File format error.");
							continue;
						}
					}

					channelOpened->LoadXmlElement(pChannelNode);

					if (matchedChannel == 0)
						SThis->AddChannelUnattached(channelOpened, processOpened);

					ChannelLoadedEvent e;
					e._channel = channelOpened;
					e._plotWnd = plotWindowOpened;
					channelOpened->GetRect(e._rect);

					SThis->EventChannelLoaded.Raise(SThis.get(), e);
				}
			}
		}

		SThis->_loadLookupProcessSet.clear();

		::CoUninitialize();
	});
}

void ObjLookup::Save(const CString & filepath)
{
	auto SThis = shared_from_this();
	this->_taskQueue->DoTask([SThis, filepath](){

		::CoInitialize(NULL);

		BSTR savePath = SysAllocString(filepath);
		IXMLDOMDocument *pXmlDoc;
		IXMLDOMNode *pout;
		IXMLDOMElement * pVersionElement = NULL;
		IXMLDOMElement *pRootElement = NULL;
		IXMLDOMElement *pe = NULL;

		HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pXmlDoc));

		assert(hr == S_OK);

		VARIANT varFileName;
		VariantInit(&varFileName);

		// settings
		pXmlDoc->put_async(VARIANT_FALSE);  
		pXmlDoc->put_validateOnParse(VARIANT_FALSE);
		pXmlDoc->put_resolveExternals(VARIANT_FALSE);
		pXmlDoc->put_preserveWhiteSpace(VARIANT_TRUE);

		// create and add root element
		pXmlDoc->createElement(L"DebugSession",&pRootElement);
		pXmlDoc->appendChild(pRootElement,&pout);
		
		// create version element
		CreateAndAddElementNode(pXmlDoc, L"Version", L"\n\t", (IXMLDOMNode *)pRootElement, &pVersionElement);
		pVersionElement->put_text(L"2.0");

		for (auto iter = SThis->_mapProcessPlotwnd.begin();
			iter != SThis->_mapProcessPlotwnd.end();
			++iter)
		{
			auto plotWnd = iter->second;
			plotWnd->CreateXmlElement(pXmlDoc, pRootElement);
		}
		
		CreateAndAddTextNode(pXmlDoc, L"\n", pRootElement);

		V_VT(&varFileName)   = VT_BSTR;
		V_BSTR(&varFileName) = savePath;
		hr = pXmlDoc->save(varFileName);
		SysFreeString(savePath);

		::CoUninitialize();

	});
}

void ObjLookup::AddPlotWnd(std::shared_ptr<PlotWindowOpened> plotWnd, std::shared_ptr<ProcessOpened> process)
{
	auto SThis = shared_from_this();
	this->_taskQueue->QueueTask([SThis, plotWnd, process](){
		SThis->_mapProcessPlotwnd.insert(make_pair(process, plotWnd));
	});
}

void ObjLookup::RemovePlotWnd(std::shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = shared_from_this();
	this->_taskQueue->QueueTask([SThis, plotWnd](){
		for (auto iter = SThis->_mapProcessPlotwnd.begin();
			iter != SThis->_mapProcessPlotwnd.end();)
		{
			if (iter->second == plotWnd)
			{
				SThis->_mapProcessPlotwnd.erase(iter++);
			}
			else
			{
				++iter;
			}
		}
	});
}

void ObjLookup::ClearPlotWindows()
{
	auto SThis = shared_from_this();
	this->_taskQueue->QueueTask([SThis](){
		SThis->CloseAllPlotWindow();
	});	
}

void ObjLookup::CloseAllPlotWindow()
{
	for (auto iter = this->_mapProcessPlotwnd.begin();
		iter != this->_mapProcessPlotwnd.end();
		++iter)
	{
		auto plotWnd = iter->second;
		plotWnd->Close();

		auto process = iter->first;

		for (auto iterProcess = this->_processChannelMap.begin();
			iterProcess != this->_processChannelMap.end();
			++iterProcess)
		{
			for (auto iterCh = iterProcess->second.begin();
				iterCh != iterProcess->second.end();
				++iterCh)
			{
				auto channel = *iterCh;
				if (this->IsOpened(channel))
					this->MarkOpened(channel, false);
			}

			auto process = iterProcess->first;
			if (this->IsOpened(process))
				this->MarkOpened(process, false);
		}
	}

	this->_mapProcessPlotwnd.clear();
}

void ObjLookup::PollPauseEvent()
{
	if (::InterlockedCompareExchange(&_pollPauseEventToken, 1, 0) != 0)
		return;

	auto SThis = shared_from_this();
	this->_taskQueue->QueueTask([SThis](){
		auto SThis2 = SThis;
		for (
			auto iterPs = SThis->_processAttatchedSet.begin();
			iterPs != SThis->_processAttatchedSet.end();
			++iterPs)
		{
			auto process = *iterPs;
			process->PollPauseEvent([SThis2, process](bool isPause){
				if (isPause)
				{
					SThis2->EventPauseProcess.Raise(SThis2.get(), process);
				}
			});
		}

		::InterlockedCompareExchange(&SThis->_pollPauseEventToken, 0, 1);
	});
}
