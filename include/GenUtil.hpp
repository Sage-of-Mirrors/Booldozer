#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>

namespace LGenUtility
{
	// Returns the index of the given element in the given vector, or -1 if the element is not in that vector.
	template<typename T>
	ptrdiff_t VectorIndexOf(std::vector<std::shared_ptr<T>> vec, std::shared_ptr<T> elem)
	{
		ptrdiff_t result = -1;

		auto it = std::find(vec.begin(), vec.end(), elem);
		if (it != vec.end())
			result = it - vec.begin();

		return result;
	}

	// Returns a string with the given formatting.
	template<typename ... T>
	std::string Format(T && ... x)
	{
		std::stringstream buff;
		(buff << ... << x);

		return buff.str();
	}
}
