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

#include "emb-lin-util/Stopwatch.hpp"

#include "emb-lin-util/Chronometer.hpp"

Stopwatch::Stopwatch() : m_t0(std::chrono::nanoseconds::zero())
{
	
}

Stopwatch::~Stopwatch()
{
	
}

bool Stopwatch::start()
{
	std::chrono::nanoseconds t_i;
	if( ! Chronometer::get_time(&t_i) )
	{
		return false;
	}

	m_t0 = t_i;

	return true;
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
		*dt = (t_i - m_t0.load());
	}

	return true;
}

bool Stopwatch::is_expired(const std::chrono::nanoseconds& alarm_dt, bool* const is_exp)
{
	std::chrono::nanoseconds dt;
	if( ! get_time(&dt) )
	{
		return false;
	}

	if(is_exp)
	{
		*is_exp = (dt >= alarm_dt);
	}

	return true;
}
