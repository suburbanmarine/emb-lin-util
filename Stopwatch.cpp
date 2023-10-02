/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Stopwatch.hpp"

#include "Chronometer.hpp"

Stopwatch::Stopwatch()
{
	t0 = std::chrono::nanoseconds::zero();
	t1 = std::chrono::nanoseconds::zero();
}

Stopwatch::~Stopwatch()
{
	
}

bool Stopwatch::start()
{
	return Chronometer::get_time(&t0);
}

bool Stopwatch::reset()
{
	return start();
}

bool Stopwatch::get_time(std::chrono::nanoseconds* const dt) const
{
	std::chrono::nanoseconds t_i;
	if( ! Chronometer::get_time(&t_i) )
	{
		return false;
	}

	if(dt)
	{
		*dt = t_i - t0;
	}

	return true;
}

void Stopwatch::set_alarm(const std::chrono::nanoseconds& dt)
{
	t1 = dt + t0;
}

bool Stopwatch::is_expired(bool* const is_exp)
{
	std::chrono::nanoseconds t_i;
	if( ! Chronometer::get_time(&t_i) )
	{
		return false;
	}

	if(is_exp)
	{
		*is_exp = (t1 <= t_i);
	}

	return true;
}
