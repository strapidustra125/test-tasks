# Минимальная версия CMake:
cmake_minimum_required(VERSION 3.0)


# Характеристики проекта "zmq_app":
project(zmq_app VERSION 1.0 LANGUAGES CXX)


# -std=c++11 - поддержка стандарта c++11
# -Wall      - выводить все варнинги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)


# Папка с хидерами
include_directories("Includes/")


## load in pkg-config support
find_package(PkgConfig)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )


# Переменная для списка источников
set(SOURCES Source/application.cpp)

# Скомпилировать исполняемый файл "zmq_app":
add_executable(zmq_app ${SOURCES})


## add the include directory to our compile directives
target_include_directories(zmq_app PUBLIC ${ZeroMQ_INCLUDE_DIR})
## at the 0mq library to our link directive
target_link_libraries(zmq_app PUBLIC ${ZeroMQ_LIBRARY})





# Заголовок библиотеки
#target_precompile_headers(zmq_app zmq.hpp)

# Присвоить переменной "C_STAND_VALUE" значение свойства "C_STANDARD":
#get_target_property(C_STAND_VALUE zmq_app C_STANDARD)

# Вывести значение полученного свойства на экран:
#message("'C_STANDARD' property is equal to [${VALUE}]")

# Установить свойству 'C_STANDARD' значение "11",
# а свойству 'C_STANDARD_REQUIRED' значение "ON":
#set_target_properties(zmq_app PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

# Выведет "The target was defined!" если таргет "zmq_app" уже определен,
# а иначе выведет "The target was not defined!":
#if(TARGET zmq_app)
#    message("The target was defined!")
#else()
#    message("The target was not defined!")
#endif()

# Установить таргет "zmq_app" в директорию "~/bin":
#install(TARGETS zmq_app DESTINATION ~/bin)
