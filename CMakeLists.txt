cmake_minimum_required(VERSION 3.11)


project(finalmq)

option(FINALMQ_FETCH_HL7 "Fetches HL7" ON)
option(FINALMQ_BUILD_EXAMPLES "Build examples" ON)
option(FINALMQ_BUILD_TESTS "Build tests" OFF)
option(FINALMQ_BUILD_SERVICES "Build services" OFF)
option(FINALMQ_BUILD_COVERAGE "Enable gcov" OFF)
option(FINALMQ_BUILD_DOXYGEN "Enable doxygen" OFF)
option(FINALMQ_HAS_NOT_WEAK_FROM_THIS "Has weak_from_this" OFF)
option(FINALMQ_INSTALL "Enable installation" ON)

if (WIN32)
    option(FINALMQ_USE_SSL "Build with openssl" OFF)    # avoid dependency to openssl for windows
else()
    option(FINALMQ_USE_SSL "Build with openssl" ON)
endif()


SET(FINALMQ_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(FINALMQ_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})


if (FINALMQ_FETCH_HL7)
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
CloneRepository("https://github.com/beckdave/hl7-dictionary.git" "beckdave-fix-compounds" "hl7-dictionary" "${FINALMQ_BINARY_DIR}/hl7-dictionary")
endif(FINALMQ_FETCH_HL7)

# Add compile flags to determine coverage   
if (FINALMQ_BUILD_COVERAGE)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -O0")
    SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
endif(FINALMQ_BUILD_COVERAGE)


if (WIN32)
else()
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function" )
endif()

if (FINALMQ_USE_SSL)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DUSE_OPENSSL" )
endif(FINALMQ_USE_SSL)


SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -DNOMINMAX" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

if (FINALMQ_HAS_NOT_WEAK_FROM_THIS)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DFINALMQ_HAS_NOT_WEAK_FROM_THIS" )
endif(FINALMQ_HAS_NOT_WEAK_FROM_THIS)

set(CMAKE_CXX_STANDARD 14)

# this target needs to be set before digging into subdirectories
add_custom_target( verify ${CMAKE_CURRENT_BINARY_DIR}/test/testfinalmq)

if (FINALMQ_BUILD_COVERAGE)
# go for the latest gcovr version locally installed via pip if available
# since latest only gcov 3.4 supports branch coverage display in html details
find_program(GCOVR NAMES gcovr PATHS /usr/local/bin /usr/bin)
if ( IS_ABSOLUTE ${GCOVR} )

    # compute the test coverage to html

    add_custom_target(coverage
        COMMAND echo " ========== COVERAGE finalmq ==========="
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/coverage
        COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/coverage/*.html
        COMMAND ${GCOVR} -r ${CMAKE_CURRENT_SOURCE_DIR}
        --object-directory ${CMAKE_CURRENT_BINARY_DIR}
        --exclude '${CMAKE_CURRENT_BINARY_DIR}/test/.*'
        --html --html-details --output ${CMAKE_CURRENT_BINARY_DIR}/coverage/coverage.html
        )

    #set_tests_properties(output_test PROPERTIES PASS_REGULAR_EXPRESSION "0;5;10")

else(  IS_ABSOLUTE ${GCOVR}  )

    MESSAGE("++ ERROR: gcovr application not found to measure coverage")

endif(  IS_ABSOLUTE ${GCOVR}  )
endif(FINALMQ_BUILD_COVERAGE)

SET(CODEGENERATOR ${FINALMQ_SOURCE_DIR}/codegenerator)


include_directories(${CMAKE_CURRENT_BINARY_DIR}/inc)

SET(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
SET(CODEGENERATOR_PROTO ${CODEGENERATOR}/proto/proto.js)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/metadataserialize/metadata.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize --exportmacro=EXPORT_finalmq
    COMMAND node ${CODEGENERATOR_PROTO} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/metadataserialize/variant.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/metadataserialize/metadata.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/metadata.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/metadata.fmq.h
    COMMENT "Generating cpp code out of metadata.fmq."
)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/metadataserialize/variant.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/metadataserialize/variant.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/variant.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/variant.fmq.h
    COMMENT "Generating cpp code out of variant.fmq."
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/remoteentity)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/remoteentity/entitydata.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/remoteentity --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/remoteentity/entitydata.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/remoteentity/entitydata.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/remoteentity/entitydata.fmq.h
    COMMENT "Generating cpp code out of entitydata.fmq."
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqreg.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqreg.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqreg.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqreg.fmq.h
    COMMENT "Generating cpp code out of fmqreg.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqprocess.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqprocess.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqprocess.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqprocess.fmq.h
    COMMENT "Generating cpp code out of fmqprocess.fmq."
)

add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqlog.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/interfaces/fmqlog.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqlog.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqlog.fmq.h
    COMMENT "Generating cpp code out of fmqlog.fmq."
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/Qt)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/Qt/qtdata.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/Qt --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/finalmq/Qt/qtdata.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/Qt/qtdata.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/Qt/qtdata.fmq.h
    COMMENT "Generating cpp code out of qtdata.fmq."
)


#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp" "inc/finalmq/*.h"
                  "src/helpers/*.cpp" "inc/finalmq/helpers/*.h"
                  "src/hl7/*.cpp" "inc/finalmq/hl7/*.h"
                  "src/json/*.cpp" "inc/finalmq/json/*.h"
                  "src/logger/*.cpp" "inc/finalmq/logger/*.h"
                  "src/metadata/*.cpp" "inc/finalmq/metadata/*.h"
                  "src/metadataserialize/*.cpp" "inc/finalmq/metadataserialize/*.h"
                  "src/poller/*.cpp" "inc/finalmq/poller/*.h"
                  "src/streamconnection/*.cpp" "inc/finalmq/streamconnection/*.h"
                  "src/protocolsession/*.cpp" "inc/finalmq/protocolsession/*.h"
                  "src/protocols/*.cpp" "inc/finalmq/protocols/*.h"
                  "src/protocols/mqtt5/*.cpp" "inc/finalmq/mqtt5/protocols/*.h"
                  "src/protocols/protocolhelpers/*.cpp" "inc/finalmq/protocols/protocolhelpers/*.h"
                  "src/remoteentity/*.cpp" "inc/finalmq/remoteentity/*.h"
                  "src/connectionhub/*.cpp" "inc/finalmq/connectionhub/*.h"
                  "src/serialize/*.cpp" "inc/finalmq/serialize/*.h"
                  "src/serializehl7/*.cpp" "inc/finalmq/serializehl7/*.h"
                  "src/serializeproto/*.cpp" "inc/finalmq/serializeproto/*.h"
                  "src/serializeqt/*.cpp" "inc/finalmq/serializeqt/*.h"
                  "src/serializejson/*.cpp" "inc/finalmq/serializejson/*.h"
                  "src/serializevariant/*.cpp" "inc/finalmq/serializevariant/*.h"
                  "src/serializestruct/*.cpp" "inc/finalmq/serializestruct/*.h"
                  "src/variant/*.cpp" "inc/finalmq/variant/*.h"
                  "inc/finalmq/conversions/*.h"
                  "mock/*.h")

#Generate the library from the sources

SET(FINALMQ_LIBRARY_SOURCES ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/metadata.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/metadataserialize/variant.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/remoteentity/entitydata.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqreg.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqprocess.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/interfaces/fmqlog.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq/Qt/qtdata.fmq.cpp)

add_library(finalmq SHARED ${FINALMQ_LIBRARY_SOURCES})

target_include_directories(finalmq SYSTEM PUBLIC ${FINALMQ_SOURCE_DIR}/inc)
target_include_directories(finalmq SYSTEM PUBLIC ${FINALMQ_BINARY_DIR}/inc)


set_property(TARGET finalmq PROPERTY DISABLE_STATIC_ANALYSIS TRUE)


if (WIN32)
#    message("== WIN32 ==")
    set(LINKLIBS ${LINKLIBS} wsock32 ws2_32 Rpcrt4)
    if (FINALMQ_USE_SSL)
        SET(OPENSSL_DIR "$ENV{OPENSSL_ROOT_DIR}")
        include_directories(${OPENSSL_DIR}/include)
        link_directories(${OPENSSL_DIR}/lib)
        set(LINKLIBS ${LINKLIBS} wsock32 ws2_32 Rpcrt4 "${OPENSSL_DIR}/lib/libssl.lib" "${OPENSSL_DIR}/lib/libcrypto.lib")
    endif()
    target_compile_options(finalmq PRIVATE -DEXPORT_finalmq)
elseif(UNIX)
#    message("== UNIX ==")
    target_compile_options(finalmq PRIVATE -Werror -Wall -Wextra)
    if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
#        message("== QNX ==")
    else()
        set(LINKLIBS ${LINKLIBS} uuid pthread)
    endif()
    if (FINALMQ_USE_SSL)
        set(LINKLIBS ${LINKLIBS} ssl)
    endif()
else()
    message("== UNKNOWN PLATFORM ==")
endif()

target_link_libraries(finalmq ${LINKLIBS})


###############################################################
# INSTALL
###############################################################

if (FINALMQ_INSTALL)
    # Set the location for library installation
    # Use "sudo make install" to apply
    install(TARGETS finalmq DESTINATION lib)
    install(DIRECTORY inc/finalmq DESTINATION include FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/finalmq DESTINATION include FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${CODEGENERATOR} DESTINATION lib/finalmq)
endif()

###############################################################



if (FINALMQ_BUILD_SERVICES)
    add_subdirectory(services)
endif()


if (FINALMQ_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (FINALMQ_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


if (FINALMQ_BUILD_DOXYGEN)
    # add a target to generate API documentation with Doxygen
    find_package(Doxygen
                 REQUIRED dot
                 OPTIONAL_COMPONENTS mscgen dia)
    if(DOXYGEN_FOUND)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
      add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen (finalmq)" VERBATIM)
    endif(DOXYGEN_FOUND)
endif (FINALMQ_BUILD_DOXYGEN)




