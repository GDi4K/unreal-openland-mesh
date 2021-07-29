#pragma once

#include <vector>

using namespace std;

template <typename T>
class OLArray
{
	bool bLockForever = false;
	bool bFreeze = false;
	bool bLocked = false;
	vector<T> Data;

	void CheckLocked()
	{
		checkf(bLocked == false, TEXT("It's not possible to modify values of a locked OLAarray<%hs>"), typeid(T).name())
	}

	void CheckFreeze()
	{
		checkf(bFreeze == false, TEXT("It's not possible to extend a freezed OLArray<%hs>"), typeid(T).name())
	}

	void CheckLockForever()
	{
		checkf(bLockForever == false, TEXT("It's not possible to unlock OLArray<%hs> which is locked forever"),
		       typeid(T).name())
	}

public:
	OLArray()
	{
	}

	OLArray(std::initializer_list<T> InitialList)
	{
		Data.reserve(InitialList.size());
		for (T Item : InitialList)
			Push(Item);
	}

	size_t Push(const T Item)
	{
		CheckFreeze();
		CheckLocked();
		size_t Index = Data.size();
		Data.push_back(Item);

		return Index;
	}

	void Clear()
	{
		Data.clear();
	}

	T Get(size_t Index)
	{
		return Data[Index];
	}

	const T Get(size_t Index) const
	{
		return Data[Index];
	}

	T& GetRef(size_t Index)
	{
		CheckLocked();
		return Data[Index];
	}

	const T& GetRef(size_t Index) const
	{
		CheckLocked();
		return Data[Index];
	}

	void Set(size_t Index, T Value)
	{
		CheckLocked();
		Data[Index] = Value;
	}

	size_t Length() const
	{
		return Data.size();
	}

	void SetLength(size_t NewSize)
	{
		CheckFreeze();
		CheckLocked();
		Data.resize(NewSize);
	}

	void Append(OLArray<T> Other)
	{
		for (size_t Index = 0; Index < Other.Length(); Index++)
			Push(Other.Get(Index));
	}

	void Freeze()
	{
		bFreeze = true;
	}

	void LockForever()
	{
		bLockForever = true;
		Lock();
	}

	void Lock()
	{
		bLocked = true;
	}

	void UnLock()
	{
		CheckLockForever();
		bLocked = false;
	}
};
