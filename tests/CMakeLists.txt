add_executable(emb-lin-util-tests
	interval_timerfd_tests.cpp
	interval_timer_tests.cpp
)

target_link_libraries(emb-lin-util-tests
	emb-lin-util

	googletest_main
)

INSTALL(
	TARGETS
		emb-lin-util-tests
	DESTINATION
		tests
)
