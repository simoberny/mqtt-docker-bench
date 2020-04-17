cmake_minimum_required(VERSION 3.5)
project(MQTT-C VERSION 1.1.0 LANGUAGES C)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

# MQTT-C library
add_library(mqttc STATIC 
    src/mqtt_pal.c
    src/mqtt.c
)

target_include_directories(mqttc PUBLIC include)

target_link_libraries(mqttc PUBLIC 
    $<$<C_COMPILER_ID:MSVS>:ws2_32>
)

# Build examples
find_package(Threads REQUIRED)

add_executable(publish publisher/simple_publisher.c)
target_link_libraries(publish Threads::Threads mqttc)


# Install includes and library
install(TARGETS mqttc 
    DESTINATION lib
)
install(DIRECTORY include
    DESTINATION include
)