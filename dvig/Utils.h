#pragma once

namespace utils {
	template<typename Type>
	void zero_memory(Type* memory) {
		std::memset(memory, 0, sizeof(Type));
	}
}
