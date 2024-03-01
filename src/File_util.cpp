/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "emb-lin-util/File_util.hpp"

#include <boost/lexical_cast.hpp>

#include <spdlog/spdlog.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <cstdlib>
#include <fstream>
#include <limits>

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

bool File_util::readSmallFile(const std::string& filename, std::vector<uint8_t>* const out_value)
{
	return readSmallFile(filename, std::numeric_limits<ssize_t>::max(), out_value);
}

bool File_util::readSmallFile(const std::string& filename, const ssize_t max_to_read, std::vector<uint8_t>* const out_value)
{
	if(max_to_read < 0)
	{
		return false;
	}

	int fd = ::open(filename.c_str(), O_RDONLY | O_CLOEXEC);
	if(fd < 0)
	{
		return false;
	}

	const off_t file_len = lseek(fd, 0, SEEK_END);
	if(file_len < 0)
	{
		::close(fd);
		return false;
	}

	if(lseek(fd, 0, SEEK_SET) < 0)
	{
		::close(fd);
		return false;
	}

	const ssize_t num_to_read = std::min<ssize_t>(max_to_read, file_len);
	out_value->resize(num_to_read);

	ssize_t num_read = 0;
	do
	{
		ssize_t ret = read(fd, out_value->data() + num_read, num_to_read - num_read);
		if(ret < 0)
		{
			out_value->resize(num_read);
			
			::close(fd);
			return false;
		}

		num_read += ret;
	} while(num_read < num_to_read);
	
	out_value->resize(num_read);

	::close(fd);
	return true;
}

bool File_util::writeSmallFile(const std::string& filename, const std::vector<uint8_t>& value)
{
	if(value.size() > std::numeric_limits<ssize_t>::max())
	{
		return false;
	}

	int fd = ::open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, (S_IRUSR | S_IWUSR) | (S_IRGRP));
	if(fd < 0)
	{
		SPDLOG_WARN("File_util::writeSmallFile - Could not open {:s}", filename);
		return false;
	}

	{
		ssize_t ret = ::write(fd, value.data(), value.size());
		if(ret < 0) // we could retry, it might have been interrupted by a signal
		{
			SPDLOG_WARN("File_util::writeSmallFile error on write - {:s}", "");
			return false;
		}

		if(size_t(ret) != value.size()) // we could retry, it might have been interrupted by a signal
		{
			SPDLOG_WARN("File_util::writeSmallFile wrote unexpected amount");
			return false;
		}
	}

	int ret = ::close(fd);
	if(ret != 0)
	{
		SPDLOG_WARN("File_util::writeSmallFile close failed");
	}
	return ret == 0;
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
