cmake_minimum_required(VERSION 3.10)


project(helloworld)

file(COPY ${FINALMQ_SOURCE_DIR}/htdocs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)



SET(CODEGENERATOR ${FINALMQ_SOURCE_DIR}/codegenerator)



set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
set(HELLOWORLD_FMQ ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.fmq)
set(HELLOWORLD_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/helloworld.fmq.cpp)
set(HELLOWORLD_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/helloworld.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${HELLOWORLD_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HELLOWORLD_FMQ}
    OUTPUT ${HELLOWORLD_FMQ_CPP} ${HELLOWORLD_FMQ_H}
    COMMENT "Generating cpp code out of helloworld.fmq."
)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${FINALMQ_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${FINALMQ_BINARY_DIR})
endif()



# SERVER

add_executable(helloworld_server ${HELLOWORLD_FMQ_CPP} helloworld_server.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(helloworld_server finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(helloworld_server finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(helloworld_server finalmq wsock32 ws2_32)
    else()
        target_link_libraries(helloworld_server finalmq )
    endif()
endif(FINALMQ_USE_SSL)


# Client

add_executable(helloworld_client ${HELLOWORLD_FMQ_CPP} helloworld_client.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(helloworld_client finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(helloworld_client finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(helloworld_client finalmq wsock32 ws2_32)
    else()
        target_link_libraries(helloworld_client finalmq )
    endif()
endif(FINALMQ_USE_SSL)


