#include "emb-lin-util.hpp"
#include <spdlog/async.h>

namespace emblinutil
{
	std::shared_ptr<spdlog::logger> create_logger(const std::string& logger_name, const std::vector<spdlog::sink_ptr>& sinks)
	{
		auto logger = spdlog::get(logger_name);

		if( ! logger )
		{
			logger = std::make_shared<spdlog::async_logger>(logger_name, std::begin(sinks), std::end(sinks), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
			logger->set_level(spdlog::level::debug);
			spdlog::set_default_logger( logger );
		}

		return logger;
	}

	void register_logger(const std::shared_ptr<spdlog::logger>& logger)
	{
		spdlog::register_logger(logger);
		spdlog::set_default_logger(logger);
	}
}

