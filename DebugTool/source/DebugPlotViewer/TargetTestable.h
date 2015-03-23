#pragma once

class TargetTestable
{
public:
	virtual ~TargetTestable() {}
	virtual bool TestTarget(void * obj) = 0;
	virtual void AddToTarget(void * obj) = 0;
	virtual void HighLight(bool highlight) = 0;
};
