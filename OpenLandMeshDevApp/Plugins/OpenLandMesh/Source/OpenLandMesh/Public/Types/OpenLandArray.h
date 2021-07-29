// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include <vector>

using namespace std;

template <typename T>
class OPENLANDMESH_API TOpenLandArray
{
	bool bLockForever = false;
	bool bFreeze = false;
	bool bLocked = false;
	vector<T> Data;

	void CheckLocked();
	void CheckFreeze();
	void CheckLockForever();

public:
	TOpenLandArray();
	TOpenLandArray(std::initializer_list<T> InitialList);

	size_t Push(const T Item);

	void Clear();

	T Get(size_t Index);
	const T Get(size_t Index) const;

	T& GetRef(size_t Index);

	void Set(size_t Index, T Value);

	size_t Length() const;
	void SetLength(size_t NewSize);

	void Append(TOpenLandArray<T> Other);

	void Freeze();

	void LockForever();

	void Lock();

	void UnLock();
};
