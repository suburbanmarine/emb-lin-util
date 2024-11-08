/**
 * This file is part of emb-lin-util, a collection of utility code for embedded linux.
 * 
 * This software is distrubuted in the hope it will be useful, but without any warranty, including the implied warrranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See LICENSE.txt for details.
 * 
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2024 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the LGPL-3.0 license. See LICENSE.txt for details.
 * SPDX-License-Identifier: LGPL-3.0-only
*/

#pragma once

#include <emb-lin-util/File_util.hpp>
#include "emb-lin-util/Zlib_util.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <deque>

#include <cstdint>

template<typename T>
class JSON_CBOR_helper
{
public:

	static std::vector<uint8_t> to_cbor(const T& x)
	{
		nlohmann::json j;
		to_json(j, x);

		std::vector<uint8_t> v;
		nlohmann::json::to_cbor(j, v);

		return v;
	}

	static void from_cbor(const std::vector<uint8_t>& v, T& out_x)
	{
		const nlohmann::json j(nlohmann::json::from_cbor(v));
		from_json(j, out_x);
	}

	static void from_cbor(const std::deque<uint8_t>& v, T& out_x)
	{
		const nlohmann::json j(nlohmann::json::from_cbor(v));
		from_json(j, out_x);
	}

	virtual std::vector<uint8_t> to_cbor() const
	{
		nlohmann::json j;
		to_json(j, *dynamic_cast<T const * const>(this));

		std::vector<uint8_t> v;
		nlohmann::json::to_cbor(j, v);

		return v;
	}

	virtual bool to_cbor(const bool compress, std::vector<uint8_t>& out_v) const
	{
		nlohmann::json j;
		to_json(j, *dynamic_cast<T const * const>(this));

		nlohmann::json::to_cbor(j, out_v);

		if(compress)
		{
			std::vector<uint8_t> data_comp;

			Zlib_util zlib;
			if( ! zlib.deflate_oneshot(out_v, &data_comp) )
			{
				return false;
			}

			out_v = data_comp;
		}

		return true;
	}

	virtual void from_cbor(const std::vector<uint8_t>& v)
	{
		from_cbor(v, *dynamic_cast<T*>(this));
	}

	virtual void from_cbor(const std::deque<uint8_t>& v)
	{
		from_cbor(v, *dynamic_cast<T*>(this));
	}

	virtual bool from_cbor(const bool decompress, std::vector<uint8_t>& v)
	{
		if(decompress)
		{
			std::deque<uint8_t> data_decomp;

			Zlib_util zlib;
			if( ! zlib.inflate_oneshot(v, &data_decomp) )
			{
				return false;
			}

			from_cbor(data_decomp, *dynamic_cast<T*>(this));
		}
		else
		{
			from_cbor(v, *dynamic_cast<T*>(this));
		}

		return true;
	}

	virtual bool read_cbor(const std::string& p, const bool decompress)
	{
		std::vector<uint8_t> file_data;
		if( ! File_util::readSmallFile(p, &file_data) )
		{
			return false;
		}

		if(decompress)
		{
			std::deque<uint8_t> file_data_dec;

			Zlib_util zlib;
			if( ! zlib.inflate_oneshot(file_data, &file_data_dec) )
			{
				return false;
			}

			from_cbor(file_data_dec);
		}
		else
		{
			from_cbor(file_data);
		}

		return true;
	}
	virtual bool write_cbor(const std::string& p, const bool compress) const
	{
		std::vector<uint8_t> file_data = to_cbor();

		bool ret = false;

		if(compress)
		{
			std::vector<uint8_t> file_data_comp;

			Zlib_util zlib;
			if( ! zlib.deflate_oneshot(file_data, &file_data_comp) )
			{
				return false;
			}

			ret = File_util::writeSmallFile(p, file_data_comp);
		}
		else
		{
			ret = File_util::writeSmallFile(p, file_data);
		}

		return ret;
	}

	virtual std::string to_json_string() const
	{
		nlohmann::json j;
		to_json(j, *dynamic_cast<T const * const>(this));

		return j.dump();
	}

	virtual std::string to_json_string_pretty() const
	{
		nlohmann::json j;
		to_json(j, *dynamic_cast<T const * const>(this));

		return j.dump(4);
	}

	virtual bool read_json(const std::string& p)
	{
		std::vector<uint8_t> file_data;
		if( ! File_util::readSmallFile(p, &file_data) )
		{
			return false;
		}

		const nlohmann::json j = nlohmann::json::parse(file_data);
		from_json(j, *dynamic_cast<T*>(this));

		return true;
	}
	virtual bool write_json(const std::string& p) const
	{
		return File_util::writeSmallFile(p, to_json_string());
	}

	virtual bool write_json_pretty(const std::string& p) const
	{
		return File_util::writeSmallFile(p, to_json_string_pretty());
	}
};
