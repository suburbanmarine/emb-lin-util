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

#pragma once

#include <atomic>
#include <chrono>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch();

	// MT safe
	bool start();
	// MT safe
	bool reset();

	// MT safe
	bool get_time(std::chrono::nanoseconds* const dt) const;
	// MT safe
	bool is_expired(const std::chrono::nanoseconds& alarm_dt, bool* const is_exp);

protected:

	static_assert(std::is_trivially_copyable_v<std::chrono::nanoseconds>);
	static_assert(std::atomic<std::chrono::nanoseconds>::is_always_lock_free);

	// ABS time of start
	std::atomic<std::chrono::nanoseconds> m_t0;
};
