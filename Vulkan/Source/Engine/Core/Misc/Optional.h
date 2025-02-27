#pragma once
#include "GenericPlatform/GenericPlatform.h"

enum class EInPlace : uint8 {InPlace};

template<typename T>
class TOptional
{
	using ElementType = T;
	using DataType = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
public:
	TOptional(const ElementType& Other)
	: bSet(true)
	{
		Emplace(Other);
	}

	TOptional()
	: bSet(false)
	{
	}

	template <typename... ArgTypes>
	explicit TOptional(ArgTypes&&... Args)
	{
		Emplace(std::forward<ArgTypes>(Args)...);
	}

	TOptional& operator = (const TOptional& Other)
	{
		if (&Other != this)
		{
			Reset();
			if (Other.bSet)
			{
				Emplace(*(ElementType*)(&Other.Value));
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
				Emplace(*(ElementType*)(&Other.Value));
			}
			bSet = true;
		}
		
		return *this;
	}
	
	TOptional& operator = (const ElementType& Other)
	{
		if (&Other != &Value)
		{
			Reset();
			bSet = true;
			Emplace(Other);
		}
		
		return *this;
	}

	TOptional& operator = (const ElementType&& Other)
	{
		if (&Other != &Value)
		{
			Reset();
			bSet = true;
			Emplace(Other);
		}
		
		return *this;
	}

	ElementType& operator*() 
	{
		check(IsSet());
		return *(ElementType*)(&Value);
	}

	const ElementType& operator*() const
	{
		check(IsSet());
		return *(ElementType*)(&Value);
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
			reinterpret_cast<ElementType*>(&Value)->~ElementType();
		}
	}
	
private:
	template <typename... ArgsType>
	ElementType& Emplace(ArgsType&&... Args)
	{
		Reset();
		bSet = true;
		new (&Value) ElementType(std::forward<ArgsType>(Args)...);
	}
	
private:
	bool bSet = false;
	DataType Value;
};


