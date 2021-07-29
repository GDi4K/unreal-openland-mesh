#pragma once

#include "Types/OpenLandArray.h"
#include "Types/OpenLandMeshTriangle.h"
#include "Types/OpenLandMeshInfo.h"

template <typename T>
void TOpenLandArray<T>::CheckLocked()
{
	checkf(bLocked == false, TEXT("It's not possible to modify values of a locked OLAarray<%hs>"), typeid(T).name())
}

template <typename T>
void TOpenLandArray<T>::CheckFreeze()
{
	checkf(bFreeze == false, TEXT("It's not possible to extend a freezed OLArray<%hs>"), typeid(T).name())
}

template <typename T>
void TOpenLandArray<T>::CheckLockForever()
{
	checkf(bLockForever == false, TEXT("It's not possible to unlock OLArray<%hs> which is locked forever"), typeid(T).name())
}

template <typename T>
TOpenLandArray<T>::TOpenLandArray()
{
	
}

template <typename T>
TOpenLandArray<T>::TOpenLandArray(std::initializer_list<T> InitialList)
{
	Data.reserve(InitialList.size());
	for (T Item: InitialList)
	{
		Push(Item);
	}
}

template <typename T>
size_t TOpenLandArray<T>::Push(const T Item)
{
	CheckFreeze();
	CheckLocked();
	size_t Index = Data.size();
	Data.push_back(Item);

	return Index;
}

template <typename T>
void TOpenLandArray<T>::Clear()
{
	Data.clear();
}

template <typename T>
T TOpenLandArray<T>::Get(size_t Index)
{
	return Data[Index];
}

template <typename T>
const T TOpenLandArray<T>::Get(size_t Index) const
{
	return Data[Index];
}

template <typename T>
T& TOpenLandArray<T>::GetRef(size_t Index)
{
	CheckLocked();
	return Data[Index];
}

template <typename T>
void TOpenLandArray<T>::Set(size_t Index, T Value)
{
	CheckLocked();
	Data[Index] = Value;
}

template <typename T>
size_t TOpenLandArray<T>::Length() const
{
	return Data.size();
}

template <typename T>
void TOpenLandArray<T>::SetLength(size_t NewSize)
{
	CheckFreeze();
	CheckLocked();
	Data.resize(NewSize);
}

template <typename T>
void TOpenLandArray<T>::Append(TOpenLandArray<T> Other)
{
	for(size_t Index=0; Index<Other.Length(); Index++)
	{
		Push(Other.Get(Index));
	}
}

template <typename T>
void TOpenLandArray<T>::Freeze()
{
	bFreeze = true;
}

template <typename T>
void TOpenLandArray<T>::LockForever()
{
	bLockForever = true;
	Lock();
}

template <typename T>
void TOpenLandArray<T>::Lock()
{
	bLocked = true;
}

template <typename T>
void TOpenLandArray<T>::UnLock()
{
	CheckLockForever();
	bLocked = false;
}

template class TOpenLandArray<FVector>;
template class TOpenLandArray<FOpenLandMeshTriangle>;
template class TOpenLandArray<FOpenLandMeshVertex>;
template class TOpenLandArray<FSimpleMeshInfoPtr>;