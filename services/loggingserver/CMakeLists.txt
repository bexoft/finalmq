cmake_minimum_required(VERSION 3.10)


project(loggingserver)


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()


set(CMAKE_CXX_STANDARD 14)


include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
    link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



add_executable(loggingserver main.cpp)



if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(loggingserver finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(loggingserver finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(loggingserver finalmq wsock32 ws2_32)
    else()
        target_link_libraries(loggingserver finalmq )
    endif()
endif(FINALMQ_USE_SSL)



