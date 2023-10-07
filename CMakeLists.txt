add_library(emb-lin-util
	Chronometer.cpp
	Interval_timer.cpp
	Interval_timer_fd.cpp
	Stopwatch.cpp
	Thread_base.cpp
	Timespec_util.cpp
)

target_link_libraries(emb-lin-util
	Threads::Threads
	rt

	Boost::boost

	spdlog::spdlog
)

target_include_directories(emb-lin-util
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

INSTALL(
	TARGETS
		emb-lin-util
	DESTINATION
		lib
)

add_subdirectory(tests)
