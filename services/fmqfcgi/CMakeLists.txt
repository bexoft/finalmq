cmake_minimum_required(VERSION 3.10)


project(fmqfcgi)


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)


include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
include_directories(C:/Users/David/source/repos/fcgi-2.4.0/include)     # path to fcgi library
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR} C:/Users/David/source/repos/fcgi-2.4.0/x64/Debug)
endif()



add_executable(fmqfcgi fmqfcgi.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(fmqfcgi finalmq fcgi libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(fmqfcgi finalmq fcgi ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(fmqfcgi finalmq fcgi wsock32 ws2_32)
    else()
        target_link_libraries(fmqfcgi finalmq fcgi )
    endif()
endif(FINALMQ_USE_SSL)



