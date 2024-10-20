/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <chrono>

class Chronometer
{
public:

	// Returns a timestamp from CLOCK_MONOTONIC
	static bool get_time(std::chrono::nanoseconds* const out_time);
	static bool get_time(timespec*                 const out_time);
	static bool get_mono_time(timespec*            const out_time);
	
	// relative to 1970-01-01 00:00:00, may include leap seconds depending on local system configuration
	static bool get_real_time(timespec*             const out_time);
	// relative to 1958-01-01 00:00:00 without leap seconds
	static bool get_tai_time(timespec*              const out_time);
};
