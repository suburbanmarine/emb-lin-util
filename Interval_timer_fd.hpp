/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "Timespec_util.hpp"

#include <signal.h>
#include <time.h>

#include <atomic>
#include <chrono>

#include <condition_variable>
#include <mutex>
#include <optional>

//
// A timerfd / epoll based interval timer for linux
//
class Interval_timer_fd
{
public:

	Interval_timer_fd();
	virtual ~Interval_timer_fd();

	// NOT MT safe
	bool init();

	// NOT MT safe
	bool start(const std::chrono::nanoseconds& dt);

	// MT safe
	// Stops timer events but does not modify pending event count or cancelation status
	bool stop();

	// NOT MT safe
	// Do not call with waiters - call notify_cancel first
	// And ensure they all have exited
	void reset();

	// MT safe
	bool wait_for_event();

	// MT safe
	bool is_cancel_requested() const
	{
		return m_pending_cancel;
	}

	// MT safe
	// This can be used to force wake all waiters with a cancel msg
	// After this is called, all waiters should wake and not-rewait
	// This cancelation is latching
	// After calling this, call reset -> init -> start to reuse
	bool notify_cancel();

protected:

	int m_timer_fd; // timer fd
	int m_epoll_fd; // epoll fd
	fd_pair m_pipe_fd; // cancelation fd, read from 0, write to 1

	std::atomic<int> m_pending_event_count;
	std::atomic<bool> m_pending_cancel;
	
	std::mutex              m_mutex;
	std::condition_variable m_cond_var;

	bool has_event() const
	{
		return (m_pending_event_count > 0) || is_cancel_requested();
	}

	// in the event of cancelation, m_pending_event_count may be 0
	// check if it was 0 or less, and undo the subtraction if it was
	void dec_event_ctr() const
	{
		const int prev = m_pending_event_count.fetch_sub(1);
		if(prev <= 0)
		{
			m_pending_event_count++;
		}
	}
};
