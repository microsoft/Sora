#include <Windows.h>
#include <assert.h>
#include <sstream>
#include "ILog.h"
#include "LogWithFileBackup.h"

using namespace std;

int TestFileLog()
{
	const int MAX_NAME = 256;
	char folder[256];
	::GetCurrentDirectoryA(MAX_NAME-1, folder);

	ostringstream ss;
	ss << folder << "\\logFolder\\bbb";

	ILog * log = LogWithBackUpFile::Make(ss.str().c_str(), "aLog");

	assert(log);

	char outbuf[64];
	for (int i = 0; i < 100; ++i)
	{
		sprintf(outbuf, "record: %d\n", i);
		log->AddRecord(outbuf);
	}

	for (int i = 0; i < log->RecordCount(); ++i)
	{
		char * logRecord = log->Record(i);
		assert(logRecord != nullptr);
		printf("%s", logRecord);
		delete [] logRecord;
	}

	delete log;

	return 0;
}
