cmake_minimum_required(VERSION 3.10)


project(hl7)

file(COPY ${CMAKE_SOURCE_DIR}/htdocs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(COPY ${CMAKE_SOURCE_DIR}/htdocs DESTINATION ${CMAKE_BINARY_DIR})


if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


if (!WIN32)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
endif()



set(CMAKE_CXX_STANDARD 14)



SET(CODEGENERATOR ${CMAKE_SOURCE_DIR}/codegenerator)


set(CODEGENERATOR_HL7DEF ${CODEGENERATOR}/hl7/hl7.js)
set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)



# The HL7 defines a very big amount of messages (trigger events), segments and types. 
# In hl7options.json there is a filter to use only the messages (trigger events) you need in
# your project.

set(HL7DEF_DICTIONARY ${CMAKE_BINARY_DIR}/hl7-dictionary/dist/hl7dictionary.2.7.1.js)
set(HL7DEF_TABLES ${CMAKE_BINARY_DIR}/hl7-dictionary/lib/tables.js)
set(HL7DEF_FMQ ${CMAKE_CURRENT_BINARY_DIR}/hl7dictionary.2.7.1.js.fmq)
set(HL7DEF_OPTIONS ${CMAKE_CURRENT_SOURCE_DIR}/hl7options.json)
add_custom_command(
    COMMAND node ${CODEGENERATOR_HL7DEF} --input=${HL7DEF_DICTIONARY} --tables=${HL7DEF_TABLES} --options=${HL7DEF_OPTIONS} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HL7DEF_DICTIONARY} ${HL7DEF_TABLES} ${HL7DEF_OPTIONS}
    OUTPUT ${HL7DEF_FMQ}
    COMMENT "Generating HL7 definition code from ${HL7DEF_DICTIONARY}"
)

set(HL7DEF_FMQ_CPP ${HL7DEF_FMQ}.cpp)
set(HL7DEF_FMQ_H ${HL7DEF_FMQ}.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${HL7DEF_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HL7DEF_FMQ}
    OUTPUT ${HL7DEF_FMQ_CPP} ${HL7DEF_FMQ_H}
    COMMENT "Generating cpp code out of ${HL7DEF_FMQ}"
)




include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/inc)


if (WIN32)
link_directories(${OPENSSL_DIR}/lib ${CMAKE_BINARY_DIR})
endif()



# SERVER

add_executable(hl7_server ${HL7DEF_FMQ_CPP} hl7_server.cpp)


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

add_executable(hl7_client ${HL7DEF_FMQ_CPP} hl7_client.cpp)

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


