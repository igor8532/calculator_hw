cmake_minimum_required(VERSION 3.10)

project(calculator_hw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
    libmath_ho
    GIT_REPOSITORY git@github.com:igor8532/libmath_ho.git
    GIT_TAG main
)

FetchContent_MakeAvailable(libmath_ho)

add_executable(calculator_hw
    src/app.cpp
    src/main.cpp
)

target_include_directories(calculator_hw
    PRIVATE
    ${libmath_ho_SOURCE_DIR}/include
)

install(TARGETS calculator_hw
    RUNTIME DESTINATION /usr/local/bin
)

option(ENABLE_CLANG_FORMAT "Enable clang-format" OFF)
if(ENABLE_CLANG_FORMAT)
    find_program(CLANG_FORMAT_PATH NAMES clang-format-19 clang-format REQUIRED)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_PATH}")

    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        src/*.h
        src/*.cpp
    )

    # Цель для проверки форматирования (без исправления)
    add_custom_target(clang-format-check
        COMMAND
            ${CLANG_FORMAT_PATH} --dry-run -Werror -style=file ${CLANG_FORMAT_SRC}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Checking format with ${CLANG_FORMAT_PATH} ..."
    )

    # Цель для исправления форматирования
    add_custom_target(clang-format-fix
        COMMAND
            ${CLANG_FORMAT_PATH} -style=file -i ${CLANG_FORMAT_SRC}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Formatting with ${CLANG_FORMAT_PATH} ..."
    )

    add_dependencies(calculator_hw clang-format-fix)
endif()

option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy-19 clang-tidy REQUIRED)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_PATH}")

    set(CLANG_TIDY_COMMAND ${CLANG_TIDY_PATH} "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
    set_target_properties(calculator_hw PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()
