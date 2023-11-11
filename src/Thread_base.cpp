/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "emb-lin-util/Thread_base.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <fmt/std.h>

void Thread_base::launch()
{
	if( ! m_thread.joinable() )
	{
		m_keep_running = true;
		m_thread = std::thread(&Thread_base::dispatch_work, this);
	}
}

void Thread_base::work()
{

}

//MT safe
void Thread_base::interrupt()
{
	m_keep_running.store(false);
	m_keep_running_cv.notify_all();
}
void Thread_base::join()
{
	if(joinable())
	{
		m_thread.join();
	}
}
bool Thread_base::joinable() const
{
	return m_thread.joinable();
}

void Thread_base::wait_for_interruption()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_keep_running_cv.wait(lock, std::bind(&Thread_base::is_interrupted, this));
}

void Thread_base::dispatch_work()
{
	SPDLOG_DEBUG("Thread started: {}", std::this_thread::get_id());
	try
	{
		work();
	}
	catch(const std::exception& e)
	{
		SPDLOG_DEBUG("Thread caught exception {:s}", e.what());
		throw;
	}
	catch(...)
	{
		SPDLOG_DEBUG("Thread caught exception ...");
		throw;
	}

	SPDLOG_DEBUG("Thread exiting: {}", std::this_thread::get_id());
}
