cmake_minimum_required(VERSION 3.10)


project(fmqfcgi)


if (WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
else()
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL -Wall" )
endif()


set(CMAKE_CXX_STANDARD 14)


include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
include_directories(C:/Users/David/source/repos/fcgi-2.4.0/include)     # path to fcgi library
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR} C:/Users/David/source/deployment/win32-msvc2010/debug/lib)
endif()



add_executable(fmqfcgi fmqfcgi.cpp)

if (WIN32)
target_link_libraries(fmqfcgi finalmq fcgi libssl libcrypto wsock32 ws2_32)
else()
target_link_libraries(fmqfcgi finalmq fcgi ssl pthread)
endif()



