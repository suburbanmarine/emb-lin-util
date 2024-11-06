/**
 * This file is part of emb-lin-util, a collection of utility code for embedded linux.
 * 
 * This software is distrubuted in the hope it will be useful, but without any warranty, including the implied warrranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See LICENSE.txt for details.
 * 
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the LGPL-3.0 license. See LICENSE.txt for details.
 * SPDX-License-Identifier: LGPL-3.0-only
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

