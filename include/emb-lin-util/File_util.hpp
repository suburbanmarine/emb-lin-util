/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <string>
#include <string_view>
#include <vector>

class File_util
{
public:


	//read first line from file
	static bool readSmallFile(char const * const filename, std::string* const out_value);
	static bool readSmallFile(const std::string& filename, std::string* const out_value)
	{
		return readSmallFile(filename.c_str(), out_value);
	}

	//read first line from file and convert to int
	static bool readSmallFileToInt(char const * const filename, int* const out_value);
	static bool readSmallFileToInt(const std::string& filename, int* const out_value)
	{
		return readSmallFileToInt(filename.c_str(), out_value);
	}

	//read up to max_to_read B from a file
	static bool readSmallFile(const std::string& filename, std::vector<uint8_t>* const out_value);
	static bool readSmallFile(const std::string& filename, const ssize_t max_to_read, std::vector<uint8_t>* const out_value);

	static bool writeSmallFile(const std::string& filename, std::vector<uint8_t> const value);

	static std::string getenv_or_empty(char const * const env_name);
	static std::string getenv_or_str(char const * const env_name, const std::string_view& def_str);

protected:
};
