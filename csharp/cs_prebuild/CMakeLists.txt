cmake_minimum_required(VERSION 3.22)

project(generate NONE)

message("=== RUN cmake =========")

SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)
SET(CODEGENERATOR ${ROOT_SOURCE_DIR}/codegenerator)
SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CODEGENERATOR_CSHARP ${CODEGENERATOR}/csharp/csharp.js)


add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/metadataserialize/metadata.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/metadataserialize/metadata.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/metadataserialize/metadata.fmq.cs
    COMMENT "Generating cpp code out of metadata.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/metadataserialize/variant.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/metadataserialize/variant.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/metadataserialize/variant.fmq.cs
    COMMENT "Generating cpp code out of variant.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/remoteentity/entitydata.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/remoteentity/entitydata.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/remoteentity/entitydata.fmq.cs
    COMMENT "Generating cpp code out of entitydata.fmq."
)



ADD_CUSTOM_TARGET( generate ALL 
                    DEPENDS 
                        ${GENERATOR_OUTPUT_DIR}/metadataserialize/metadata.fmq.cs
                        ${GENERATOR_OUTPUT_DIR}/metadataserialize/variant.fmq.cs
                        ${GENERATOR_OUTPUT_DIR}/remoteentity/entitydata.fmq.cs
                   COMMENT "Checking if re-generation is required" )
                   
