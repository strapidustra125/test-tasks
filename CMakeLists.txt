# Задать третью минимальную версию CMake:
cmake_minimum_required(VERSION 3.0)

# Задать характеристики проекта "zmq_app":
project(zmq_app VERSION 1.0 LANGUAGES CXX)

# Скомпилировать исполняемый файл "zmq_app" из исходников
# "application.cpp":
add_executable(zmq_app application.cpp zmq.hpp)

# Заголовок библиотеки
#target_precompile_headers(zmq_app zmq.hpp)

# Присвоить переменной "C_STAND_VALUE" значение свойства "C_STANDARD":
get_target_property(C_STAND_VALUE zmq_app C_STANDARD)

# Вывести значение полученного свойства на экран:
#message("'C_STANDARD' property is equal to [${VALUE}]")

# Установить свойству 'C_STANDARD' значение "11",
# а свойству 'C_STANDARD_REQUIRED' значение "ON":
set_target_properties(zmq_app PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

# Выведет "The target was defined!" если таргет "zmq_app" уже определен,
# а иначе выведет "The target was not defined!":
if(TARGET zmq_app)
    message("The target was defined!")
else()
    message("The target was not defined!")
endif()

# Установить таргет "zmq_app" в директорию "~/bin":
install(TARGETS zmq_app DESTINATION bin)
