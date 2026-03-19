cmake_minimum_required(VERSION 3.16)
project(LLRPAgent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Rutas al SDK ────────────────────────────────────────────────────────────
set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# LTKCPP (extraído de lib/libltkcpp_lib-*.tgz en lib/ltkcpp/)
set(LTKCPP_ROOT "${REPO_ROOT}/lib/ltkcpp")
set(LTKCPP_INCLUDE "${LTKCPP_ROOT}/inc")
set(LTKCPP_LIB_DIR "${LTKCPP_ROOT}/lib")

# Mercury API C (compilada en mercuryapi-1.37.0.80/c/src/)
set(MERCURYC_INCLUDE "${REPO_ROOT}/mercuryapi-1.37.0.80/c/src")
set(MERCURYC_LIB_DIR "${REPO_ROOT}/mercuryapi-1.37.0.80/c/src")

# ── Fuentes ─────────────────────────────────────────────────────────────────
set(AGENT_SOURCES
    src/main.cpp
    src/llrp/Config.cpp
    src/reader/ReaderAdapter.cpp
    src/readers/GenericLLRPReader.cpp
    src/readers/ThingMagicM7EReader.cpp
)

set(TEST_CLIENT_SOURCES
    src/LLRPTestClient.cpp
)

# ── Ejecutable: agente ───────────────────────────────────────────────────────
add_executable(llrp_agent ${AGENT_SOURCES})

target_include_directories(llrp_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LTKCPP_INCLUDE}
    ${MERCURYC_INCLUDE}
)

target_link_directories(llrp_agent PRIVATE
    ${LTKCPP_LIB_DIR}
    ${MERCURYC_LIB_DIR}
)

target_link_libraries(llrp_agent PRIVATE
    ltkcpp
    mercuryapi
    pthread
)

# ── Ejecutable: test client ─────────────────────────────────────────────────
add_executable(llrp_test_client ${TEST_CLIENT_SOURCES})

target_include_directories(llrp_test_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LTKCPP_INCLUDE}
)

target_link_directories(llrp_test_client PRIVATE
    ${LTKCPP_LIB_DIR}
)

target_link_libraries(llrp_test_client PRIVATE
    ltkcpp
    pthread
)

# ── Salida en cpp/build/ ────────────────────────────────────────────────────
set_target_properties(llrp_agent llrp_test_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "LTKCPP include: ${LTKCPP_INCLUDE}")
message(STATUS "Mercury C include: ${MERCURYC_INCLUDE}")
