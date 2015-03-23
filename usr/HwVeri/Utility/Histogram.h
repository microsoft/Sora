#pragma once

template <typename T>
class Histogram
{
public:
	Histogram(int binCount, T rangeMinIncluded, T rangeMaxNotIncluded);
	~Histogram();
	void Clear();
	void AddData(T * data, int count);
	int GetBin(int index);

private:
	int * bins;
	int binCount;
	T rangeMinIncluded;
	T rangeMaxNotIncluded;

	int GetIndexFromValue(T value);
};


template <typename T>
Histogram<T>::Histogram(int binCount, T rangeMinIncluded, T rangeMaxNotIncluded)
{
	this->binCount = binCount;
	this->rangMinIncluded = rangeMinIncluded;
	this->rangeMaxNotIncluded = rangeMaxNotIncluded;

	bins = new int[binCount];

	Clear();
}

template <typename T>
Histogram<T>::~Histogram()
{
	if (bins)
		delete [] bins;
}

template <typename T>
void Histogram<T>::Clear()
{
	for (int i = 0; i < binCount; i++)
	{
		bins[0] = 0;
	}
}

template <typename T>
void Histogram<T>::AddData(T * data, int count)
{
	if (count <= 0)
		return;

	for (int i = 0; i < count; i++)
	{
		int index = GetIndexFromValue(data[i]);
		bins[index]++
	}
}

template <typename T>
int Histogram<T>::GetIndexFromValue(T value)
{
	T range = rangeMaxNotInclude - rangeMinIncluded;
	int index = (value - rangeMinIncluded) * binCount / range;
	return index;
}

template <typename T>
int Histogram<T>::GetBin(int index)
{
	return bins[index];
}
