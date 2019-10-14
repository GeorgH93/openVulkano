#pragma once
#include <vector>
#include <string>
#include <set>
#include <algorithm>

namespace openVulkanoCpp
{
	class Utils
	{
	public:
		static std::vector<const char*> toCString(const std::vector<std::string>& values)
		{
			std::vector<const char*> result;
			result.reserve(values.size());
			for (const auto& string : values) {
				result.push_back(string.c_str());
			}
			return result;
		}

		static std::vector<const char*> toCString(const std::set<std::string>& values)
		{
			std::vector<const char*> result;
			result.reserve(values.size());
			for (const auto& string : values) {
				result.push_back(string.c_str());
			}
			return result;
		}

		template <typename T>
		static bool Contains(std::vector<T>& vec, const T& element)
		{
			return (std::find(vec.begin(), vec.end(), element) != vec.end());
		}

		template <typename T>
		static void Remove(std::vector<T>& vec, const T& element)
		{
			vec.erase(std::remove(vec.begin(), vec.end(), element), vec.end());
		}

		template <typename Enumeration>
		static auto EnumAsInt(Enumeration const value)
			-> typename std::underlying_type<Enumeration>::type
		{
			return static_cast<typename std::underlying_type<Enumeration>::type>(value);
		}

		static bool MatchesAnyElementWise(const glm::vec3& a, const glm::vec3& b)
		{
			return a.x == b.x || a.y == b.y || a.z == b.z;
		}
	};
}
