cmake_minimum_required(VERSION 3.22)

project(generate NONE)

message("=== RUN cmake =========")

SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../..)
SET(CODEGENERATOR ${ROOT_SOURCE_DIR}/codegenerator)
SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

#set(CODEGENERATOR_HL7DEF ${CODEGENERATOR}/hl7/hl7.js)
set(CODEGENERATOR_CSHARP ${CODEGENERATOR}/csharp/csharp.js)
set(CODEGENERATOR_PROTO ${CODEGENERATOR}/proto/proto.js)


# The HL7 defines a very big amount of messages (trigger events), segments and types. 
# In hl7options.json there is a filter to use only the messages (trigger events) you need in
# your project.

#set(HL7DEF_FILE hl7dictionary.2.7.1.js)
#set(HL7DEF_DICTIONARY ${CMAKE_SOURCE_DIR}/hl7-dictionary/dist/${HL7DEF_FILE})
#set(HL7DEF_TABLES ${CMAKE_SOURCE_DIR}/hl7-dictionary/lib/tables.js)
#set(HL7DEF_FMQ_DIR ${CMAKE_SOURCE_DIR}/..)
#set(HL7DEF_FMQ_FILE ${HL7DEF_FMQ_DIR}/hl7dictionary.2.7.1.js.fmq)
#add_custom_command(
#    COMMAND node ${CODEGENERATOR_HL7DEF} --input=${HL7DEF_DICTIONARY} --tables=${HL7DEF_TABLES} --options=${HL7DEF_OPTIONS} --outpath=${HL7DEF_FMQ_DIR}
#    DEPENDS ${HL7DEF_DICTIONARY} ${HL7DEF_TABLES}
#    OUTPUT ${HL7DEF_FMQ_FILE}
#    COMMENT "Generating HL7 definition code from ${HL7DEF_DICTIONARY}"
#)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/test.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/test.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/test.fmq.cs
    COMMENT "Generating cpp code out of test.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${PROJECT_SOURCE_DIR}/testhl7.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/testhl7.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/testhl7.fmq.cs
    COMMENT "Generating cpp code out of testhl7.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_PROTO} --input=${PROJECT_SOURCE_DIR}/test.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/test.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/test.proto
    COMMENT "Generating proto code out of test.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_PROTO} --input=${PROJECT_SOURCE_DIR}/../../src/metadataserialize/variant.fmq --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/../../src/metadataserialize/variant.fmq
    OUTPUT ${GENERATOR_OUTPUT_DIR}/variant.proto
    COMMENT "Generating proto code out of variant.fmq."
)

add_custom_command(
    COMMAND protoc --proto_path=${GENERATOR_OUTPUT_DIR} --csharp_out=${GENERATOR_OUTPUT_DIR} ${GENERATOR_OUTPUT_DIR}/test.proto ${GENERATOR_OUTPUT_DIR}/variant.proto
    DEPENDS ${GENERATOR_OUTPUT_DIR}/test.proto ${GENERATOR_OUTPUT_DIR}/variant.proto
    OUTPUT ${GENERATOR_OUTPUT_DIR}/test.cs ${GENERATOR_OUTPUT_DIR}/variant.cs
    COMMENT "Generating cs code out of test.proto and variant.proto."
)



ADD_CUSTOM_TARGET( generate ALL 
                    DEPENDS 
                        ${GENERATOR_OUTPUT_DIR}/testhl7.fmq.cs 
                        ${GENERATOR_OUTPUT_DIR}/test.fmq.cs
                        ${GENERATOR_OUTPUT_DIR}/test.cs
                        ${GENERATOR_OUTPUT_DIR}/variant.cs
                   COMMENT "Checking if re-generation is required" )
                   
