#pragma once

#include <emb-lin-util/File_util.hpp>

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

	virtual bool read(const std::string& p)
	{
		std::vector<uint8_t> v;

		if( ! File_util::readSmallFile(p, &v) )
		{
			return false;
		}

		from_cbor(v, *dynamic_cast<T*>(this));

		return true;
	}
	virtual bool write(const std::string& p) const
	{
		return File_util::writeSmallFile(p, to_cbor(*dynamic_cast<T const * const>(this)));
	}
};
