cmake_minimum_required(VERSION 3.22)

project(generate NONE)

message("=== RUN cmake =========")



SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../..)
SET(CODEGENERATOR ${ROOT_SOURCE_DIR}/codegenerator)
SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CODEGENERATOR_CSHARP ${CODEGENERATOR}/csharp/csharp.js)
set(CODEGENERATOR_HL7DEF ${CODEGENERATOR}/hl7/hl7.js)



# The HL7 defines a very big amount of messages (trigger events), segments and types. 
# In hl7options.json there is a filter to use only the messages (trigger events) you need in
# your project.

set(HL7DEF_DICTIONARY ${ROOT_SOURCE_DIR}/csharp/bin/hl7-dictionary/dist/hl7dictionary.2.7.1.js)
set(HL7DEF_TABLES ${ROOT_SOURCE_DIR}/csharp/bin/hl7-dictionary/lib/tables.js)
set(HL7DEF_FMQ ${GENERATOR_OUTPUT_DIR}/hl7dictionary.2.7.1.js.fmq)
set(HL7DEF_OPTIONS ${PROJECT_SOURCE_DIR}/hl7options.json)
add_custom_command(
    COMMAND node ${CODEGENERATOR_HL7DEF} --input=${HL7DEF_DICTIONARY} --tables=${HL7DEF_TABLES} --options=${HL7DEF_OPTIONS} --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${HL7DEF_DICTIONARY} ${HL7DEF_TABLES} ${HL7DEF_OPTIONS}
    OUTPUT ${HL7DEF_FMQ}
    COMMENT "Generating HL7 definition code from ${HL7DEF_DICTIONARY}"
)


set(HL7DEF_FMQ_CS ${HL7DEF_FMQ}.cs)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CSHARP} --input=${HL7DEF_FMQ} --outpath=${GENERATOR_OUTPUT_DIR}
    DEPENDS ${HL7DEF_FMQ}
    OUTPUT ${HL7DEF_FMQ_CS}
    COMMENT "Generating cs code out of ${HL7DEF_FMQ}"
)

ADD_CUSTOM_TARGET( generate ALL 
                    DEPENDS 
                        ${HL7DEF_FMQ_CS}
                    COMMENT "Checking if re-generation is required" )
                   
