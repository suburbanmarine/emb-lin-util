project(emb-lin-util
	VERSION 0.1.0
	DESCRIPTION "Misc utility code"
	LANGUAGES C CXX
)

configure_file(./src/version.hpp.in version.hpp)

add_library(emb-lin-util
	src/Chronometer.cpp
	src/File_util.cpp
	src/Interval_timer.cpp
	src/Interval_timer_fd.cpp
	src/JSON_CBOR_helper.cpp
	src/Signal_handler.cpp
	src/Stopwatch.cpp
	src/Thread_base.cpp
	src/Timespec_util.cpp
	src/Zlib_util.cpp

	src/emb-lin-util.cpp

	src/version.cpp
)

target_link_libraries(emb-lin-util
	Threads::Threads
	rt

	Boost::boost
	nlohmann_json::nlohmann_json
	spdlog::spdlog
	ZLIB::ZLIB
)

target_include_directories(emb-lin-util
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(emb-lin-util
PROPERTIES
	VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

INSTALL(
	DIRECTORY
		include/
	DESTINATION
		include
)

INSTALL(
	TARGETS
		emb-lin-util
	DESTINATION
		lib
)

add_subdirectory(tests)
