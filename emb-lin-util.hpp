#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace emblinutil
{
	// NOT MT safe - call before using library
	std::shared_ptr<spdlog::logger> setup_logger(const std::string& logger_name, const std::vector<spdlog::sink_ptr>& sinks);

	// NOT MT safe - call before using library
	void register_logger(const std::shared_ptr<spdlog::logger>& logger);
}
