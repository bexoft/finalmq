cmake_minimum_required(VERSION 3.10)


project(restapi)

file(COPY ${CMAKE_SOURCE_DIR}/htdocs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)



SET(CODEGENERATOR ${CMAKE_SOURCE_DIR}/codegenerator)



set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
set(restapi_FMQ ${CMAKE_CURRENT_SOURCE_DIR}/restapi.fmq)
set(restapi_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/restapi.fmq.cpp)
set(restapi_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/restapi.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${restapi_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${restapi_FMQ}
    OUTPUT ${restapi_FMQ_CPP} ${restapi_FMQ_H}
    COMMENT "Generating cpp code out of restapi.fmq."
)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(restapi_server ${restapi_FMQ_CPP} restapi_server.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(restapi_server finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(restapi_server finalmq ssl pthread)
    endif()
else()
    if (WIN32)
        target_link_libraries(restapi_server finalmq wsock32 ws2_32)
    else()
        target_link_libraries(restapi_server finalmq pthread)
    endif()
endif(FINALMQ_USE_SSL)


# Client

add_executable(restapi_client ${restapi_FMQ_CPP} restapi_client.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(restapi_client finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(restapi_client finalmq ssl pthread)
    endif()
else()
    if (WIN32)
        target_link_libraries(restapi_client finalmq wsock32 ws2_32)
    else()
        target_link_libraries(restapi_client finalmq pthread)
    endif()
endif(FINALMQ_USE_SSL)


