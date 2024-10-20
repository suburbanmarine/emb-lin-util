/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "emb-lin-util/Timespec_util.hpp"

#include <signal.h>

#include <atomic>


class Signal_handler
{
public:

	Signal_handler() : got_sighup(false), got_sigint(false), got_sigalrm(false), got_sigusr1(false), got_sigusr2(false), got_sigterm(false)
	{

	}

	//mask all signals, for worker threads
	bool mask_all_signals();

	//mask def signals, for main thread
	bool mask_handled_signals();

	//wait for signal
	template <typename Rep, typename Period >
	bool wait_for_signal(sigset_t* const set, const std::chrono::duration<Rep, Period>& timeout)
	{
		return wait_for_signal(set, Timespec_util::from_chrono(timeout));
	}

	bool wait_for_signal(sigset_t* const set, const timespec& timeout);

	bool xch_sighup()
	{
		return got_sighup.exchange(false);
	}

	bool xch_sigint()
	{
		return got_sigint.exchange(false);
	}

	bool xch_sigalrm()
	{
		return got_sigalrm.exchange(false);
	}

	bool xch_sigusr1()
	{
		return got_sigusr1.exchange(false);
	}

	bool xch_sigusr2()
	{
		return got_sigusr2.exchange(false);
	}

	bool xch_sigterm()
	{
		return got_sigterm.exchange(false);
	}

	bool make_handled_signal_mask(sigset_t* const set);

protected:


	std::atomic<bool> got_sighup;  // -1
	std::atomic<bool> got_sigint;  // -2
	std::atomic<bool> got_sigalrm; // -6
	std::atomic<bool> got_sigusr1; // -10
	std::atomic<bool> got_sigusr2; // -12
	std::atomic<bool> got_sigterm; // -15
};