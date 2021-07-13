cmake_minimum_required(VERSION 3.12)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(hzCli)

set(LIB_SOURCES
	hz_cli.cpp
	hz_cli.h
	hz_cli_key.h
)

add_library(hzCli SHARED ${LIB_SOURCES})
# target_include_directories(hzCli PRIVATE "/usr/include/botan-2/")
# target_link_libraries(hzCli PUBLIC ${DEP_LIBS})

set(DEP_LIBS
	hzCli
	pthread
	boost_thread
	fmt
	z
)

set(SOURCES
	main.cpp
)
add_executable(hzCliTest ${SOURCES})
# target_include_directories(hzCliTest PRIVATE "/usr/include/botan-2/")
target_link_libraries(hzCliTest PUBLIC ${DEP_LIBS})

