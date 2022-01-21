#Add here any new .cpp file created that needs to be built and linked.
set(IPC_SRC
    "sem.cpp"
    "server.cpp"
    "signal.cpp"
    "socket.cpp"
    "thread.cpp"
    "tools.cpp"
    "mutex.cpp")


add_library(ipc_lib ${IPC_SRC})

target_include_directories(ipc_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_compile_options(ipc_lib PUBLIC -pthread)
target_link_options(ipc_lib PUBLIC -pthread)