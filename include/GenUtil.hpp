#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <climits>
#include <glm/glm.hpp>

namespace LGenUtility
{
    extern std::fstream Log;
    // Returns the index of the given element in the given vector, or -1 if the element is not in that vector.
    template<typename T>
    std::ptrdiff_t VectorIndexOf(const std::vector<T>& vec, const T& elem)
    {
        std::ptrdiff_t result = -1;

        auto it = std::find(vec.begin(), vec.end(), elem);
        if (it != vec.end())
            result = it - vec.begin();

        return result;
    }

    // Returns whether the given element is contained in the given vector.
    template<typename T>
    std::ptrdiff_t VectorContains(const std::vector<T>& vec, const T& elem)
    {
        return VectorIndexOf(vec, elem) != -1;
    }

    // Returns whether the given element is contained in the given vector, with the index parameter set to the element's position.
    template<typename T>
    std::ptrdiff_t VectorContains(const std::vector<T>& vec, const T& elem, std::ptrdiff_t& index)
    {
        index = VectorIndexOf(vec, elem);
        return index != -1;
    }
    
    // Returns an endian-swapped version of the given value.
    // Copied from https://stackoverflow.com/a/4956493
    template <typename T>
    T SwapEndian(T u)
    {
        static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

        union
        {
            T u;
            unsigned char u8[sizeof(T)];
        } source, dest;

        source.u = u;

        for (size_t k = 0; k < sizeof(T); k++)
            dest.u8[k] = source.u8[sizeof(T) - k - 1];

        return dest.u;
    }

    inline size_t PadToBoundary(size_t original, size_t boundary)
    {
        return (original + (boundary - 1)) & ~(boundary - 1);
    }

    // Converts the given string from UTF-8 encoding to Shift-JIS.
    std::string Utf8ToSjis(const std::string& value);

    // Converts the given string from Shift-JIS encoding to UTF-8.
    std::string SjisToUtf8(const std::string& value);

    bool TriBoxIntersect(glm::vec3 tri[3], glm::vec3 boxCenter, glm::vec3 boxExtents);

}
