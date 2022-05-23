cmake_minimum_required(VERSION 3.10)


project(http)


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)




#include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(http_server http_server.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(http_server finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(http_server finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(http_server finalmq wsock32 ws2_32)
    else()
        target_link_libraries(http_server finalmq )
    endif()
endif(FINALMQ_USE_SSL)


# Client

add_executable(http_client http_client.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(http_client finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(http_client finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(http_client finalmq wsock32 ws2_32)
    else()
        target_link_libraries(http_client finalmq )
    endif()
endif(FINALMQ_USE_SSL)


