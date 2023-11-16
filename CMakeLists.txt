add_library(emb-lin-util
	src/Chronometer.cpp
	src/File_util.cpp
	src/Interval_timer.cpp
	src/Interval_timer_fd.cpp
	src/Signal_handler.cpp
	src/Stopwatch.cpp
	src/Thread_base.cpp
	src/Timespec_util.cpp
	src/Zlib_util.cpp

	src/emb-lin-util.cpp
)

target_link_libraries(emb-lin-util
	Threads::Threads
	rt

	Boost::boost

	spdlog::spdlog

	ZLIB::ZLIB
)

target_include_directories(emb-lin-util
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INSTALL(
	TARGETS
		emb-lin-util
	DESTINATION
		lib
)

add_subdirectory(tests)
