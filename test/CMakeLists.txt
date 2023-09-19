set(TEST_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/test_msg_queue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_sem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_shared_mem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_signal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_thread.cpp"
    PARENT_SCOPE)

set(TEST_INC
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    PARENT_SCOPE
)