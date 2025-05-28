cmake_minimum_required(VERSION 3.10)
project(Lab1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Поиск Flex
find_package(FLEX REQUIRED)

# Генерация Flex-лексера
set(LEXER_FILE "${CMAKE_SOURCE_DIR}/include/DifRecog/lexer.l")
flex_target(LEXER ${LEXER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

# Общие исходники
set(COMMON_SOURCES
        include/DifRecog/SmcRecog_sm.cpp
        ${FLEX_LEXER_OUTPUTS}
)

# Общие заголовки
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Основная программа
add_executable(Lab1
        src/main.cpp
        ${COMMON_SOURCES}
)

target_link_libraries(Lab1 ${FLEX_LIBRARIES})
target_compile_options(Lab1 PRIVATE -Wall -Wextra)

# Анализатор
add_executable(Analyzer
        src/analiser.cpp
        ${COMMON_SOURCES}
)

target_link_libraries(Analyzer ${FLEX_LIBRARIES})
target_compile_options(Analyzer PRIVATE -Wall -Wextra)
