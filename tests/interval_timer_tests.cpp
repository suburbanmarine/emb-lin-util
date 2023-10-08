#include "Interval_timer.hpp"
#include "Stopwatch.hpp"
#include "Thread_base.hpp"

#include "gtest/gtest.h"

#include <thread>

TEST(Interval_timer, basic_sync_wait)
{
	Interval_timer m_ival;
	ASSERT_TRUE(m_ival.init());

	ASSERT_TRUE(m_ival.start(std::chrono::milliseconds(500)));

	Stopwatch timer;
	ASSERT_TRUE(timer.start());

	for(size_t i = 0; i < 5; i++)
	{
		EXPECT_FALSE(m_ival.is_cancel_requested());

		bool got_event = m_ival.wait_for_event();

		EXPECT_TRUE(got_event);
	}

	std::chrono::nanoseconds dt;
	ASSERT_TRUE(timer.get_time(&dt));

	std::chrono::microseconds err = std::chrono::floor<std::chrono::microseconds>(std::chrono::abs(dt - std::chrono::milliseconds(2500)));
	ASSERT_LE(err, std::chrono::microseconds(1000));
}

class Interval_timer_waiters : public Thread_base
{
public:
	Interval_timer_waiters(const std::shared_ptr<Interval_timer>& ival, int wait_us = 0) : m_loop_cnt(0), m_event_cnt(0)
	{
		m_ival = ival;
		m_wait_us = wait_us;
	}
	~Interval_timer_waiters() override
	{

	}

	int get_loop_count() const
	{
		return m_loop_cnt;
	}

	int get_event_count() const
	{
		return m_event_cnt;
	}

protected:
	void work() override
	{
		while( ! is_interrupted() )
		{
			if(m_ival->is_cancel_requested())
			{
				break;
			}

			bool got_event = m_ival->wait_for_event();
			
			// if we woke spuriously or due to cancelation, don't count event
			if(got_event)
			{
				if(m_wait_us)
				{
					std::this_thread::sleep_for(std::chrono::microseconds(m_wait_us));
				}

				m_event_cnt++;
			}

			// if we detected cancel don't count loop
			if(m_ival->is_cancel_requested())
			{
				break;
			}

			m_loop_cnt++;
		}
	}

	int m_wait_us;

	std::atomic<int> m_loop_cnt;
	std::atomic<int> m_event_cnt;
	std::shared_ptr<Interval_timer> m_ival;
};

TEST(Interval_timer, single_async_waiter_cancel)
{
	std::shared_ptr<Interval_timer> m_ival = std::make_shared<Interval_timer>();
	ASSERT_TRUE(m_ival->init());

	Interval_timer_waiters m_async_waiter(m_ival);
	m_async_waiter.launch();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_TRUE(m_ival->start(std::chrono::milliseconds(100)));

	std::this_thread::sleep_for(std::chrono::milliseconds(2510));

	m_ival->notify_cancel();
	m_async_waiter.join();

	EXPECT_EQ(m_async_waiter.get_event_count(), 25);
	EXPECT_EQ(m_async_waiter.get_loop_count(), 25);
}

TEST(Interval_timer, single_async_waiter_stop)
{
	std::shared_ptr<Interval_timer> m_ival = std::make_shared<Interval_timer>();
	ASSERT_TRUE(m_ival->init());

	Interval_timer_waiters m_async_waiter(m_ival);
	m_async_waiter.launch();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_TRUE(m_ival->start(std::chrono::milliseconds(100)));

	std::this_thread::sleep_for(std::chrono::milliseconds(2510));

	ASSERT_TRUE(m_ival->stop());
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	m_ival->notify_cancel();
	m_async_waiter.join();

	EXPECT_EQ(m_async_waiter.get_event_count(), 25);
	EXPECT_EQ(m_async_waiter.get_loop_count(), 25);
}

TEST(Interval_timer, multiple_async_waiters)
{
	std::shared_ptr<Interval_timer> m_ival = std::make_shared<Interval_timer>();
	ASSERT_TRUE(m_ival->init());

	std::array<std::shared_ptr<Interval_timer_waiters>, 8> m_async_waiters;
	for(auto& val : m_async_waiters)
	{
		val = std::make_shared<Interval_timer_waiters>(m_ival);
		val->launch();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_TRUE(m_ival->start(std::chrono::milliseconds(100)));

	std::this_thread::sleep_for(std::chrono::milliseconds(2510));

	// send async cancel, we expect 25 events to be handled across all of the waiters
	m_ival->notify_cancel();

	for(auto& val : m_async_waiters)
	{
		val->join();
	}

	size_t total_event_cnt = 0;
	size_t total_loop_cnt = 0;
	for(auto& val : m_async_waiters)
	{
		total_event_cnt += val->get_event_count();
		total_loop_cnt  += val->get_loop_count();
	}
	EXPECT_EQ(total_event_cnt, 25U);

	EXPECT_LE(total_loop_cnt, 25U * m_async_waiters.size());
	EXPECT_GE(total_loop_cnt, 25U);
}

TEST(Interval_timer, multiple_async_waiters_fast)
{
	std::shared_ptr<Interval_timer> m_ival = std::make_shared<Interval_timer>();
	ASSERT_TRUE(m_ival->init());

	std::array<std::shared_ptr<Interval_timer_waiters>, 8> m_async_waiters;
	for(auto& val : m_async_waiters)
	{
		val = std::make_shared<Interval_timer_waiters>(m_ival);
		val->launch();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_TRUE(m_ival->start(std::chrono::milliseconds(1)));

	std::this_thread::sleep_for(std::chrono::milliseconds(2500) + std::chrono::microseconds(300));

	// send async cancel, we expect 25 events to be handled across all of the waiters
	m_ival->notify_cancel();

	for(auto& val : m_async_waiters)
	{
		val->join();
	}

	size_t total_event_cnt = 0;
	size_t total_loop_cnt = 0;
	for(auto& val : m_async_waiters)
	{
		total_event_cnt += val->get_event_count();
		total_loop_cnt  += val->get_loop_count();
	}
	EXPECT_EQ(total_event_cnt, 2500U);

	EXPECT_LE(total_loop_cnt, 2500U * m_async_waiters.size());
	EXPECT_GE(total_loop_cnt, 2500U);
}


TEST(Interval_timer, multiple_async_waiters_fast_backlog)
{
	std::shared_ptr<Interval_timer> m_ival = std::make_shared<Interval_timer>();
	ASSERT_TRUE(m_ival->init());

	std::array<std::shared_ptr<Interval_timer_waiters>, 8> m_async_waiters;
	for(auto& val : m_async_waiters)
	{
		val = std::make_shared<Interval_timer_waiters>(m_ival, 50000);
		val->launch();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_TRUE(m_ival->start(std::chrono::milliseconds(1)));

	std::this_thread::sleep_for(std::chrono::milliseconds(2500) + std::chrono::microseconds(300));

	ASSERT_TRUE(m_ival->stop());

	// send async cancel, we expect 25 events to be handled across all of the waiters
	m_ival->notify_cancel();

	for(auto& val : m_async_waiters)
	{
		val->join();
	}

	size_t total_event_cnt = 0;
	size_t total_loop_cnt = 0;
	for(auto& val : m_async_waiters)
	{
		total_event_cnt      += val->get_event_count();
		total_loop_cnt       += val->get_loop_count();
	}
	EXPECT_GE(total_loop_cnt, 49 * (int)m_async_waiters.size()); // per thread 2500 / 50 - 1
	EXPECT_LE(total_loop_cnt, 50 * (int)m_async_waiters.size()); // per thread 2500 / 50
	
	EXPECT_GE(total_event_cnt, 49 * (int)m_async_waiters.size());
	EXPECT_LE(total_event_cnt, 50 * (int)m_async_waiters.size());
	
	EXPECT_GE(m_ival->pending_event_count(), 2500 - total_event_cnt-(int)m_async_waiters.size());
	EXPECT_LE(m_ival->pending_event_count(), 2500 - total_event_cnt);
}
