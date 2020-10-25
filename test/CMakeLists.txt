cmake_minimum_required(VERSION 3.10)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ssltest.cert.pem DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ssltest.key.pem DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ssltest.cert.pem DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ssltest.key.pem DESTINATION ${CMAKE_BINARY_DIR})

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()


find_package(Protobuf REQUIRED)


# Download and unpack googletest at configure time
configure_file(cmake/gtest-download.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)


if(PROTOBUF_FOUND)
    message ("protobuf found")
else()
    message (FATAL_ERROR "Cannot find Protobuf")
endif()


add_custom_command(
    COMMAND node ${CODEGENERATOR}/cpp/cpp.js ${CMAKE_CURRENT_SOURCE_DIR}/test.fmq ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/test.fmq.h
    COMMENT "Generating cpp code out of test.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR}/proto/proto.js ${CMAKE_CURRENT_SOURCE_DIR}/test.fmq ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.proto
    COMMENT "Generating proto code out of test.fmq."
)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}/test.proto)


message("proto headers: ${PROTO_HDRS}")

include_directories("${gmock_SOURCE_DIR}/include")
include_directories("${CMAKE_BINARY_DIR}/test")

include_directories("${CMAKE_SOURCE_DIR}/mock")

file(GLOB TESTSOURCES "*.cpp")
#file(GLOB GENCODE ${CMAKE_CURRENT_BINARY_DIR}/test.fmq.cpp)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(testfinalmq ${TESTSOURCES} ${PROTO_SRCS} ${PROTO_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/test.fmq.cpp)
target_link_libraries(testfinalmq gtest_main finalmq gmock ssl ${PROTOBUF_LIBRARIES})
#add_test(NAME example_test COMMAND example)

