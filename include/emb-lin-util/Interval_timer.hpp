/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
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
// A timer based interval timer for posix
//
class Interval_timer
{
public:

	Interval_timer();
	virtual ~Interval_timer();

	// NOT MT safe
	bool init();

	// NOT MT safe
	bool start(const std::chrono::nanoseconds& dt);

	// MT safe
	// stops timer but does not cancel or reset event count
	bool stop();

	// NOT MT safe
	// Do not call with waiters - call notify_cancel first
	// And ensure they all have exited
	void reset();

	// MT safe
	// true  - has event
	// false - no event
	bool wait_for_event();

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
	void notify_cancel()
	{
		m_pending_cancel = true;
		m_cond_var.notify_all();
	}

protected:

	std::optional<timer_t> m_timer;

	std::atomic<int> m_pending_event_count;
	std::atomic<bool> m_pending_cancel;
	
	std::mutex              m_mutex;
	std::condition_variable m_cond_var;

	bool should_wake() const
	{
		return (m_pending_event_count > 0) || is_cancel_requested();
	}

	bool has_event() const
	{
		return m_pending_event_count > 0;
	}

	static void dispatch_event(sigval sig)
	{
		reinterpret_cast<Interval_timer*>(sig.sival_ptr)->handle_event(sig);
	}
	void handle_event(sigval sig)
	{
		m_pending_event_count++;
		m_cond_var.notify_one();
	}

};
