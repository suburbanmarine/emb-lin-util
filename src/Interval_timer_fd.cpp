/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "emb-lin-util/Interval_timer_fd.hpp"

#include <sys/epoll.h>
#include <sys/timerfd.h>

#include <cerrno>
#include <cstring>

Interval_timer_fd::Interval_timer_fd() : m_pending_event_count(0), m_pending_cancel(false)
{
	m_timer_fd = -1;
	m_epoll_fd = -1;
	m_pipe_fd.fill(-1);
}
Interval_timer_fd::~Interval_timer_fd()
{
	reset();
}

bool Interval_timer_fd::init()
{
	reset();

	m_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | O_NONBLOCK);
	if(m_timer_fd < 0)
	{
		reset();
		return false;
	}

	m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if(m_epoll_fd < 0)
	{
		reset();
		return false;
	}

	int ret = pipe2(m_pipe_fd.data(), O_CLOEXEC | O_NONBLOCK);
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
		ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_timer_fd, &ev);
		if(ret != 0)
		{
			reset();
			return false;
		}

		memset(&ev, 0, sizeof(ev));
		ev.events  = EPOLLIN;
		ev.data.fd = m_timer_fd;
		ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_pipe_fd[0], &ev);
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

	if(m_pipe_fd[0] >= 0)
	{
		int ret = close(m_pipe_fd[0]);
		m_pipe_fd[0] = -1;
	}

	if(m_pipe_fd[1] >= 0)
	{
		int ret = close(m_pipe_fd[1]);
		m_pipe_fd[1] = -1;
	}

	m_pending_event_count = 0;
	m_pending_cancel      = false;
}

bool Interval_timer_fd::wait_for_event(bool* const out_got_event)
{
	bool got_error  = false;
	bool got_event  = false;

	// check early for cached events to avoid more syscalls
	// m_pending_event_count will not be updated, any pending ticks will remain kernel side until has_event is false
	if(has_event())
	{
		got_event = dec_event_ctr_clamp();
	}
	else
	{
		std::array<epoll_event, 2> evs;
		memset(evs.data(), 0, evs.size() * sizeof(epoll_event));

		int ret = 0;
		errno   = 0;
		do
		{
			ret = epoll_wait(m_epoll_fd, evs.data(), evs.size(), -1);
		} while( (ret == -1) && (errno == EINTR) );

		if(ret > 0)
		{
			for(int i = 0; i < ret; i++)
			{
				if(evs[i].data.fd == m_timer_fd)
				{
					// if m_timer_fd, read a uint64_t to get number of missed events and increment
					// we hope that buf == 1, but hey, sometimes things are slow

					// ask kernel for event count
					uint64_t buf = 0;
					if( ! read_counter(&buf) )
					{
						got_error = true;
					}

					// record the number of events
					if(buf > 0)
					{
						buf--;
						m_pending_event_count.fetch_add(buf);
						got_event = true;
					}
					else
					{
						// m_pending_event_count may be non zero, try to decrement it
						// in the event of a race condition we may not actually get an event but will wake
						// in any case, the application will check if got_event == false, and do nothing in this thread context if so
						// there is probably some room to optimize here, this is the "thundering herd" problem
						// We do not worry about it now as this will mostly be used for single worker thread calling wait.
						// generally a single thread will read the timer backlog count, post it, and the other threads will decrement it here
						got_event = dec_event_ctr_clamp();
					}
				}
				else if(evs[i].data.fd == m_pipe_fd[0])
				{
					//if m_pipe_fd[0], cancel is requested

					// we do not read the pipe, since we want cancel request to latch and prevent waiting

					// we do not check for pending timer events, since epoll_wait will notify us and we handle it in the previous if stmt

					// users should check if it has been canceled before sleeping
				}
				else
				{
					//Log Error, unexpected fd
				}
			}
		}
		else if(ret == 0)
		{
			// spurious wake?
		}
		else
		{
			// error
			got_error = true;
		}
	}

	if(out_got_event)
	{
		*out_got_event = got_event;
	}

	return ! got_error;
}

bool Interval_timer_fd::notify_cancel()
{
	const bool prev_pending_cancel = m_pending_cancel.exchange(true);

	// only give the pipe if we are the first to cancel
	// we do not want pipe to have more than 1 byte, eg we want a binary semaphore
	if( ! prev_pending_cancel )
	{
		return give_pipe();
	}

	// otherwise we do not need to give the pipe
	return true;
}

bool Interval_timer_fd::give_pipe()
{
	uint8_t buf = 0;

	ssize_t ret = 0;
	errno   = 0;

	do
	{
		ret = write(m_pipe_fd[1], &buf, sizeof(buf));
	} while((ret == -1) && (errno == EINTR) );
	
	if(ret != 1)
	{
		return false;
	}

	return true;
}
#if 0
bool Interval_timer_fd::take_pipe()
{
	uint8_t buf = 0;

	ssize_t ret = 0;
	errno   = 0;

	do
	{
		ret = read(m_pipe_fd[0], &buf, sizeof(buf));
	} while((ret == -1) && (errno == EINTR) );
	
	if(ret != 1)
	{
		return false;
	}

	return true;
}

bool Interval_timer_fd::is_pipe_empty(bool* const out_is_empty)
{
	int read_availible = 0;
	int ret = ioctl(m_pipe_fd[0], &FIONREAD, &read_availible);
	if(ret != 0)
	{
		return false;
	}

	if(out_is_empty)
	{
		*out_is_empty = read_availible > 0;
	}

	return true;
}
#endif

bool Interval_timer_fd::read_counter(uint64_t* const ctr)
{
	ssize_t ret = 0;
	errno   = 0;

	bool fret = false;

	*ctr = 0;

	do
	{
		ret = read(m_timer_fd, ctr, sizeof(uint64_t));
	} while((ret == -1) && (errno == EINTR) );

	if( ret < 0 )
	{
		if(errno == EAGAIN)
		{
			// no ticks & O_NONBLOCK is set, not really an error
			fret = true;
		}
		else
		{
			// other error
			fret = false;
		}
	}
	else if(ret != sizeof(uint64_t))
	{
		//something is very broken
		fret = false;
	}
	else if(ret == sizeof(uint64_t))
	{
		// ctr was updated by read
		fret = true;
	}
	else
	{
		//something is very broken
		fret = false;	
	}

	return fret;
}
