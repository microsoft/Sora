#pragma once

#include "Histogram.h"

template <typename T>
class Integral
{
public:
	Integral(int binCount, T rangeMinIncluded, T rangeMaxNotIncluded);
	~Integral();
	void Clear();
	void Calc();
	void AddData(T * data, int count);
	int GetBin(int index);
	
private:
	Histogram<T> * histogram;
	int * bins;
	int binCount;
};


template <typename T>
Integral<T>::Integral(int binCount, T rangeMinIncluded, T rangeMaxNotIncluded)
{
	this->binCount = binCount;
	this->bins = new T[binCount];
	this->histogram = new Histogram<T>(binCount, rangeMinIncluded, rangeMaxNotIncluded);
}

template <typename T>
Integral<T>::~Integral()
{
	if (bins)
		delete [] bins;
	if (histogram)
		delete histogram;
}

template <typename T>
void Integral<T>::Clear()
{
	histogram->Clear();
	for (int i = 0; i < binCount; i++)
	{
		bins[i] = 0;
	}
}

template <typename T>
void Integral<T>::Calc()
{
	bins[0] = histogram->GetBin(0);
	for (int i = 1; i < binCount; i++)
	{
		bins[i] = histogram->GetBin(i) + bins[i-1];
	}
}

template <typename T>
void Integral<T>::AddData(T * data, int count)
{
	if (count <= 0)
		return;

	histogram->AddData(data, count);
}

template <typename T>
int Integral<T>::GetBin(int index)
{
	return bins[index];
}
