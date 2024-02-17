# Минимальная версия CMake
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)


# Характеристики проекта "zmq_project"
project(zmq_project VERSION 1.0 LANGUAGES CXX)


# -std=c++11 - поддержка стандарта c++11
# -Wall      - выводить все варнинги
# -Wextra    - дополнительные варнинги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)


set(TARGET zmq_app)

# Переменная для списка источников
set(SOURCES "Source/application.cpp")
set(PROTO_SRCS "Includes/command.pb.cc")
set(PROTO_HDRS "Includes/command.pb.h")
set(ZMQ_CMD_CLASS "Includes/command.pb.hpp")

# Добавляем исполняемый файл "zmq_app"
add_executable(${TARGET} ${SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})







######## Подключение ZeroMQ ########

# Загрузка PkgConfig
find_package(PkgConfig REQUIRED)

# Использование pkg-config чтобы найти ZeroMQ
pkg_check_modules(PC_ZeroMQ QUIET zmq)

# Поиск 'zmq.hpp'
find_path(ZeroMQ_INCLUDE_DIR NAMES zmq.hpp PATHS ${PC_ZeroMQ_INCLUDE_DIRS})

# Поиск libzmq
find_library(ZeroMQ_LIBRARY NAMES zmq PATHS ${PC_ZeroMQ_LIBRARY_DIRS})

# Добавляем пути поиска заголовков к цели my_app
target_include_directories(${TARGET} PUBLIC ${ZeroMQ_INCLUDE_DIR})

# Добавляем список библиотек для компоновки с целью my_app
target_link_libraries(${TARGET} ${ZeroMQ_LIBRARY})



######## Подключение Protobuf ########



# Загрузка PkgConfig
find_package(Protobuf REQUIRED)

target_include_directories(${TARGET} PUBLIC ${Protobuf_INCLUDE_DIR})

target_link_libraries(${TARGET} ${Protobuf_LIBRARY})
