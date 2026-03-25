cmake_minimum_required(VERSION 3.16)
project(LLRPAgent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ENABLE_M7E "Build optional ThingMagic M7E support" OFF)

# ── Rutas al SDK ────────────────────────────────────────────────────────────
set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# LTKCPP: headers from the installed package (has out_ltkcpp.h),
# but we compile the library from source (1.0.0.5) with MinGW.
set(LTKCPP_ROOT "${REPO_ROOT}/lib/libltkcpp_lib-x86-linux-windows_1_0_0_8/libltkcpp" CACHE PATH "LTKCPP header folder")
set(LTKCPP_INCLUDE "${LTKCPP_ROOT}")

# LTKCPP C++ source directory (version 1.0.0.5 – compilable with MinGW)
set(LTKCPP_SRC_DIR "${REPO_ROOT}/lib/ltkcpp_src/ltkcpp_src_1_0_0_5/LTK/LTKCPP/Library")

# libxml2: use the system MinGW package (already installed via pacman)
find_package(LibXml2 REQUIRED)

# Mercury API C (solo para soporte M7E opcional)
set(MERCURYC_INCLUDE "${REPO_ROOT}/mercuryapi-1.37.0.80/c/src")
set(MERCURYC_LIB_DIR "${REPO_ROOT}/mercuryapi-1.37.0.80/c/src")

# ── Compile LTKCPP from source ───────────────────────────────────────────────
# The 1.0.0.5 sources use #ifdef WIN32, but -std=c++17 in MinGW only defines
# _WIN32 (not WIN32). We therefore add WIN32 as a compile definition.
set(LTKCPP_SOURCES
    ${LTKCPP_SRC_DIR}/ltkcpp_array.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_connection.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_element.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_encdec.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_error.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_framedecode.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_frameencode.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_frameextract.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_genout.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_hdrfd.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_typeregistry.cpp
    ${LTKCPP_SRC_DIR}/ltkcpp_xmltextencode.cpp
)

add_library(ltkcpp_built STATIC ${LTKCPP_SOURCES})

# LTKCPP 1.0.0.5 generates many write-strings warnings in auto-generated code
# on modern GCC/Clang. Keep project warnings clean without mutating vendor code.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ltkcpp_built PRIVATE -Wno-write-strings)
endif()

target_include_directories(ltkcpp_built PUBLIC
    ${LTKCPP_INCLUDE}        # provides out_ltkcpp.h and the installed headers
    ${LTKCPP_SRC_DIR}        # provides ltkcpp_platform.h from source
    ${LIBXML2_INCLUDE_DIR}
)

# Define WIN32 so that the platform header activates the Windows typedefs
# even when compiled with -std=c++17 (which drops the non-standard WIN32 macro)
target_compile_definitions(ltkcpp_built PRIVATE WIN32)

if(WIN32)
    target_link_libraries(ltkcpp_built PUBLIC ws2_32 ${LIBXML2_LIBRARIES})
else()
    target_link_libraries(ltkcpp_built PUBLIC pthread ${LIBXML2_LIBRARIES})
endif()

# ── Fuentes ─────────────────────────────────────────────────────────────────
set(AGENT_SOURCES
    src/main.cpp
    src/llrp/Config.cpp
    src/reader/ReaderAdapter.cpp
    src/readers/GenericLLRPReader.cpp
    src/readers/MockLLRPReader.cpp
)

if(ENABLE_M7E)
    list(APPEND AGENT_SOURCES src/readers/ThingMagicM7EReader.cpp)
endif()

set(TEST_CLIENT_SOURCES
    src/LLRPTestClient.cpp
    src/llrp/Config.cpp
)

# ── Ejecutable: agente ───────────────────────────────────────────────────────
add_executable(llrp_agent ${AGENT_SOURCES})

target_include_directories(llrp_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LTKCPP_INCLUDE}
)

if(ENABLE_M7E)
    target_include_directories(llrp_agent PRIVATE ${MERCURYC_INCLUDE})
endif()

if(ENABLE_M7E)
    target_link_directories(llrp_agent PRIVATE ${MERCURYC_LIB_DIR})
endif()

target_link_libraries(llrp_agent PRIVATE ltkcpp_built)

if(ENABLE_M7E)
    target_link_libraries(llrp_agent PRIVATE mercuryapi)
    target_compile_definitions(llrp_agent PRIVATE LLRP_AGENT_ENABLE_M7E=1)
endif()

# ── Ejecutable: test client ─────────────────────────────────────────────────
add_executable(llrp_test_client ${TEST_CLIENT_SOURCES})

target_include_directories(llrp_test_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LTKCPP_INCLUDE}
)

target_link_libraries(llrp_test_client PRIVATE ltkcpp_built)

# ── Salida en cpp/build/ ────────────────────────────────────────────────────
set_target_properties(llrp_agent llrp_test_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "LTKCPP root: ${LTKCPP_ROOT}")
message(STATUS "LTKCPP source dir: ${LTKCPP_SRC_DIR}")
message(STATUS "ThingMagic M7E support: ${ENABLE_M7E}")
