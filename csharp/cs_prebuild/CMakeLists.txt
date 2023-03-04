cmake_minimum_required(VERSION 3.22)

project(generate NONE)

message("=== RUN cmake =========")


include(FetchContent)

function(CloneRepository repositoryURL branchName projectName sourceDir)
  #Commands are left empty so that we only checkout the source and no not perform any kind of build
  message("Starting to clone ${projectName} into ${sourceDir}")
  FetchContent_Declare(
    "${projectName}"
    GIT_REPOSITORY "${repositoryURL}"
    SOURCE_DIR "${sourceDir}"
    GIT_TAG "origin/${branchName}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  FetchContent_MakeAvailable(${projectName})
endfunction(CloneRepository)

CloneRepository("https://github.com/beckdave/hl7-dictionary.git" "beckdave-fix-compounds" "hl7-dictionary" "${CMAKE_SOURCE_DIR}/../bin/hl7-dictionary")



SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)
SET(CODEGENERATOR ${ROOT_SOURCE_DIR}/codegenerator)
SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CODEGENERATOR_CSHARP ${CODEGENERATOR}/csharp/csharp.js)


add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/metadataserialize/metadata.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/metadataserialize/metadata.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/metadataserialize/metadata.fmq.cs
    COMMENT "Generating cs code out of metadata.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/metadataserialize/variant.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/metadataserialize/variant.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/metadataserialize/variant.fmq.cs
    COMMENT "Generating cs code out of variant.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/remoteentity/entitydata.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/remoteentity/entitydata.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/remoteentity/entitydata.fmq.cs
    COMMENT "Generating cs code out of entitydata.fmq."
)



ADD_CUSTOM_TARGET( generate ALL 
                    DEPENDS 
                        ${GENERATOR_OUTPUT_DIR}/metadataserialize/metadata.fmq.cs
                        ${GENERATOR_OUTPUT_DIR}/metadataserialize/variant.fmq.cs
                        ${GENERATOR_OUTPUT_DIR}/remoteentity/entitydata.fmq.cs
                   COMMENT "Checking if re-generation is required" )
                   
