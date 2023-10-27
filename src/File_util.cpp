/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "emb-lin-util/File_util.hpp"

#include <boost/lexical_cast.hpp>

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <fstream>

bool File_util::readSmallFile(char const * const filename, std::string* const out_value)
{
	if( ! out_value )
	{
		return false;
	}

	std::ifstream file(filename);
	if (!file) {
	    SPDLOG_WARN("Could not open the file: {:s}", filename);
	    return false;
	}

	std::getline(file, *out_value);

	if (!file) {
		SPDLOG_WARN("Could not read the file: {:s}", filename);
		return false;
	}

	return true;
}

bool File_util::readSmallFileToInt(char const * const filename, int* const out_value)
{
	if( ! out_value )
	{
		return false;
	}

	std::string file_data;
	if( ! readSmallFile(filename, &file_data) )
	{
		return false;
	}

	try
	{
		*out_value = boost::lexical_cast<int>(file_data);
	}
	catch(const std::exception& e)
	{
		SPDLOG_WARN("Could not parse an integer from the file: {:s}", filename);
		return false;
	}

	return true;
}

std::string File_util::getenv_or_empty(char const * const env_name)
{
	if(env_name)
	{
		char* str = getenv(env_name);
		if(str)
		{
			return str;
		}
	}

	return std::string();
}

std::string File_util::getenv_or_str(char const * const env_name, const std::string_view& def_str)
{
	if(env_name)
	{
		char* str = getenv(env_name);
		if(str)
		{
			return str;
		}
	}

	return std::string(def_str);	
}
