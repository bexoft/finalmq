# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== USEFILE - the file containing the usage message for the application.
USEFILE=

# Next lines are for C++ projects only

EXTRA_SUFFIXES+=cxx cpp

#===== NAME - name of the project (default - name of project directory).
NAME=finalmq

#===== CXXFLAGS - add the flags to the C compiler command line.
CXXFLAGS+=-Wp,-MD,$(basename $@).d -Y _cxx -D_QNX_SOURCE -std=c++14

#===== EXTRA_CLEAN - additional files to delete when cleaning the project.
EXTRA_CLEAN+=*.d

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/inc

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+= \
	$(PROJECT_ROOT)/src/ConnectionHub \
	$(PROJECT_ROOT)/src/helpers \
	$(PROJECT_ROOT)/src/json \
	$(PROJECT_ROOT)/src/logger \
	$(PROJECT_ROOT)/src/metadata \
	$(PROJECT_ROOT)/src/poller \
	$(PROJECT_ROOT)/src/protocols \
	$(PROJECT_ROOT)/src/protocols/mqtt5 \
	$(PROJECT_ROOT)/src/protocols/protocolhelpers \
	$(PROJECT_ROOT)/src/protocolsession \
	$(PROJECT_ROOT)/src/serialize \
	$(PROJECT_ROOT)/src/serializejson \
	$(PROJECT_ROOT)/src/serializeproto \
	$(PROJECT_ROOT)/src/serializestruct \
	$(PROJECT_ROOT)/src/streamconnection \
	$(PROJECT_ROOT)/src/variant \
	$(PROJECT_ROOT)/src/serializevariant \
	$(PROJECT_ROOT)/src/remoteentity \
	$(PROJECT_ROOT)/src/metadataserialize \
	$(PROJECT_ROOT)/inc/finalmq/serializevariant \
	$(PROJECT_ROOT)/inc/finalmq/remoteentity \
	$(PROJECT_ROOT)/inc/finalmq/interfaces \
	$(PROJECT_ROOT)/inc/finalmq/metadataserialize

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Y _cxx

#===== generate code.
generate:
	node $(PROJECT_ROOT)/codegenerator/cpp/cpp.js --input=$(PROJECT_ROOT)/inc/finalmq/metadataserialize/metadata.fmq -outpath=. 
	node $(PROJECT_ROOT)/codegenerator/cpp/cpp.js --input=$(PROJECT_ROOT)/inc/finalmq/metadataserialize/variant.fmq -outpath=. 
	node $(PROJECT_ROOT)/codegenerator/cpp/cpp.js --input=$(PROJECT_ROOT)/inc/finalmq/remoteentity/entitydata.fmq -outpath=. 
	node $(PROJECT_ROOT)/codegenerator/cpp/cpp.js --input=$(PROJECT_ROOT)/inc/finalmq/interfaces/fmqreg.fmq -outpath=. 

#===== VERSION_TAG_SO - version tag for SONAME. Use it only if you don't like SONAME_VERSION
override VERSION_TAG_SO=

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

-include *.d

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

