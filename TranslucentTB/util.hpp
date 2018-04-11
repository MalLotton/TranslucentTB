#pragma once
#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <string>
#include <vector>

class Util {

public:
	inline static void ToLower(std::wstring &data)
	{
		std::transform(data.begin(), data.end(), data.begin(), std::towlower);
	}

	inline static std::wstring Trim(const std::wstring& str)
	{
		size_t first = str.find_first_not_of(' ');

		if (first == std::wstring::npos)
		{
			return L"";
		}

		size_t last = str.find_last_not_of(' ');
		return str.substr(first, (last - first + 1));
	}

	inline static void QuoteSpaces(std::wstring &path)
	{
		if (path.find_first_of(' ') != std::wstring::npos)
		{
			path = L"\"" + path + L"\"";
		}
	}

	// For std::bind magic
	template<typename T>
	inline static void UpdateValue(T &toupdate, T newvalue, unsigned int)
	{
		toupdate = newvalue;
	}

	inline static void InvertBool(bool &value, unsigned int)
	{
		value = !value;
	}

	static void EditFile(std::wstring file);
	static void PickColor(uint32_t &color);
	static void AddValuesToVectorByDelimiter(const wchar_t &delimiter, std::vector<std::wstring> &vector, std::wstring line);
	static bool IsStartVisible();
};