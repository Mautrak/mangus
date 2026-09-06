cmake_minimum_required(VERSION 3.16)
project(mangus LANGUAGES C)

# Mangus - Uzak Diyarlar tabanlı Türkçe MUD sunucusu.
# Derleme:  cmake -B build && cmake --build build
# Test:     ctest --test-dir build --output-on-failure

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(MANGUS_WERROR "Uyarıları hata say" OFF)

include(CheckSymbolExists)
include(CheckIncludeFile)

# Eski crypt(3) parola özetlerini doğrulayabilmek için (yalnızca POSIX).
set(MANGUS_CRYPT_LIB "")
if(NOT WIN32)
  check_include_file(crypt.h HAVE_CRYPT_H)
  find_library(CRYPT_LIBRARY crypt)
  if(CRYPT_LIBRARY)
    set(CMAKE_REQUIRED_LIBRARIES ${CRYPT_LIBRARY})
  endif()
  if(HAVE_CRYPT_H)
    check_symbol_exists(crypt crypt.h HAVE_CRYPT)
  else()
    check_symbol_exists(crypt unistd.h HAVE_CRYPT)
  endif()
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(HAVE_CRYPT AND CRYPT_LIBRARY)
    set(MANGUS_CRYPT_LIB ${CRYPT_LIBRARY})
  endif()
endif()

file(GLOB MANGUS_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.c)
add_executable(mangus ${MANGUS_SOURCES})
target_include_directories(mangus PRIVATE ${CMAKE_SOURCE_DIR}/src)

set(MANGUS_WARNINGS -Wall -Wextra -Wno-unused-parameter)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  # gcc'nin bilinen gürültülü sezgisel denetimleri (yanlış pozitif üretir):
  list(APPEND MANGUS_WARNINGS -Wno-format-truncation -Wno-stringop-truncation
       -Wno-maybe-uninitialized)
endif()
if(MANGUS_WERROR)
  list(APPEND MANGUS_WARNINGS -Werror)
endif()

foreach(tgt mangus)
  target_compile_options(${tgt} PRIVATE ${MANGUS_WARNINGS})
  if(HAVE_CRYPT)
    target_compile_definitions(${tgt} PRIVATE HAVE_CRYPT=1)
  endif()
  if(HAVE_CRYPT_H)
    target_compile_definitions(${tgt} PRIVATE HAVE_CRYPT_H=1)
  endif()
endforeach()

if(WIN32)
  target_link_libraries(mangus PRIVATE ws2_32)
  target_compile_definitions(mangus PRIVATE _WIN32_WINNT=0x0601)
elseif(MANGUS_CRYPT_LIB)
  target_link_libraries(mangus PRIVATE ${MANGUS_CRYPT_LIB})
endif()

# ---------------------------------------------------------------- testler
enable_testing()

add_executable(test_unit
  tests/unit/test_unit.c
  src/utf8.c
  src/password.c)
target_include_directories(test_unit PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(test_unit PRIVATE ${MANGUS_WARNINGS})
add_test(NAME unit COMMAND test_unit)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME e2e
    COMMAND ${Python3_EXECUTABLE} -m pytest -q ${CMAKE_SOURCE_DIR}/tests/e2e
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  set_tests_properties(e2e PROPERTIES
    ENVIRONMENT "MANGUS_BIN=$<TARGET_FILE:mangus>"
    TIMEOUT 600)
endif()
