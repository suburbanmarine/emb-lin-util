/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Chronometer.hpp"

#include "Timespec_util.hpp"

bool Chronometer::get_time(std::chrono::nanoseconds* const out_time)
{
	timespec t0;
	if( ! get_time(&t0) )
	{
		return false;
	}

	if(out_time)
	{
		*out_time = Timespec_util::to_chrono<std::chrono::nanoseconds>(t0);
	}

	return true;
}
bool Chronometer::get_time(timespec*                 const out_time)
{
	int ret = clock_gettime(CLOCK_MONOTONIC, out_time);
	return ret == 0;
}
