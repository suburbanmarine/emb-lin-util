/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Interval_timer_fd.hpp"

#include <functional>

#include <cstring>

Interval_timer_fd::Interval_timer_fd() : m_pending_event_count(0), m_pending_cancel(false)
{
	m_timer_fd      = -1;
	m_epoll_fd      = -1;
	m_pipe_fd.fd[0] = -1;
	m_pipe_fd.fd[1] = -1;
}
Interval_timer_fd::~Interval_timer_fd()
{
	reset();
}

bool Interval_timer_fd::init()
{
	reset();

	m_timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if(m_timer_fd < 0)
	{
		reset();
		return false;
	}

	m_epoll_fd = epoll_create(2);
	if(m_epoll_fd < 0)
	{
		reset();
		return false;
	}

	int ret = pipe(m_pipe_fd);
	if(ret != 0)
	{
		reset();
		return false;
	}

	{
		epoll_event ev;
		memset(&ev, 0, sizeof(ev));

		ev.events  = EPOLLIN;
		ev.data.fd = m_timer_fd;
		ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_timer_fd, ev);
		if(ret != 0)
		{
			reset();
			return false;
		}

		memset(&ev, 0, sizeof(ev));
		ev.events  = EPOLLIN;
		ev.data.fd = m_timer_fd;
		ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_pipe_fd.fds[0], ev);
		if(ret != 0)
		{
			reset();
			return false;
		}
	}

	return true;
}
bool Interval_timer_fd::start(const std::chrono::nanoseconds& dt)
{
	if( ! stop() )
	{
		return false;
	}

	itimerspec new_val;
	memset(&new_val, 0, sizeof(new_val));

	// set the interval
	new_val.it_interval = Timespec_util::from_chrono(dt);

	// set the first expiration as 1 interval in future
	new_val.it_value = new_val.it_interval;

	int ret = timerfd_settime(m_timer_fd, 0, &new_val, nullptr);
	if(ret != 0)
	{
		return false;
	}

	return true;
}
bool Interval_timer_fd::stop()
{
	itimerspec new_val;
	memset(&new_val, 0, sizeof(new_val));

	int ret = timerfd_settime(m_timer_fd, 0, &new_val, nullptr);
	if(ret != 0)
	{
		return false;
	}

	return true;
}

void Interval_timer_fd::reset()
{
	if(m_timer_fd >= 0)
	{
		int ret = close(m_timer_fd);
		m_timer_fd = -1;
	}

	if(m_epoll_fd >= 0)
	{
		int ret = close(m_epoll_fd);
		m_epoll_fd = -1;
	}

	if(m_pipe_fd.fd[0] >= 0)
	{
		int ret = close(m_pipe_fd.fd[0]);
		m_pipe_fd.fd[0] = -1;
	}

	if(m_pipe_fd.fd[1] >= 0)
	{
		int ret = close(m_pipe_fd.fd[1]);
		m_pipe_fd.fd[1] = -1;
	}

	m_pending_event_count = 0;
	m_pending_cancel      = false;
}

bool Interval_timer_fd::wait_for_event()
{
	// check early for cached events to avoid more syscalls
	if(has_event())
	{
		dec_event_ctr();
		return true;
	}

	std::array<epoll_event, 2> evs;
	memset(&ev[0], 0, sizeof(ev));
	memset(&ev[1], 0, sizeof(ev));

	int ret = epoll_wait(m_epoll_fd, evs.data(), evs.size(), -1);
	if(ret > 0)
	{
		for(int i = 0; i < ret; i++)
		{
			if(evs[i].data.fd == m_timer_fd)
			{
				//if m_timer_fd, read a uint64_t to get number of missed events and increment
				uint64_t buf = 0;
				int ret = read(m_timer_fd, &buf, sizeof(buf));
				if(ret != sizeof(buf))
				{
					return false;
				}

				m_pending_event_count += buf;
			}
			else if(evs[i].data.fd == m_pipe_fd.fds[0])
			{
				//if m_pipe_fd.fds[0], cancel is requested
				continue;
			}
			else
			{
				//Log Error
				continue;
			}
		}
	}
	else
	{
		//Log Error
	}


	dec_event_ctr();

	return true;
}

bool notify_cancel()
{
	m_pending_cancel = true;
	uint8_t buf = 0;
	int ret = write(m_pipe_fd.fds[1], &buf, sizeof(buf));
	if(ret != sizeof(buf))
	{
		return false;
	}

	return true;
}