# Минимальная версия CMake:
cmake_minimum_required(VERSION 3.0)


# Характеристики проекта "zmq_app":
project(zmq_app VERSION 1.0 LANGUAGES CXX)


# -std=c++11 - поддержка стандарта c++11
# -Wall      - выводить все варнинги
# -Wextra    - дополнительные варнинги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)


# Папка с хидерами
include_directories("Includes/")


# Загрузка PkgConfig
find_package(PkgConfig)

# Использование pkg-config чтобы найти ZeroMQ
pkg_check_modules(PC_ZeroMQ QUIET zmq)

# Поиск 'zmq.hpp'
find_path(ZeroMQ_INCLUDE_DIR NAMES zmq.hpp PATHS ${PC_ZeroMQ_INCLUDE_DIRS})

# Поиск libzmq
find_library(ZeroMQ_LIBRARY NAMES zmq PATHS ${PC_ZeroMQ_LIBRARY_DIRS})










## add the include directory to our compile directives
target_include_directories(zmq_app PUBLIC ${ZeroMQ_INCLUDE_DIR})



# Загрузка PkgConfig
find_package(Protobuf REQUIRED)

target_include_directories(zmq_app PUBLIC ${Protobuf_INCLUDE_DIR})



## at the 0mq library to our link directive
target_link_libraries(zmq_app PUBLIC ${ZeroMQ_LIBRARY})

target_link_libraries(zmq_app PUBLIC ${Protobuf_LIBRARIES})


# Переменная для списка источников
set(SOURCES Source/application.cpp)

# Скомпилировать исполняемый файл "zmq_app":
add_executable(zmq_app ${SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
