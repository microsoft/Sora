#include "ShareMemHelper.h"

struct MySharedObj
{
	volatile bool bAssociate;		// Note!! volatile keyword is important here
};

// you can write your own init function
inline BOOL SetToZeroInit(ShareMem * sm, void * context)
{
	MySharedObj * sharedObj = (MySharedObj *)sm->GetAddress();
	memset(sharedObj, 0, sizeof(MySharedObj));
}

void Sample()
{
	ShareMem * sm = AllocateSharedMem(
		L"NameOfTheSharedMemory",		// name
		sizeof(MySharedObj),					// size
		&DummyShareMemClassInit,				// ptr to init function
		NULL);									// context parameter

	
	MySharedObj * sharedObj = (MySharedObj *)sm->GetAddress();

	// you can write
	sharedObj->bAssociate = true;
	sharedObj->bAssociate = false;

	// or you can read
	bool state = sharedObj->bAssociate;
}
