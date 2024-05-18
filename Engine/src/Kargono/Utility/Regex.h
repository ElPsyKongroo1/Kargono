#pragma once

#include <regex>
#include <string>

namespace Kargono::Utility
{
	// Good Agnostic Regex Grammar Video: https://www.youtube.com/watch?v=sa-TUpSx1JA
	// Regular Expressions in C++: https://en.cppreference.com/w/cpp/regex
	// Grammar Reference: https://learn.microsoft.com/en-us/cpp/standard-library/regular-expressions-cpp?view=msvc-170#grammarsummary
	class Regex
	{
	public:
		static uint64_t GetMatchCount(const std::string& inputText, const std::string& regexExpression, bool caseSensitive = true)
		{
			std::regex_constants::syntax_option_type flags {};
			if (!caseSensitive)
			{
				flags |= std::regex_constants::icase;
			}
			std::regex regex {regexExpression, flags};
			std::sregex_iterator begin(inputText.begin(), inputText.end(), regex);
			std::sregex_iterator end = std::sregex_iterator();
			return std::distance(begin, end);
		}
		static bool GetMatchSuccess(const std::string& inputText, const std::string& regexExpression, bool caseSensitive = true)
		{
			std::regex_constants::syntax_option_type flags {};
			if (!caseSensitive)
			{
				flags |= std::regex_constants::icase;
			}
			std::regex regex {regexExpression, flags};
			return std::regex_search(inputText, regex);
		}
		static std::string ReplaceMatches(const std::string& inputText, const std::string& regexExpression, const std::string& replacementText)
		{
			return std::regex_replace(inputText, std::regex(regexExpression), replacementText);
		}
	};
}
