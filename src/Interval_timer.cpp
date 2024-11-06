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

#include "emb-lin-util/Interval_timer.hpp"

#include <functional>

#include <cstring>

Interval_timer::Interval_timer() : m_pending_event_count(0), m_pending_cancel(false)
{

}
Interval_timer::~Interval_timer()
{
	reset();
}

bool Interval_timer::init()
{
	sigevent sig = {};

	sig.sigev_notify            = SIGEV_THREAD;
	sig.sigev_value.sival_ptr   = this;
	sig.sigev_notify_function   = &Interval_timer::dispatch_event;
	sig.sigev_notify_attributes = NULL;

	timer_t tim;
	memset(&tim, 0, sizeof(tim));
	
	int ret = timer_create(CLOCK_MONOTONIC, &sig, &tim);
	if(ret != 0)
	{
		return false;
	}

	m_timer = tim;

	return true;
}
bool Interval_timer::start(const std::chrono::nanoseconds& dt)
{
	if( ! stop() )
	{
		return false;
	}

	{
		m_pending_event_count = 0;
		m_pending_cancel      = false;
	}

	itimerspec new_val;
	memset(&new_val, 0, sizeof(new_val));

	// set the interval
	new_val.it_interval = Timespec_util::from_chrono(dt);

	// set the first expiration as 1 interval in future
	new_val.it_value = new_val.it_interval;

	int ret = timer_settime(m_timer.value(), 0, &new_val, nullptr);
	if(ret != 0)
	{
		return false;
	}

	return true;
}
bool Interval_timer::stop()
{
	itimerspec new_val;
	memset(&new_val, 0, sizeof(new_val));

	int ret = timer_settime(m_timer.value(), 0, &new_val, nullptr);
	if(ret != 0)
	{
		return false;
	}

	return true;
}

void Interval_timer::reset()
{
	if(m_timer.has_value())
	{
		timer_delete(m_timer.value());
		m_timer.reset();

		m_pending_event_count = 0;
		m_pending_cancel      = false;
	}
}

bool Interval_timer::wait_for_event()
{
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_cond_var.wait(lock, std::bind(&Interval_timer::should_wake, this));
	}

	// in the event of cancelation, m_pending_event_count may be 0
	// check if it was 0 or less, and undo the subtraction if it was
	bool consumed_event = true;
	const int prev = m_pending_event_count.fetch_sub(1);
	if(prev <= 0)
	{
		consumed_event = false;
		m_pending_event_count++;
	}

	return consumed_event;
}
