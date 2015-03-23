#pragma once

#include "HashTable.h"
#include "SimpleVector.h"

unsigned long HashCodeString(const char * str);
bool EqualString(const char * s1, const char * s2);
unsigned long HashCodeInt(int n);
bool EqualInt(int n1, int n2);

inline unsigned long HashCodeString(const char * str)
{
	int hashVal = 4967;
	int c;

	while (c = *str++)
		hashVal = ((hashVal << 4) + hashVal) + c;

	return hashVal;
}

inline bool EqualString(const char * s1, const char * s2)
{
	return strcmp(s1, s2) == 0;
}

struct SeriesCachedInfo;
typedef SimpleHashTable<const char *, SeriesCachedInfo *, 1024, HashCodeString, EqualString> HashType;

HashType * SingletonGetHashTable();
void SingletonReleaseHashTable();


typedef SimpleVector<char *> TextBufVecType;
TextBufVecType * SingletonGetTextVector();
void SingletonReleaseTextVector();
