

add_subdirectory(cpp/helloworld)
add_subdirectory(cpp/timer)
add_subdirectory(cpp/restapi)
add_subdirectory(cpp/experimental/registry)
add_subdirectory(cpp/experimental/http)

if (FINALMQ_FETCH_HL7)
	add_subdirectory(cpp/hl7)
endif(FINALMQ_FETCH_HL7)