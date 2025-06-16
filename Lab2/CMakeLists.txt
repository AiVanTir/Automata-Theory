# Минимальная версия CMake
cmake_minimum_required(VERSION 3.16)

# Название проекта 
project(TA_LAB2)

# Стандарт языка
set (CMAKE_CXX_STANDARD 20)

# Подключаемые каталоги для поиска заголовочных файлов
include_directories(${CMAKE_SOURCE_DIR}/parser ${CMAKE_SOURCE_DIR}/ast ${CMAKE_SOURCE_DIR}/automaton ${CMAKE_SOURCE_DIR}/regex)

# Флаги компиляции (сейчас используются стандартные)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Создание статической библиотеки libregex.a
add_library(regex STATIC
    parser/parser.cpp
    ast/ast.cpp
    automaton/automaton.cpp
    regex/regex.cpp
)

# Поиск библиотеки Catch2 3 версии
find_package(Catch2 3 REQUIRED)

# Создание исполняемого файла и подключение libregex
add_executable(main main.cpp)
target_link_libraries(main PRIVATE regex)

# Создание тестов и подключение libregex + Catch2WithMain
add_executable(test test.cpp)
target_link_libraries(test PRIVATE regex Catch2::Catch2WithMain)

# Инструкция по подключению нашей библиотеки вручную 
# g++ ваш_файл.cpp -Lпуть_к_библиотеке -lregex -o ваша_программа
