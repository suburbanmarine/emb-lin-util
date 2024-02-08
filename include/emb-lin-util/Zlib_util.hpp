#pragma once

#include <functional>
#include <vector>
#include <deque>

#include <cstdint>
#include <cstddef>

class Zlib_util
{
public:
	typedef std::function<bool(uint8_t const * const ptr, const size_t len)> Block_callback;
	
	Zlib_util();
	virtual ~Zlib_util();

	bool init();

	// out_deflate_data will be resized to fit using deflateBound
	bool deflate_oneshot(std::vector<uint8_t>& in_data, std::vector<uint8_t>* const out_deflate_data);
	
	bool inflate_oneshot(std::vector<uint8_t>& in_data, std::vector<uint8_t>* const out_inflate_data);
	bool inflate_oneshot(std::vector<uint8_t>& in_data, std::deque<uint8_t>* const out_inflate_data);

	// STL container
	bool deflate(std::vector<uint8_t>& in_data, const Block_callback& cb)
	{
		return deflate(in_data.data(), in_data.size(), cb);
	}
	bool inflate(std::vector<uint8_t>& in_data, const Block_callback& cb)
	{
		return inflate(in_data.data(), in_data.size(), cb);
	}

	bool deflate(uint8_t* in_data, const size_t in_data_len, const Block_callback& cb);
	bool inflate(uint8_t* in_data, const size_t in_data_len, const Block_callback& cb);
protected:
	// chunk size for callback deflate/inflate
	const size_t deflate_block_size = 64*1024;
	const size_t inflate_block_size = 64*1024;
	const bool gzip_mode = false;
	const int mem_level  = 8;
};