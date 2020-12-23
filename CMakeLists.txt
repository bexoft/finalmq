cmake_minimum_required(VERSION 3.10)


project(finalmq)

option(FINALMQ_BUILD_EXAMPLES "Build tests" ON)
option(FINALMQ_BUILD_TESTS "Build tests" OFF)
option(FINALMQ_BUILD_COVERAGE "Enable gcov" OFF)
option(FINALMQ_BUILD_DOXYGEN "Enable doxygen" OFF)
option(FINALMQ_USE_SSL "Build with openssl" ON)



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

set(CMAKE_CXX_STANDARD 14)

# this target needs to be set before digging into subdirectories
add_custom_target( verify ${CMAKE_CURRENT_BINARY_DIR}/test/testfinalmq)

message(${CMAKE_CURRENT_BINARY_DIR})

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

SET(CODEGENERATOR ${CMAKE_SOURCE_DIR}/codegenerator)


include_directories(${CMAKE_CURRENT_BINARY_DIR}/inc)

SET(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/metadataserialize)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/metadataserialize/metadata.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/metadataserialize --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/metadataserialize/metadata.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/metadataserialize/metadata.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/metadataserialize/metadata.fmq.h
    COMMENT "Generating cpp code out of metadata.fmq."
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/remoteentity)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/remoteentity/entitydata.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/remoteentity --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/remoteentity/entitydata.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/remoteentity/entitydata.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/remoteentity/entitydata.fmq.h
    COMMENT "Generating cpp code out of entitydata.fmq."
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inc/interfaces)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${CMAKE_CURRENT_SOURCE_DIR}/inc/interfaces/fmqreg.fmq --outpath=${CMAKE_CURRENT_BINARY_DIR}/inc/interfaces --exportmacro=EXPORT_finalmq
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inc/interfaces/fmqreg.fmq
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inc/interfaces/fmqreg.fmq.cpp ${CMAKE_CURRENT_BINARY_DIR}/inc/interfaces/fmqreg.fmq.h
    COMMENT "Generating cpp code out of fmqreg.fmq."
)



#Bring the headers into the project
include_directories(inc)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp" "inc/*.h"
                  "src/helpers/*.cpp" "inc/helpers/*.h"
                  "src/json/*.cpp" "inc/json/*.h"
                  "src/logger/*.cpp" "inc/logger/*.h"
                  "src/metadata/*.cpp" "inc/metadata/*.h"
                  "src/metadataserialize/*.cpp" "inc/metadataserialize/*.h"
                  "src/poller/*.cpp" "inc/poller/*.h"
                  "src/streamconnection/*.cpp" "inc/streamconnection/*.h"
                  "src/protocolconnection/*.cpp" "inc/protocolconnection/*.h"
                  "src/protocols/*.cpp" "inc/protocols/*.h"
                  "src/remoteentity/*.cpp" "inc/remoteentity/*.h"
                  "src/connectionhub/*.cpp" "inc/connectionhub/*.h"
                  "src/serialize/*.cpp" "inc/serialize/*.h"
                  "src/serializeproto/*.cpp" "inc/serializeproto/*.h"
                  "src/serializejson/*.cpp" "inc/serializejson/*.h"
                  "src/serializevariant/*.cpp" "inc/serializevariant/*.h"
                  "src/serializestruct/*.cpp" "inc/serializestruct/*.h"
                  "src/variant/*.cpp" "inc/variant/*.h"
                  "inc/conversions/*.h"
                  "mock/*.h")

#Generate the shared library from the sources
add_library(finalmq SHARED ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/inc/metadataserialize/metadata.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/remoteentity/entitydata.fmq.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/inc/interfaces/fmqreg.fmq.cpp
    )



if (WIN32)
if (FINALMQ_USE_SSL)
    SET(OPENSSL_DIR "$ENV{OPENSSL_INSTALL_DIR}")
    include_directories(${OPENSSL_DIR}/include)
    link_directories(${OPENSSL_DIR}/lib)
    target_link_libraries(finalmq wsock32 ws2_32 ${OPENSSL_DIR}/lib/libssl.lib ${OPENSSL_DIR}/lib/libcrypto.lib)
else()
    target_link_libraries(finalmq wsock32 ws2_32)
endif()
    target_compile_options(finalmq PRIVATE -DEXPORT_finalmq)
else()
    target_compile_options(finalmq PRIVATE -Werror -Wall -Wextra)
endif()
#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
install(TARGETS finalmq DESTINATION /usr/lib)


add_subdirectory(services)


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




