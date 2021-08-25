#pragma once

#include <vector>
#include <algorithm>
#include <memory>

namespace LGenUtility
{
	// Returns the index of the given element in the given vector, or -1 if the element is not in that vector.
	template<typename T>
	int32_t VectorIndexOf(std::vector<std::shared_ptr<T>> vec, std::shared_ptr<T> elem)
	{
		int32_t result = -1;

		auto it = std::find(vec.begin(), vec.end(), elem);
		if (it != vec.end())
			result = it - vec.begin();

		return result;
	}
}
