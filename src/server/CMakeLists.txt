add_executable (long_time_response long_time_response.cpp)

target_link_libraries (long_time_response uv pthread rt)
