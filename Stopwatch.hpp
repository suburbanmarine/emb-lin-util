/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <chrono>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch();

	bool start();
	// bool stop();
	bool reset();

	bool get_time(std::chrono::nanoseconds* const dt) const;

	void set_alarm(const std::chrono::nanoseconds& dt);

	bool is_expired(bool* const is_exp);

protected:

	std::chrono::nanoseconds t0;
	std::chrono::nanoseconds t1;
};
