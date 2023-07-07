#pragma once
#include "pch.h"

namespace dvig {
	// Smart pointers
	template<typename Type>
	using Unique = std::unique_ptr<Type>;

	template<typename Type>
	using Shared = std::shared_ptr<Type>;

	template<typename Type>
	using Weak = std::weak_ptr<Type>;

	// TODO:
	/*
	template<typename Type>
	Ref<Type> MakeRef(Type&& value) {
		return std::make_shared<Type>(value);
	}
	*/
}