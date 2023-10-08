/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "Timespec_util.hpp"

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <array>
#include <atomic>
#include <chrono>

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
	bool wait_for_event(bool* const out_got_event);

	// MT safe
	bool is_cancel_requested() const
	{
		return m_pending_cancel;
	}

	// MT safe
	int pending_event_count() const
	{
		return m_pending_event_count;
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
	std::array<int, 2> m_pipe_fd; // cancelation fd, read from [0], write to [1]

	std::atomic<int> m_pending_event_count;
	std::atomic<bool> m_pending_cancel;

	bool give_pipe();
	// bool take_pipe();
	// bool is_pipe_empty(bool* const out_is_empty);

	// out_got_error is true
	// return true - ctr is valid
	bool read_counter(uint64_t* const ctr);

	bool has_event() const
	{
		return (m_pending_event_count > 0) || is_cancel_requested();
	}

	// m_pending_event_count may be 0 if canceled, or if in early event check
	// wait_for_event may also wake spuriously if more threads wait than events were recevied, and some threads will fall through with out_event == false
	// check if it was 0 or less, and undo the subtraction if it was
	// returns true if an event was consumed
	bool dec_event_ctr_clamp()
	{
		const int prev = m_pending_event_count.fetch_sub(1);
		if(prev <= 0)
		{
			m_pending_event_count++;
		}

		return prev > 0;
	}
};
