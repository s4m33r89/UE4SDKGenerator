#include "IGenerator.hpp"
#include "ObjectsStore.hpp"
#include "NamesStore.hpp"
#include "Main.h"

class Generator : public IGenerator
{
public:
	bool Initialize(void* module) override
	{
		alignasClasses = {
			{ "ScriptStruct CoreUObject.Plane", 16 },
			{ "ScriptStruct CoreUObject.Quat", 16 },
			{ "ScriptStruct CoreUObject.Transform", 16 },
			{ "ScriptStruct CoreUObject.Vector4", 16 },

			{ "ScriptStruct Engine.RootMotionSourceGroup", 8 }
		};

		virtualFunctionPattern["Class CoreUObject.Object"] = {
			{ "\x45\x33\xF6\x4D\x8B\xE0", "xxxxxx", 71, R"(	inline void ProcessEvent(class UFunction* function, void* parms)
	{
		return GetVFunction<void(*)(UObject*, class UFunction*, void*)>(this, %d)(this, function, parms);
	})"
    ,71 }
		};
		virtualFunctionPattern["Class CoreUObject.Class"] = {
			{ "\x4C\x8B\xDC\x57\x48\x81\xEC", "xxxxxxx", 107, R"(	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject*(*)(UClass*)>(this, %d)(this);
	})"
    ,107 }
		};

		predefinedMembers["Class CoreUObject.Object"] = {
			{ "void*", "Vtable" },
			{ "int32_t", "ObjectFlags" },
			{ "int32_t", "InternalIndex" },
			{ "class UClass*", "Class" },
			{ "FName", "Name" },
			{ "class UObject*", "Outer" }
		};
		predefinedStaticMembers["Class CoreUObject.Object"] = {
			{ "FUObjectArray*", "GObjects" }
		};
		predefinedMembers["Class CoreUObject.Field"] = {
			{ "class UField*", "Next" }
		};
		predefinedMembers["Class CoreUObject.Struct"] = {
			{ "class UStruct*", "SuperField" },
			{ "class UField*", "Children" },
			{ "int32_t", "PropertySize" },
			{ "int32_t", "MinAlignment" },
			{ "unsigned char", "UnknownData0x0048[0x28]" }
		};
		predefinedMembers["Class CoreUObject.Function"] = {
			{ "int32_t", "FunctionFlags" },
			//{ "int16_t", "RepOffset" },
			{ "int8_t", "NumParms" },
			{ "int16_t", "ParmsSize" },
			{ "int16_t", "ReturnValueOffset" },
			{ "int16_t", "RPCId" },
			{ "int16_t", "RPCResponseId" },
			//{ "class UProperty*", "FirstPropertyToInit" },
			//{ "class UFunction*", "EventGraphFunction" },
			//{ "int32_t", "EventGraphCallOffset" },
            { "unsigned char", "UnknownData0x00c[0xC]" },
			{ "void*", "Func" }
		};

		predefinedMethods["ScriptStruct CoreUObject.Vector2D"] = {
			PredefinedMethod::Inline(R"(	inline FVector2D()
		: X(0), Y(0)
	{ })"),
			PredefinedMethod::Inline(R"(	inline FVector2D(float x, float y)
		: X(x),
		  Y(y)
	{ })")
		};
		predefinedMethods["ScriptStruct CoreUObject.LinearColor"] = {
			PredefinedMethod::Inline(R"(	inline FLinearColor()
		: R(0), G(0), B(0), A(0)
	{ })"),
			PredefinedMethod::Inline(R"(	inline FLinearColor(float r, float g, float b, float a)
		: R(r),
		  G(g),
		  B(b),
		  A(a)
	{ })")
		};

		predefinedMethods["Class CoreUObject.Object"] = {
			PredefinedMethod::Inline(R"(	static inline TUObjectArray& GetGlobalObjects()
	{
		return GObjects->ObjObjects;
	})"),
			PredefinedMethod::Default("std::string GetName() const", R"(std::string UObject::GetName() const
{
	std::string name(Name.GetName());
	if (Name.Number > 0)
	{
		name += '_' + std::to_string(Name.Number);
	}

	auto pos = name.rfind('/');
	if (pos == std::string::npos)
	{
		return name;
	}
	
	return name.substr(pos + 1);
})"),
			PredefinedMethod::Default("std::string GetFullName() const", R"(std::string UObject::GetFullName() const
{
	std::string name;

	if (Class != nullptr)
	{
		std::string temp;
		for (auto p = Outer; p; p = p->Outer)
		{
			temp = p->GetName() + "." + temp;
		}

		name = Class->GetName();
		name += " ";
		name += temp;
		name += GetName();
	}

	return name;
})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GetGlobalObjects().Num(); ++i)
		{
			auto object = GetGlobalObjects().GetByIndex(i);
	
			if (object == nullptr)
			{
				continue;
			}
	
			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	})"),
			PredefinedMethod::Inline(R"(	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* GetObjectCasted(std::size_t index)
	{
		return static_cast<T*>(GetGlobalObjects().GetByIndex(index));
	})"),
			PredefinedMethod::Default("bool IsA(UClass* cmp) const", R"(bool UObject::IsA(UClass* cmp) const
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperField))
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
})")
		};
		predefinedMethods["Class CoreUObject.Class"] = {
			PredefinedMethod::Inline(R"(	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	})")
		};

		return true;
	}

	std::string GetGameName() const override
	{
		return  gameFullName;
	}

	std::string GetGameNameShort() const override
	{
		return gameShortName;
	}

	std::string GetGameVersion() const override
	{
		return  gameVersion;
	}

	std::string GetNamespaceName() const override
	{
		return "SDK";
	}

	std::vector<std::string> GetIncludes() const override
	{
		return { };
	}

	std::string GetBasicDeclarations() const override
	{
		return R"(template<typename Fn>
inline Fn GetVFunction(void *thiz, int idx)
{
    auto VTable = *reinterpret_cast<void***>(const_cast<void*>(thiz));
    return (Fn)(VTable[idx]);
}

class UObject;

class FUObjectItem
{
public:
    UObject* Object;
    int32_t Flags;
    int32_t ClusterIndex;
    int32_t SerialNumber;

    enum class ObjectFlags : int32_t
    {
        None = 0,
        Native = 1 << 25,
        Async = 1 << 26,
        AsyncLoading = 1 << 27,
        Unreachable = 1 << 28,
        PendingKill = 1 << 29,
        RootSet = 1 << 30,
        NoStrongReference = 1 << 31
    };

    inline bool IsUnreachable() const
    {
        return !!(Flags & static_cast<std::underlying_type_t<ObjectFlags>>(ObjectFlags::Unreachable));
    }
    inline bool IsPendingKill() const
    {
        return !!(Flags & static_cast<std::underlying_type_t<ObjectFlags>>(ObjectFlags::PendingKill));
    }
};

class TUObjectArray
{
public:
    inline int32_t Num() const
    {
        return NumElements;
    }

    inline UObject* GetByIndex(int32_t index) const
    {
        return Objects[index].Object;
    }

    inline FUObjectItem* GetItemByIndex(int32_t index) const
    {
        if (index < NumElements)
        {
            return &Objects[index];
        }
        return nullptr;
    }

private:
    FUObjectItem* Objects;
    int32_t MaxElements;
    int32_t NumElements;
};

class FUObjectArray
{
public:
    int32_t ObjFirstGCIndex;
    int32_t ObjLastNonGCIndex;
    int32_t MaxObjectsNotConsideredByGC;
    int32_t OpenForDisregardForGC;
    TUObjectArray ObjObjects;
};

template<class T>
struct TArray
{
    friend struct FString;

public:
    inline TArray()
    {
        Data = nullptr;
        Count = Max = 0;
    };

    inline int Num() const
    {
        return Count;
    };

    inline T& operator[](int i)
    {
        return Data[i];
    };

    inline const T& operator[](int i) const
    {
        return Data[i];
    };

    inline bool IsValidIndex(int i) const
    {
        return i < Num();
    }

private:
    T* Data;
    int32_t Count;
    int32_t Max;
};

class FNameEntry
{
public:
    static const auto NAME_WIDE_MASK = 0x1;
    static const auto NAME_INDEX_SHIFT = 1;

    int32_t Index;
#if defined(__LP64__)
    char pad[0x8];
#else
    char pad[0x4];
#endif

    union
    {
        char AnsiName[1024];
        wchar_t WideName[1024];
    };

    inline const int32_t GetIndex() const
    {
        return Index >> NAME_INDEX_SHIFT;
    }

    inline bool IsWide() const
    {
        return Index & NAME_WIDE_MASK;
    }

    inline const char* GetAnsiName() const
    {
        return AnsiName;
    }

    inline const wchar_t* GetWideName() const
    {
        return WideName;
    }
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
    inline size_t Num() const
    {
        return NumElements;
    }

    inline bool IsValidIndex(int32_t index) const
    {
        return index < Num() && index > 0;
    }

    inline ElementType const* const& operator[](int32_t index) const
    {
        return *GetItemPtr(index);
    }

private:
    inline ElementType const* const* GetItemPtr(int32_t Index) const
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
    int32_t NumElements;
    int32_t NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

struct FName
{
    union
    {
        struct
        {
            int32_t ComparisonIndex;
            int32_t Number;
        };
    };

    inline FName()
        : ComparisonIndex(0),
          Number(0)
    {
    };

    inline FName(int32_t i)
        : ComparisonIndex(i),
          Number(0)
    {
    };

    FName(const char* nameToFind)
        : ComparisonIndex(0),
          Number(0)
    {
        static std::unordered_set<int> cache;

        for (auto i : cache)
        {
            if (!std::strcmp(GetNames()[i]->GetAnsiName(), nameToFind))
            {
                ComparisonIndex = i;

                return;
            }
        }

        for (auto i = 0; i < GetNames().Num(); ++i)
        {
            if (GetNames()[i] != nullptr)
            {
                if (!std::strcmp(GetNames()[i]->GetAnsiName(), nameToFind))
                {
                    cache.insert(i);

                    ComparisonIndex = i;

                    return;
                }
            }
        }
    };

    static TNameEntryArray *GNames;
    static inline TNameEntryArray& GetNames()
    {
        return *GNames;
    };

    inline const char* GetName() const
    {
        return GetNames()[ComparisonIndex]->GetAnsiName();
    };

    inline bool operator==(const FName &other) const
    {
        return ComparisonIndex == other.ComparisonIndex;
    };
};

struct FString : private TArray<unsigned short>
{
    inline FString()
    {
    }

    FString(const std::wstring s)
    {
        Max = Count = !s.empty() ? (s.length() * 2) + 1 : 0;
        if (Count)
        {
            Data = (unsigned short *)(s.data());
        }
    }

    FString(const wchar_t *s) : FString(std::wstring(s)) {
    }

    FString(const wchar_t *s, int len) : FString(std::wstring(s, s + len)) {
    }

    FString(const std::string s) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring ws = converter.from_bytes(s);

        Max = Count = !ws.empty() ? (ws.length() * 2) + 1 : 0;
        if (Count)
        {
            Data = (unsigned short *)(ws.data());
        }
    }

    FString(const char *s) : FString(std::string(s)) {
    }

    FString(const char *s, int len) : FString(std::string(s, s + len)) {
    }

    inline bool IsValid() const
    {
        return Data != nullptr;
    }

    inline const wchar_t* ToWString() const
    {
        wchar_t *output = new wchar_t[Count + 1];

        for (int i = 0; i < Count; i++) {
            const char16_t uc = Data[i];
            if (uc - 0xd800u >= 2048u) {
                output[i] = uc;
            } else {
                if ((uc & 0xfffffc00) == 0xd800 && (uc & 0xfffffc00) == 0xdc00)
                    output[i] = (uc << 10) + Data[i] - 0x35fdc00;
                else
                    output[i] = L'?';
            }
        }

        output[Count] = 0;
        return output;
    }

    inline const char* ToString() const
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
        return convert.to_bytes(std::u16string(Data, Data + Count)).c_str();
    }
};

template<class TEnum>
class TEnumAsByte
{
public:
    inline TEnumAsByte()
    {
    }

    inline TEnumAsByte(TEnum _value)
        : value(static_cast<uint8_t>(_value))
    {
    }

    explicit inline TEnumAsByte(int32_t _value)
        : value(static_cast<uint8_t>(_value))
    {
    }

    explicit inline TEnumAsByte(uint8_t _value)
        : value(_value)
    {
    }

    inline operator TEnum() const
    {
        return (TEnum)value;
    }

    inline TEnum GetValue() const
    {
        return (TEnum)value;
    }

private:
    uint8_t value;
};

class FScriptInterface
{
private:
    UObject* ObjectPointer;
    void* InterfacePointer;

public:
    inline UObject* GetObject() const
    {
        return ObjectPointer;
    }

    inline UObject*& GetObjectRef()
    {
        return ObjectPointer;
    }

    inline void* GetInterface() const
    {
        return ObjectPointer != nullptr ? InterfacePointer : nullptr;
    }
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
    inline InterfaceType* operator->() const
    {
        return (InterfaceType*)GetInterface();
    }

    inline InterfaceType& operator*() const
    {
        return *((InterfaceType*)GetInterface());
    }

    inline operator bool() const
    {
        return GetInterface() != nullptr;
    }
};

struct FText
{
#if defined(__LP64__)
    char pad[24];
#else
    char pad[12];
#endif
};

struct FScriptDelegate
{
    char pad[16];
};

struct FScriptMulticastDelegate
{
#if defined(__LP64__)
    char pad[16];
#else
    char pad[12];
#endif
};

template<typename Key, typename Value>
class TMap
{
#if defined(__LP64__)
    char pad[80];
#else
    char pad[60];
#endif
};

struct FWeakObjectPtr
{
public:
    inline bool SerialNumbersMatch(FUObjectItem* ObjectItem) const
    {
        return ObjectItem->SerialNumber == ObjectSerialNumber;
    }

    bool IsValid() const;

    UObject* Get() const;

    int32_t ObjectIndex;
    int32_t ObjectSerialNumber;
};

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : private TWeakObjectPtrBase
{
public:
    inline T* Get() const
    {
        return (T*)TWeakObjectPtrBase::Get();
    }

    inline T& operator*() const
    {
        return *Get();
    }

    inline T* operator->() const
    {
        return Get();
    }

    inline bool IsValid() const
    {
        return TWeakObjectPtrBase::IsValid();
    }
};

template<class T, class TBASE>
class TAutoPointer : public TBASE
{
public:
    inline operator T*() const
    {
        return TBASE::Get();
    }

    inline operator const T*() const
    {
        return (const T*)TBASE::Get();
    }

    explicit inline operator bool() const
    {
        return TBASE::Get() != nullptr;
    }
};

template<class T>
class TAutoWeakObjectPtr : public TAutoPointer<T, TWeakObjectPtr<T>>
{
public:
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
    FWeakObjectPtr WeakPtr;
    int32_t TagAtLastTest;
    TObjectID ObjectID;
};

struct FStringAssetReference_
{

};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference_>
{

};

template<typename ObjectType>
class TAssetPtr : FAssetPtr
{

};

struct FUniqueObjectGuid_
{

};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid_>
{

};

template<typename ObjectType>
class TLazyObjectPtr : FLazyObjectPtr
{

};)";
	}

	std::string GetBasicDefinitions() const override
	{
		return R"(TNameEntryArray* FName::GNames = nullptr;
FUObjectArray* UObject::GUObjectArray = nullptr;
//---------------------------------------------------------------------------
bool FWeakObjectPtr::IsValid() const
{
    if (ObjectSerialNumber == 0)
    {
        return false;
    }
    if (ObjectIndex < 0)
    {
        return false;
    }
    auto ObjectItem = UObject::GetGlobalObjects().GetItemByIndex(ObjectIndex);
    if (!ObjectItem)
    {
        return false;
    }
    if (!SerialNumbersMatch(ObjectItem))
    {
        return false;
    }
    return !(ObjectItem->IsUnreachable() || ObjectItem->IsPendingKill());
}
//---------------------------------------------------------------------------
UObject* FWeakObjectPtr::Get() const
{
    if (IsValid())
    {
        auto ObjectItem = UObject::GetGlobalObjects().GetItemByIndex(ObjectIndex);
        if (ObjectItem)
        {
            return ObjectItem->Object;
        }
    }
    return nullptr;
}
//---------------------------------------------------------------------------)";
	}
};

Generator _generator;
IGenerator* generator = &_generator;
