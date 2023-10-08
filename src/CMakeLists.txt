set(USER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/http_types.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/httpserver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/temp_sensor.cpp"
    PARENT_SCOPE)

set(USER_INC
    "${PROJECT_SOURCE_DIR}/include"
    PARENT_SCOPE
)