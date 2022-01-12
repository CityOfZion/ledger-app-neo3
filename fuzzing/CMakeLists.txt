cmake_minimum_required(VERSION 3.10)

project(NeoFuzzer VERSION 0.1.9 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

# BOLOS SDK
set(BOLOS_SDK $ENV{BOLOS_SDK})
add_compile_definitions(IO_HID_EP_LENGTH=64)

add_compile_definitions(HAVE_ECC HAVE_HASH HAVE_SHA256 HAVE_RIPEMD160)
include_directories(.
        ../src/lib
        "${BOLOS_SDK}/include"
        "${BOLOS_SDK}/lib_cxng/include"
)

add_compile_options(-g -ggdb2 -O3)

# Build with code coverage generation
if(CODE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        link_libraries(gcov)
    else()
        message(FATAL_ERROR "Unsupported compiler used with code coverage generation")
    endif()
endif()

# Fuzzer target
set(APP_SRC_DIR "../src")

set(APP_SOURCES
    ${APP_SRC_DIR}/transaction/deserialize.c
    ${APP_SRC_DIR}/transaction/deserialize.h
    ${APP_SRC_DIR}/transaction/tx_utils.c
    ${APP_SRC_DIR}/transaction/tx_utils.h
    ${APP_SRC_DIR}/common/base58.c
    ${APP_SRC_DIR}/common/base58.h
    ${APP_SRC_DIR}/common/buffer.c
    ${APP_SRC_DIR}/common/buffer.h
    ${APP_SRC_DIR}/common/read.c
    ${APP_SRC_DIR}/common/read.h
    ${APP_SRC_DIR}/common/write.c
    ${APP_SRC_DIR}/common/write.h
    ${APP_SRC_DIR}/common/varint.c
    ${APP_SRC_DIR}/common/varint.h
    ${APP_SRC_DIR}/ui/utils.c
    ${APP_SRC_DIR}/ui/utils.h
)

add_executable(fuzz_message
        fuzz_neo3.c
        os_mocks.c
        ${APP_SOURCES}
)

target_include_directories(fuzz_message PUBLIC ../src)
target_compile_options(fuzz_message PUBLIC -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_message PUBLIC -fsanitize=fuzzer,address,undefined)
