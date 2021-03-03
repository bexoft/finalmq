cmake_minimum_required(VERSION 3.10)


project(timer)


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)



SET(CODEGENERATOR ${CMAKE_SOURCE_DIR}/codegenerator)



set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
set(TIMER_FMQ ${CMAKE_CURRENT_SOURCE_DIR}/timer.fmq)
set(TIMER_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/timer.fmq.cpp)
set(TIMER_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/timer.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${TIMER_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TIMER_FMQ}
    OUTPUT ${TIMER_FMQ_CPP} ${TIMER_FMQ_H}
    COMMENT "Generating cpp code out of timer.fmq."
)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(timer_server ${TIMER_FMQ_CPP} timer_server.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(timer_server finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(timer_server finalmq ssl pthread)
    endif()
else()
    if (WIN32)
        target_link_libraries(timer_server finalmq wsock32 ws2_32)
    else()
        target_link_libraries(timer_server finalmq pthread)
    endif()
endif(FINALMQ_USE_SSL)


# Client

add_executable(timer_client ${TIMER_FMQ_CPP} timer_client.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(timer_client finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(timer_client finalmq ssl pthread)
    endif()
else()
    if (WIN32)
        target_link_libraries(timer_client finalmq wsock32 ws2_32)
    else()
        target_link_libraries(timer_client finalmq pthread)
    endif()
endif(FINALMQ_USE_SSL)


