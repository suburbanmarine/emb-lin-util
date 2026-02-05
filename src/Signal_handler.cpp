/**
 * This file is part of emb-lin-util, a collection of utility code for embedded linux.
 * 
 * This software is distrubuted in the hope it will be useful, but without any warranty, including the implied warrranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See LICENSE.txt for details.
 * 
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the LGPL-3.0 license. See LICENSE.txt for details.
 * SPDX-License-Identifier: LGPL-3.0-only
*/

#include "emb-lin-util/Signal_handler.hpp"

#include <spdlog/spdlog.h>

bool Signal_handler::mask_all_signals()
{
	sigset_t set;
	int ret = sigfillset(&set);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not create signal mask");
		return false;
	}

	ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}
	return true;
}

bool Signal_handler::mask_handled_signals()
{
	sigset_t set;
	if( ! make_handled_signal_mask(&set) )
	{
		SPDLOG_ERROR("Could not make signal mask");
		return false;
	}

	int ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}

	return true;
}

bool Signal_handler::mask_def_stop_signals()
{
	sigset_t set;
	if( ! make_def_stop_signal_mask(&set) )
	{
		SPDLOG_ERROR("Could not make signal mask");
		return false;
	}

	int ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}

	return true;
}

bool Signal_handler::mask_signals(sigset_t* const set)
{
	int ret = pthread_sigmask(SIG_SETMASK, set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}

	return true;
}

bool Signal_handler::wait_for_signal(sigset_t* const set, const timespec& timeout)
{
	siginfo_t info;

	bool keep_going = true;
	do
	{
		int ret = sigtimedwait(set, &info, &timeout);
		if(ret < 0)
		{
			switch(errno)
			{
				case EAGAIN:
				{
					return true;
				}
				case EINTR:
				{
					continue;
				}
				case EINVAL:
				{
					return false;
				}
				default:
				{
					return false;
				}
			}
		}
		else
		{
			keep_going = false;
			switch(ret)
			{
				case SIGHUP:
				{
					got_sighup = true;
					break;
				}
				case SIGINT:
				{
					got_sigint = true;
					break;
				}
				case SIGALRM:
				{
					got_sigalrm = true;
					break;
				}
				case SIGUSR1:
				{
					got_sigusr1 = true;
					break;
				}
				case SIGUSR2:
				{
					got_sigusr2 = true;
					break;
				}
				case SIGTERM:
				{
					got_sigterm = true;
					break;
				}
				default:
				{
					SPDLOG_WARN("Got unexpected signal: {:d}", ret);			
				}
			}
		}
	} while(keep_going);

	return true;
}

bool Signal_handler::make_handled_signal_mask(sigset_t* const set)
{
	int ret = sigemptyset(set);
	if(ret != 0)
	{
		return false;
	}

	//terminal hangup
	ret = sigaddset(set, SIGHUP);
	if(ret != 0)
	{
		return false;
	}

	// ctrl-c
	ret = sigaddset(set, SIGINT);
	if(ret != 0)
	{
		return false;
	}

	// posix timer
	ret = sigaddset(set, SIGALRM);
	if(ret != 0)
	{
		return false;
	}

	//user 1
	ret = sigaddset(set, SIGUSR1);
	if(ret != 0)
	{
		return false;
	}

	//user 2
	ret = sigaddset(set, SIGUSR2);
	if(ret != 0)
	{
		return false;
	}

	//generic exit request
	ret = sigaddset(set, SIGTERM);
	if(ret != 0)
	{
		return false;
	}

	return true;
}

bool Signal_handler::make_def_stop_signal_mask(sigset_t* const set)
{
	int ret = sigemptyset(set);
	if(ret != 0)
	{
		return false;
	}

	//terminal hangup
	ret = sigaddset(set, SIGHUP);
	if(ret != 0)
	{
		return false;
	}

	// ctrl-c
	ret = sigaddset(set, SIGINT);
	if(ret != 0)
	{
		return false;
	}

	//generic exit request
	ret = sigaddset(set, SIGTERM);
	if(ret != 0)
	{
		return false;
	}

	return true;
}
