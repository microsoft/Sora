#include <assert.h>
#include "Strategy.h"

using namespace SoraDbgPlot::Strategy;

int TestStrategy()
{
	Strategy<int, int> s;
	s.Set([](const void * sender, const int & i, int & r){
		r = i * 2;
	});

	int ret;
	bool succ = s.Call(0, 1, ret);
	assert(succ == true);
	assert(ret == 2);

	s.Set([](const void * sender, const int & i, int & r){
		r = i * 3;
	});

	succ = s.Call(0, 1, ret);
	assert(succ == true);
	assert(ret == 3);

	s.UnSet();

	succ = s.Call(0, 1, ret);
	assert(succ == false);
	assert(ret == 3);	// unchanged

	return 0;
}
