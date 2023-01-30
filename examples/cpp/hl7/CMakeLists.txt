cmake_minimum_required(VERSION 3.10)


project(hl7)

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
set(HL7_FMQ ${CMAKE_SOURCE_DIR}/hl7def/hl7.fmq)
set(HL7_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/hl7.fmq.cpp)
set(HL7_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/hl7.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${HL7_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HL7_FMQ}
    OUTPUT ${HL7_FMQ_CPP} ${HL7_FMQ_H}
    COMMENT "Generating cpp code out of hl7.fmq."
)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(hl7_server ${HL7_FMQ_CPP} hl7_server.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(hl7_server finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(hl7_server finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(hl7_server finalmq wsock32 ws2_32)
    else()
        target_link_libraries(hl7_server finalmq )
    endif()
endif(FINALMQ_USE_SSL)


# Client

add_executable(hl7_client ${HL7_FMQ_CPP} hl7_client.cpp)

if (FINALMQ_USE_SSL)
    if (WIN32)
        target_link_libraries(hl7_client finalmq libssl libcrypto wsock32 ws2_32)
    else()
        target_link_libraries(hl7_client finalmq ssl )
    endif()
else()
    if (WIN32)
        target_link_libraries(hl7_client finalmq wsock32 ws2_32)
    else()
        target_link_libraries(hl7_client finalmq )
    endif()
endif(FINALMQ_USE_SSL)


