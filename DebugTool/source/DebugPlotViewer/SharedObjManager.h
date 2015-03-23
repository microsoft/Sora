#pragma once
#include <map>
#include "SeriesObj.h"
#include "ViewTree.h"

class SharedObjManager
{
public:
	static SharedObjManager * Instance();
	static void Clean();
private:
	static SharedObjManager * instance;

public:
	~SharedObjManager();

	std::multimap<wstring, DebuggingProcessProp *> allProcessObjs;	// strong ptr
	std::multimap<wstring, SeriesProp *> allSeriesObjs;				// strong ptr
	std::map<int, SeriesProp *> seriesMapForReading;

	void LockAndUpdate();
	void LockAndClear();
	bool IsChanged();

	void Lock();
	void Unlock();

private:
	SharedObjManager();
	void RemoveDeadProcess();
	void MarkAllObjsCheck();
	void AddNewObjs();
	void RemoveClosedObj();
	DebuggingProcessProp * MatchProcess(SharedProcessInfo * sharedProcessInfo);
	SeriesProp * MatchSeries(SharedSeriesInfo * sharedSeriesInfo);

	int lastUpdateIndex;
	bool shouldUpdate;

	std::map<int, DebuggingProcessProp *> allActiveProcessObjs;	// strong ptr
	std::map<int, SeriesProp *> allActiveSeriesObjs;				// strong ptr



	CRITICAL_SECTION cs;
};

