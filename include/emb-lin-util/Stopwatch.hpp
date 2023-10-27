/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
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
