cmake_minimum_required(VERSION 3.10)


project(registry)


if (WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
else()
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL -Wall" )
endif()


set(CMAKE_CXX_STANDARD 14)



SET(CODEGENERATOR ${CMAKE_SOURCE_DIR}/codegenerator)



set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
set(REGISTRY_FMQ ${CMAKE_CURRENT_SOURCE_DIR}/registry.fmq)
set(REGISTRY_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/registry.fmq.cpp)
set(REGISTRY_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/registry.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${REGISTRY_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${REGISTRY_FMQ}
    OUTPUT ${REGISTRY_FMQ_CPP} ${REGISTRY_FMQ_H}
    COMMENT "Generating cpp code out of registry.fmq."
)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(registry_server ${REGISTRY_FMQ_CPP} registry_server.cpp)

if (WIN32)
target_link_libraries(registry_server finalmq libssl libcrypto wsock32 ws2_32)
else()
target_link_libraries(registry_server finalmq ssl pthread)
endif()


# Client

add_executable(registry_client ${REGISTRY_FMQ_CPP} registry_client.cpp)

if (WIN32)
target_link_libraries(registry_client finalmq libssl libcrypto wsock32 ws2_32)
else()
target_link_libraries(registry_client finalmq ssl pthread)
endif()


