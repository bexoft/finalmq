

add_subdirectory(fmqreg)


if (FINALMQ_BUILD_FMQFCGI)
    add_subdirectory(fmqfcgi)
endif (FINALMQ_BUILD_FMQFCGI)
