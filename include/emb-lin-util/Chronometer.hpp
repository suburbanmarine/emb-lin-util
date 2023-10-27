/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <chrono>

class Chronometer
{
public:

	// Returns a timestamp from CLOCK_MONOTONIC
	static bool get_time(std::chrono::nanoseconds* const out_time);
	static bool get_time(timespec*                 const out_time);
	
};
