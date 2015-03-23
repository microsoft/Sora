#pragma once

template <typename Key, typename Value>
class HashEntry
{
public:
	HashEntry();
	HashEntry(Key key, Value value);
	Key key;
	Value value;
	HashEntry<Key, Value> * _next;
};

template <typename Key, typename Value>
HashEntry<Key, Value>::HashEntry() :
	_next(0)
{}

template <typename Key, typename Value>
HashEntry<Key, Value>::HashEntry(Key key, Value value) :
	_next(0)
{
	this->key = key;
	this->value = value;
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
class SimpleHashTable
{
public:
	SimpleHashTable();
	~SimpleHashTable();
	void Insert(Key key, Value value);
	HashEntry<Key, Value> * Find(Key key);
	void Foreach(bool (*VisitorFunc)(Key key, Value value));
	void Clear();
private:
	HashEntry<Key, Value> * hashBucket[_hashTableSize];
	void InsertToList(HashEntry<Key, Value> ** ppEntryHead, HashEntry<Key, Value> * pEntryToInsert);
	HashEntry<Key, Value> ** HashCode2EntryListHead(unsigned long hashCode);
	void DeleteEntryList(HashEntry<Key, Value> ** ppEntry);
};

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::SimpleHashTable()
{
	for (int i = 0; i < _hashTableSize; i++)
	{
		hashBucket[i] = 0;
	}
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::~SimpleHashTable()
{
	Clear();
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
void SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::Clear()
{
	for (int i = 0; i < _hashTableSize; i++)
	{
		DeleteEntryList(this->hashBucket + i);
	}
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
void SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::DeleteEntryList(HashEntry<Key, Value> ** ppEntry)
{
	if (*ppEntry == 0)
		return;

	DeleteEntryList(&(*ppEntry)->_next);
	delete *ppEntry;
	*ppEntry = 0;
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
void SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::Insert(Key key, Value value)
{
	HashEntry<Key, Value> ** ppHashEntryListHead = HashCode2EntryListHead(HashCodeFcn(key));
	HashEntry<Key, Value> * hashEntry = new HashEntry<Key, Value>(key, value);
	InsertToList(ppHashEntryListHead, hashEntry);
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
HashEntry<Key, Value> ** SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::HashCode2EntryListHead(unsigned long hashCode)
{
	return this->hashBucket + hashCode % _hashTableSize;
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
void SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::InsertToList(HashEntry<Key, Value> ** ppEntryHead, HashEntry<Key, Value> * pEntryToInsert)
{
	HashEntry<Key, Value> * pEntryHead = *ppEntryHead;
	if (pEntryHead == 0)
	{
		*ppEntryHead = pEntryToInsert;
	}
	else
	{
		HashEntry<Key, Value> * p = pEntryHead;
		while(p->_next != 0) p = p->_next;
		p->_next = pEntryToInsert;
	}
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
HashEntry<Key, Value> * SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::Find(Key key)
{
	HashEntry<Key, Value> ** ppHashEntryListHead = HashCode2EntryListHead(HashCodeFcn(key));
	
	HashEntry<Key, Value> * pEntry = *ppHashEntryListHead;
	while (pEntry != 0)
	{
		if (EqualKey(pEntry->key, key))
			return pEntry;
		pEntry = pEntry->_next;
	}

	return 0;
}

template <typename Key, typename Value, int _hashTableSize, unsigned long (*HashCodeFcn)(Key key), bool (*EqualKey)(Key key1, Key key2)>
void SimpleHashTable<Key, Value, _hashTableSize, HashCodeFcn, EqualKey>::Foreach(bool (*VisitorFunc)(Key key, Value value))
{
	for (int i = 0; i < _hashTableSize; i++)
	{
		HashEntry<Key, Value> * pEntry = this->hashBucket[i];
		while(pEntry != 0)
		{
			bool continueLoop = VisitorFunc(pEntry->key, pEntry->value);
			if (! continueLoop)
				break;

			pEntry = pEntry->_next;
		}
	}
}
