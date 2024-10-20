/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "emb-lin-util/emb-lin-util.hpp"

namespace emblinutil
{
	std::shared_ptr<spdlog::logger> create_logger(const std::string& logger_name, const std::vector<spdlog::sink_ptr>& sinks)
	{
		auto logger = spdlog::get(logger_name);

		if( ! logger )
		{
			logger = std::make_shared<spdlog::logger>(logger_name, std::begin(sinks), std::end(sinks));
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

