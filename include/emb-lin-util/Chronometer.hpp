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
