/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class Thread_base
{
public:
	Thread_base() : m_keep_running(false)
	{

	}
	virtual ~Thread_base()
	{

	}

	//not MT safe
	void launch();
	void join();
	bool joinable() const;
	
	//MT safe
	virtual void interrupt();

	bool is_interrupted() const
	{
		return ! m_keep_running.load();
	}

	void wait_for_interruption();

	template <typename Rep, typename Period >
	bool wait_for_interruption(const std::chrono::duration<Rep, Period>& dt)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_keep_running_cv.wait_for(lock, dt, std::bind(&Thread_base::is_interrupted, this));
	}

protected:

	//internal
	virtual void work();

	void dispatch_work();

	std::atomic<bool> m_keep_running;
	std::condition_variable m_keep_running_cv;
	std::mutex m_mutex;

	std::thread m_thread;
};
