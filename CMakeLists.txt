###############################################################################
#   Initialization + Gtest include
###############################################################################
cmake_minimum_required(VERSION 3.16)
project(CCotti)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
#   Variables and nested CMakeLists.txt
###############################################################################
add_subdirectory(lib_src)
add_subdirectory(test)
add_subdirectory(src)

add_executable(cotti_server ${USER_SRC})

target_include_directories(cotti_server PUBLIC ${USER_INC})

target_link_libraries(cotti_server PUBLIC ipc_lib)


###############################################################################
#   Install
###############################################################################
install(TARGETS ipc_lib DESTINATION ${CMAKE_SOURCE_DIR}/lib)

###############################################################################
#   Testing
###############################################################################

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  ipc_testing
  ${TEST_SRC}
)

target_include_directories(
    ipc_testing
    PUBLIC
    "${TEST_INC}"
)

target_link_libraries(
  ipc_testing
  gtest_main
  ipc_lib
)

include(GoogleTest)
gtest_discover_tests(ipc_testing)