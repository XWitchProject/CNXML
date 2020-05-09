cmake_minimum_required(VERSION 3.10)
project (cnxml)

file(GLOB cnxml_files *.c)
add_library(cnxml SHARED ${cnxml_files})
include_directories(.)



add_executable(test "test.c")
target_link_libraries(test cnxml)
#target_link_libraries(test -lprofiler)
# find_package (peparse REQUIRED)
# target_link_libraries(freedomlib ${PEPARSE_LIBRARIES}})
# target_include_directories(freedomlib ${PEPARSE_INCLUDE_DIRS}})
