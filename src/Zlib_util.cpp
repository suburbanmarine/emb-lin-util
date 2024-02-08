#include <emb-lin-util//Zlib_util.hpp>

#include <spdlog/spdlog.h>

#include <zlib.h>

Zlib_util::Zlib_util()
{

}
Zlib_util::~Zlib_util()
{

}

bool Zlib_util::init()
{
	const char* ver = zlibVersion();

	if(ZLIB_VERSION[0] != ver[0])
	{
		return false;
	}

	return true;
}

bool Zlib_util::deflate_oneshot(std::vector<uint8_t>& in_data, std::vector<uint8_t>* const out_deflate_data)
{
	if( ! out_deflate_data )
	{
		return false;
	}

	std::shared_ptr<z_stream> stream(new z_stream, &::deflateEnd);
	memset(stream.get(), 0, sizeof(stream));

	stream->zalloc    = Z_NULL;
	stream->zfree     = Z_NULL;
	stream->next_in   = in_data.data();
	stream->avail_in  = in_data.size();
	stream->data_type = Z_BINARY;

	int windowBits = 15;
	if(gzip_mode)
	{
		windowBits += 16;
	}

	int ret = ::deflateInit2(stream.get(), Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, mem_level, Z_DEFAULT_STRATEGY);
	if(ret != Z_OK)
	{
		SPDLOG_WARN("deflateInit2 failed");
		return false;
	}

	stream->avail_out = ::deflateBound(stream.get(), in_data.size());
	out_deflate_data->resize(stream->avail_out);
	stream->next_out = out_deflate_data->data();

	ret = ::deflate(stream.get(), Z_FINISH);
	if(ret != Z_STREAM_END)
	{
		SPDLOG_WARN("deflate failed");
		return false;
	}

	//trim to size of deflated data
	out_deflate_data->resize(stream->avail_out);

	// ret = ::deflateEnd(&stream);
	stream.reset();

	return true;
}

bool Zlib_util::inflate_oneshot(std::vector<uint8_t>& in_data, std::vector<uint8_t>* const out_inflate_data)
{
	if( ! out_inflate_data )
	{
		return false;
	}

	auto inflate_cb = [&out_inflate_data](uint8_t const * const ptr, const size_t len)->bool
	{
		out_inflate_data->insert(out_inflate_data->end(), ptr, ptr+len);
		return true;
	};

	return inflate(in_data.data(), in_data.size(), inflate_cb);
}

bool Zlib_util::inflate_oneshot(std::vector<uint8_t>& in_data, std::deque<uint8_t>* const out_inflate_data)
{
	if( ! out_inflate_data )
	{
		return false;
	}

	auto inflate_cb = [&out_inflate_data](uint8_t const * const ptr, const size_t len)->bool
	{
		out_inflate_data->insert(out_inflate_data->end(), ptr, ptr+len);
		return true;
	};

	return inflate(in_data.data(), in_data.size(), inflate_cb);
}

bool Zlib_util::deflate(uint8_t* in_data, const size_t in_data_len, const Block_callback& cb)
{
	std::vector<uint8_t> deflate_block;
	deflate_block.resize(deflate_block_size);

	std::shared_ptr<z_stream> stream(new z_stream, &::deflateEnd);
	memset(stream.get(), 0, sizeof(stream));
	stream->zalloc    = Z_NULL;
	stream->zfree     = Z_NULL;
	stream->next_in   = in_data;
	stream->avail_in  = in_data_len;
	stream->next_out  = deflate_block.data();
	stream->avail_out = deflate_block.size();
	stream->data_type = Z_BINARY;

	int ret = ::deflateInit2(stream.get(), Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + 15, 8, Z_DEFAULT_STRATEGY);
	if(ret != Z_OK)
	{
		SPDLOG_WARN("deflateInit2 failed");
		return false;
	}

	do
	{
		// reset output pointer
		stream->next_out  = deflate_block.data();
		stream->avail_out = deflate_block.size();

		// Z_OK           -- keep going
		// Z_STREAM_END   -- done ok
		// Z_STREAM_ERROR -- fatal, stream inconsistant, usually app misuse of zlib
		// Z_BUF_ERROR    -- non fatal, need more space in buf
		// We call with Z_FINISH because all data is present in next_in/avail_in
		ret = ::deflate(stream.get(), Z_FINISH);
		switch(ret)
		{
			case Z_OK:
			case Z_STREAM_END:
			{
				// ok, check produced data
				const size_t num_to_consume = deflate_block.size() - stream->avail_out;
				if(num_to_consume > 0)
				{
					const bool cb_ok = cb(deflate_block.data(), num_to_consume);
					if( ! cb_ok )
					{
						SPDLOG_WARN("deflate cb failed");
						return false;					
					}
				}
				break;
			}
			case Z_STREAM_ERROR:
			case Z_BUF_ERROR:
			default:
			{
				SPDLOG_WARN("deflate failed: {:d}", ret);
				return false;
				break;			
			}
		}

	} while(ret == Z_OK);

	// ret = ::deflateEnd(&stream);
	stream.reset();

	return true;
}

bool Zlib_util::inflate(uint8_t* in_data, const size_t in_data_len, const Block_callback& cb)
{
	std::vector<uint8_t> inflate_block;
	inflate_block.resize(inflate_block_size);

	std::shared_ptr<z_stream> stream(new z_stream, &::inflateEnd);
	memset(stream.get(), 0, sizeof(stream));
	stream->zalloc    = Z_NULL;
	stream->zfree     = Z_NULL;
	stream->next_in   = in_data;
	stream->avail_in  = in_data_len;
	stream->next_out  = inflate_block.data();
	stream->avail_out = inflate_block.size();
	stream->data_type = Z_BINARY;

	int ret = ::inflateInit2(stream.get(), 32+15);
	if(ret != Z_OK)
	{
		SPDLOG_WARN("inflateInit2 failed");
		return false;
	}

	do
	{
		stream->next_out  = inflate_block.data();
		stream->avail_out = inflate_block.size();

		// Z_OK         -- keep going
		// Z_STREAM_END -- done ok
		// Z_NEED_DICT  -- fatal
		// Z_MEM_ERROR  -- fatal oom
		// Z_BUF_ERROR  -- non fatal, need more space in buf
		// Z_DATA_ERROR -- stream corrupt
		ret = ::inflate(stream.get(), Z_SYNC_FLUSH);
		switch(ret)
		{
			case Z_OK:
			case Z_STREAM_END:
			{
				// ok, check produced data
				const size_t num_to_consume = inflate_block.size() - stream->avail_out;
				if(num_to_consume > 0)
				{
					const bool cb_ok = cb(inflate_block.data(), num_to_consume);
					if( ! cb_ok )
					{
						SPDLOG_WARN("deflate cb failed");
						return false;					
					}
				}

				break;
			}
			case Z_NEED_DICT:
			case Z_MEM_ERROR:
			case Z_DATA_ERROR:
			case Z_BUF_ERROR:
			default:
			{
				SPDLOG_WARN("inflate failed: {:d}", ret);
				return false;
				break;			
			}
		}
	} while(ret == Z_OK);

	// ret = ::inflateEnd(&stream);
	stream.reset();

	return true;
}
