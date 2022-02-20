#include "ObjectsStore.hpp"

#include "EngineClasses.hpp"


ObjectsIterator ObjectsStore::begin()
{
    return ObjectsIterator(*this, 0);
}

ObjectsIterator ObjectsStore::begin() const
{
    return ObjectsIterator(*this, 0);
}

ObjectsIterator ObjectsStore::end()
{
    return ObjectsIterator(*this);
}

ObjectsIterator ObjectsStore::end() const
{
    return ObjectsIterator(*this);
}

UEClass ObjectsStore::FindClass(const std::string& name) const
{
    for (auto obj : *this)
    {
        if (obj.GetFullName() == name)
        {
            return obj.Cast<UEClass>();
        }
    }
    return UEClass(nullptr);
}

ObjectsIterator::ObjectsIterator(const ObjectsStore& _store)
    : store(_store),
      index(_store.GetObjectsNum())
{
}

ObjectsIterator::ObjectsIterator(const ObjectsStore& _store, size_t _index)
    : store(_store),
      index(_index),
      current(_store.GetById(_index))
{
}

ObjectsIterator::ObjectsIterator(const ObjectsIterator& other)
    : store(other.store),
      index(other.index),
      current(other.current)
{
}

ObjectsIterator::ObjectsIterator(ObjectsIterator&& other) noexcept
    : store(other.store),
      index(other.index),
      current(other.current)
{
}

ObjectsIterator& ObjectsIterator::operator=(const ObjectsIterator& rhs)
{
    index = rhs.index;
    current = rhs.current;
    return *this;
}

void ObjectsIterator::swap(ObjectsIterator& other) noexcept
{
    std::swap(index, other.index);
    std::swap(current, other.current);
}

ObjectsIterator& ObjectsIterator::operator++()
{
    for (++index; index < store.GetObjectsNum(); ++index)
    {
        current = store.GetById(index);
        if (current.IsValid())
        {
            break;
        }
    }
    return *this;
}

ObjectsIterator ObjectsIterator::operator++(int)
{
    auto tmp(*this);
    ++(*this);
    return tmp;
}

bool ObjectsIterator::operator==(const ObjectsIterator& rhs) const
{
    return index == rhs.index;
}

bool ObjectsIterator::operator!=(const ObjectsIterator& rhs) const
{
    return index != rhs.index;
}

UEObject ObjectsIterator::operator*() const
{
    assert(current.IsValid() && "ObjectsIterator::current is not valid!");

    return current;
}

UEObject ObjectsIterator::operator->() const
{
    return operator*();
}

class FUObjectItem
{
public:
	UObject* Object; //0x0000
	__int32 Flags; //0x0008
	__int32 ClusterIndex; //0x000C
	__int32 SerialNumber; //0x0010
    char pad1[0x4];
};

class TUObjectArray
{
public:
	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;
};

class FUObjectArray
{
public:
	__int32 ObjFirstGCIndex; //0x0000
	__int32 ObjLastNonGCIndex; //0x0004
	__int32 MaxObjectsNotConsideredByGC; //0x0008
	__int32 OpenForDisregardForGC; //0x000C

	TUObjectArray ObjObjects; //0x0010
};

FUObjectArray* GlobalObjects = nullptr;

bool ObjectsStore::Initialize(uintptr_t GUObjectArray_Addr)
{
   if(GUObjectArray_Addr == 0)
        return false;
        
        GlobalObjects = (FUObjectArray*) (GUObjectArray_Addr);
        
	return true;
}

void* ObjectsStore::GetAddress()
{
	return GlobalObjects;
}

size_t ObjectsStore::GetObjectsNum() const
{
	return GlobalObjects->ObjObjects.NumElements;
}

UEObject ObjectsStore::GetById(size_t id) const
{
	return GlobalObjects->ObjObjects.Objects[id].Object;
}
