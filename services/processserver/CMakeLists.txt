cmake_minimum_required(VERSION 3.10)


project(processserver)


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



add_executable(processserver defines.h main.cpp peercache.h peercache.cpp platformspecific.cpp platformspecific.h processes.cpp processes.h 
               transactionbase.cpp transactionbase.h transactionstart.cpp transactionstart.h transactionstop.cpp transactionstop.h
               watchprocesses.cpp watchprocesses.h)



if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(processserver finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(processserver finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(processserver finalmq wsock32 ws2_32)
    else()
        target_link_libraries(processserver finalmq )
    endif()
endif(FINALMQ_USE_SSL)



