#include "NamesStore.hpp"

#include "EngineClasses.hpp"

NamesIterator NamesStore::begin()
{
    return NamesIterator(*this, 0);
}

NamesIterator NamesStore::begin() const
{
    return NamesIterator(*this, 0);
}

NamesIterator NamesStore::end()
{
    return NamesIterator(*this);
}

NamesIterator NamesStore::end() const
{
    return NamesIterator(*this);
}

NamesIterator::NamesIterator(const NamesStore& _store)
    : store(_store),
      index(_store.GetNamesNum())
{
}

NamesIterator::NamesIterator(const NamesStore& _store, size_t _index)
    : store(_store),
      index(_index)
{
}

void NamesIterator::swap(NamesIterator& other) noexcept
{
    std::swap(index, other.index);
}

NamesIterator& NamesIterator::operator++()
{
    for (++index; index < store.GetNamesNum(); ++index)
    {
        if (store.IsValid(index))
        {
            break;
        }
    }
    return *this;
}

NamesIterator NamesIterator::operator++ (int)
{
    auto tmp(*this);
    ++(*this);
    return tmp;
}

bool NamesIterator::operator==(const NamesIterator& rhs) const
{
    return index == rhs.index;
}

bool NamesIterator::operator!=(const NamesIterator& rhs) const
{
    return index != rhs.index;
}

UENameInfo NamesIterator::operator*() const
{
    return { index, store.GetById(index) };
}

UENameInfo NamesIterator::operator->() const
{
    return { index, store.GetById(index) };
}

class FNameEntry
{
public:

    FNameEntry* HashNext;
    __int32 Index;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	const char* GetName() const
	{
		return AnsiName;
	}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
	int32_t Num() const
	{
		return NumElements;
	}

	bool IsValidIndex(int32_t index) const
	{
		return index >= 0 && index < Num() && GetById(index) != nullptr;
	}

	ElementType const* const& GetById(int32_t index) const
	{
		return *GetItemPtr(index);
	}

private:
	ElementType const* const* GetItemPtr(int32_t Index) const
	{
		int32_t ChunkIndex = Index / ElementsPerChunk;
		int32_t WithinChunkIndex = Index % ElementsPerChunk;
		ElementType** Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	enum
	{
		ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
	};

	ElementType** Chunks[ChunkTableSize];
	__int32 NumElements;
	__int32 NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

TNameEntryArray* GlobalNames = nullptr;

bool NamesStore::Initialize(uintptr_t GNames_Addr)
{
    if(GNames_Addr == 0)
        return false;
        
        GlobalNames = (TNameEntryArray*) (GNames_Addr);
        
	return true;
}

void* NamesStore::GetAddress()
{
	return GlobalNames;
}

size_t NamesStore::GetNamesNum() const
{
	return GlobalNames->Num();
}

bool NamesStore::IsValid(size_t id) const
{
	return GlobalNames->IsValidIndex(static_cast<int32_t>(id));
}

std::string NamesStore::GetById(size_t id) const
{
	return GlobalNames->GetById(static_cast<int32_t>(id))->GetName();
}
