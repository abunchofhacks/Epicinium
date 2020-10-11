//usr/bin/g++ src/build/quicktest.cpp -o bin/.quicktest && bin/.quicktest; exit

#include <cstdio>
#include <iostream>
#include <string>
#include <regex>


int main(int argc, char* argv[])
{
	std::string value = "\"989f-5a55d99843b80\"";
	if (argc >= 2)
	{
		value = argv[1];
	}

	std::regex etagregex = std::regex("^"
		// Strong etags start with a double quote.
		"\""
		// See above.
		"([!#-~\\x80-\\xFF]+)"
		// Entity tags end with a double quote.
		"\"$");

	std::smatch match;
	if (std::regex_match(value, match, etagregex))
	{
		std::cout << "Match: {{{" << match[1] << "}}}" << std::endl;
	}
	else
	{
		std::cout << "Invalid: {{{" << value << "}}}" << std::endl;
	}

	return 0;
}
