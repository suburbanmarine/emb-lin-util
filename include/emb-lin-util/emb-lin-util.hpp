/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace emblinutil
{
	// NOT MT safe - call before using library
	std::shared_ptr<spdlog::logger> create_logger(const std::string& logger_name, const std::vector<spdlog::sink_ptr>& sinks);

	// NOT MT safe - call before using library
	void register_logger(const std::shared_ptr<spdlog::logger>& logger);
}
