#pragma once

#include <assert.h>
#include <functional>
#include <memory>
#include "Event.h"

using namespace std;

class EventSelfDeletion
{
public:
	SoraDbgPlot::Event::Event<int> EventClose;
	void Close()
	{
		this->EventClose.Raise(this, 0);
	}
};

class A
{
public:
	A(int num){
		_num = num;
	}

	~A()
	{
		_num = 0;
	}

	int Num() const {
		return _num;
	}


private:
	int _num;
};

class B
{
public:
	B()
	{
		auto a = make_shared<A>(10);
		_f = MakeLambda(a);
		_g = MakeLambda(a);
	}

	void Func()
	{
		_f();
		_g();
	}

	function<void(void)> MakeLambda(shared_ptr<A> a)
	{
		a.reset();
		A aa(100);
		return [a, aa](){
			printf("%d\n", a->Num(), aa.Num());
		};
	}

	~B()
	{
		int j = 0;
	}

private:
	//shared_ptr<A> _a;
	std::function<void(void)> _f;
	std::function<void(void)> _g;
};

static int Test1()
{
	//shared_ptr<A> sp(new A(10));

	//auto f = [sp](){
	//	printf("%d\n", *sp);
	//};

	//sp.reset();

	B * b = new B;
	b->Func();

	delete b;

	return 0;
}

static int Test2()
{
	auto s = make_shared<EventSelfDeletion>();
	auto ss = s;

	s->EventClose.Subscribe([/*&s, */ss](const void * sender, const int & num){
		//s.reset();
	});

	s->Close();

	return 0;
}

int TestEventLambdaDeleteSelf()
{
	Test2();
	return 0;
}
