cmake_minimum_required(VERSION 3.15)
project(tests-channel CXX)

set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE)

find_package(mpsc-ring CONFIG REQUIRED)

aux_source_directory(src SRCS)
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		gtest
		pthread
		mpsc-ring
)
