#pragma once
#include "GenericPlatform/GenericPlatform.h"
#include "Misc/AssertionMacros.h"


enum class EInPlace : uint8 {InPlace};

template<typename T>
class TOptional
{
	using ElementType = T;
	using DataType = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
public:
	explicit TOptional(const TOptional& Other)
	: bSet(Other.bSet)
	{
		new(&Data) ElementType(*(const ElementType*)&Other.Data);
	}
	
	explicit TOptional(const ElementType& Other)
	: bSet(true)
	{
		Emplace(Other);
	}

	TOptional()
	: bSet(false)
	{
	}



	TOptional& operator = (const TOptional& Other)
	{
		if (&Other != this)
		{
			Reset();
			if (Other.bSet)
			{
				Emplace(*Other.GetValuePtr());
				bSet = true;
			}
		}
		
		return *this;
	}

	TOptional& operator = (const TOptional&& Other)
	{
		if (&Other != this)
		{
			if (Other.bSet)
			{
				Emplace(*Other.GetValuePtr());
			}
			bSet = true;
		}
		
		return *this;
	}
	
	TOptional& operator = (const ElementType& Other)
	{
		if (&Other != GetValuePtr())
		{
			Reset();
			bSet = true;
			Emplace(Other);
		}
		
		return *this;
	}

	TOptional& operator = (const ElementType&& Other)
	{
		if (&Other != GetValuePtr())
		{
			Reset();
			bSet = true;
			Emplace(Other);
		}
		
		return *this;
	}

	bool operator == (const TOptional& Other) const
	{
		check(IsSet() && Other.IsSet())
		return GetValue() == Other.GetValue();
	}

	bool operator != (const TOptional& Other) const
	{
		check(IsSet() && Other.IsSet())
		return GetValue() != Other.GetValue();
	}

	ElementType& operator*() 
	{
		check(IsSet());
		return *GetValuePtr();
	}

	ElementType& ValueRef() 
	{
		check(IsSet());
		return *GetValuePtr();
	}

	ElementType GetValue() 
	{
		check(IsSet());
		return *GetValuePtr();
	}

	ElementType GetValue() const
	{
		check(IsSet());
		return *GetValuePtr();
	}

	const ElementType& operator*() const
	{
		check(IsSet());
		return *GetValuePtr();
	}
	
	inline explicit operator bool() const
	{
		return IsSet();
	}

	bool IsSet() const
	{
		return bSet;
	}

	void Reset()
	{
		if (bSet)
		{
			bSet = false;
			GetValuePtr()->~ElementType();
		}
	}
	
private:
	template <typename... ArgsType>
	void Emplace(ArgsType&&... Args)
	{
		Reset();
		bSet = true;
		new (&Data) ElementType(std::forward<ArgsType>(Args)...);
	}

	ElementType* GetValuePtr() const
	{
		return (ElementType*)(&Data);
	}
	
	ElementType* GetValuePtr()
	{
		return (ElementType*)(&Data);
	}
	
private:
	bool bSet = false;
	DataType Data;
};


