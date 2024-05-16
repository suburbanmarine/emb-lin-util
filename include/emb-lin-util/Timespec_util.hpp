/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2023 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <signal.h>
#include <time.h>

#include <chrono>

class Timespec_util
{
public:
	static timespec add(const timespec& a, timespec& b)
	{
		timespec out;

		out.tv_sec  = a.tv_sec  + b.tv_sec;
		out.tv_nsec = a.tv_nsec + b.tv_nsec;

		if(out.tv_nsec > 1000000000L)
		{
			out.tv_sec++;
			out.tv_nsec -= 1000000000L;
		}

		return out;
	}
	static timespec sub(const timespec& a, timespec& b)
	{
		timespec out;
		
		out.tv_sec  = a.tv_sec  - b.tv_sec;
		out.tv_nsec = a.tv_nsec - b.tv_nsec;

		if(out.tv_nsec < 0L)
		{
			out.tv_sec--;
			out.tv_nsec += 1000000000L;
		}

		return out;
	}

	template <typename Rep, typename Period>
	static timespec from_chrono(const std::chrono::duration<Rep, Period>& dt)
	{
		const std::chrono::seconds     t_sec  = std::chrono::floor<std::chrono::seconds>(dt);
		const std::chrono::nanoseconds t_nsec = std::chrono::floor<std::chrono::nanoseconds>(dt - t_sec);

		timespec out;		
		out.tv_sec  = t_sec.count();
		out.tv_nsec = t_nsec.count();

		return out;
	}

	template <typename T>
	static T to_chrono(const timespec& dt)
	{
		std::chrono::seconds     t_sec(dt.tv_sec);
		std::chrono::nanoseconds t_nsec(dt.tv_nsec); // TODO: this is limited to +/- 584 years

		return std::chrono::floor< T >(t_sec + t_nsec); // TODO: this is limited to +/- 584 years
	}

	template <typename Rep, typename Period>
	static std::chrono::duration<Rep, Period> to_chrono(const timespec& dt)
	{
		std::chrono::seconds     t_sec(dt.tv_sec);
		std::chrono::nanoseconds t_nsec(dt.tv_nsec); // TODO: this is limited to +/- 584 years

		return std::chrono::floor< std::chrono::duration<Rep, Period> >(t_sec + t_nsec); // TODO: this is limited to +/- 584 years
	}
};
