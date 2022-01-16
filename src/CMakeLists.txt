#Add here any new .cpp file created that needs to be built and linked.
set(IPC_SRC
    "client.cpp"
    "msg_queue.cpp"
    "sem.cpp"
    "server.cpp"
    "signal.cpp"
    #"socket.cpp"
    "thread.cpp"
    "tools.cpp")


add_library(ipc_lib ${IPC_SRC})

target_include_directories(ipc_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")