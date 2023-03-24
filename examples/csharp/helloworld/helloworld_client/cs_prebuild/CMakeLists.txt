cmake_minimum_required(VERSION 3.22)

project(generateHelloWorldClient NONE)

message("=== RUN cmake helloworld_client =========")



SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../..)
SET(CODEGENERATOR ${ROOT_SOURCE_DIR}/codegenerator)
SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CODEGENERATOR_CSHARP ${CODEGENERATOR}/csharp/csharp.js)

message(${GENERATOR_OUTPUT_DIR})

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/../helloworld.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/../helloworld.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/helloworld.fmq.cs
    COMMENT "Generating cs code out of helloworld.fmq."
)



ADD_CUSTOM_TARGET( generateHelloWorldClient ALL 
                    DEPENDS 
                        ${GENERATOR_OUTPUT_DIR}/helloworld.fmq.cs
                   COMMENT "Checking if re-generation is required" )
                   
