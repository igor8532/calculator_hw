cmake_minimum_required(VERSION 3.10)

project(calculator_hw)

set(SOURCES_COMMON
    src/Application.cpp
    src/Logger.cpp
)

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
)

set(LIBRARIES_COMMON
    libmath_ho
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
    libmath_ho
    GIT_REPOSITORY git@github.com:igor8532/libmath_ho.git
    GIT_TAG main
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)

FetchContent_MakeAvailable(
    libmath_ho
    nlohmann_json
    spdlog
)

add_executable(calculator_hw
    ${SOURCES_COMMON}
    src/main.cpp
)

target_include_directories(calculator_hw
    PRIVATE
        ${INCLUDE_DIRS}
)

target_link_libraries(calculator_hw
    PRIVATE
        ${LIBRARIES_COMMON}
)

install(TARGETS calculator_hw
    RUNTIME DESTINATION /usr/local/bin
)

option(ENABLE_CLANG_FORMAT "Enable clang-format" ON)
if(ENABLE_CLANG_FORMAT)
    find_program(CLANG_FORMAT_PATH NAMES clang-format-19 clang-format REQUIRED)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_PATH}")

    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        include/*.h
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

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy-19 clang-tidy REQUIRED)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_PATH}")

    set(CLANG_TIDY_COMMAND ${CLANG_TIDY_PATH} "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
    set_target_properties(calculator_hw PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

option(ENABLE_TESTS "Enable unit tests" ON)
if(ENABLE_TESTS)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0 
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(calculator_tests
        tests/test_calculator.cpp
        ${SOURCES_COMMON}
    )

    target_include_directories(calculator_tests
        PRIVATE
            ${INCLUDE_DIRS}
            ${CMAKE_SOURCE_DIR}/tests
    )

    target_link_libraries(calculator_tests
        PRIVATE
            GTest::gtest_main
            ${LIBRARIES_COMMON}
    )

    target_compile_features(calculator_tests PRIVATE cxx_std_17)

    include(GoogleTest)
    gtest_discover_tests(calculator_tests)
endif()