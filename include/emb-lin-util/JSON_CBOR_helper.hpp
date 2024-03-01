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

	virtual void from_cbor(const std::vector<uint8_t>& v)
	{
		from_cbor(v, *dynamic_cast<T*>(this));
	}

	virtual void from_cbor(const std::deque<uint8_t>& v)
	{
		from_cbor(v, *dynamic_cast<T*>(this));
	}

	virtual bool read_cbor(const std::string& p, const bool gzip)
	{
		std::vector<uint8_t> file_data;
		if( ! File_util::readSmallFile(p, &file_data) )
		{
			return false;
		}

		if(gzip)
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
	virtual bool write_cbor(const std::string& p, const bool gzip) const
	{
		std::vector<uint8_t> file_data = to_cbor();

		bool ret = false;

		if(gzip)
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
		nlohmann::json j;
		to_json(j, *dynamic_cast<T const * const>(this));

		return File_util::writeSmallFile(p, j.dump());
	}
};
