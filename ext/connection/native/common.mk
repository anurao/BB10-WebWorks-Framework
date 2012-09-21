ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=netstatus

include ../../../../meta.mk

SRCS+=connection_bps.cpp \
      connection_js.cpp

LIBS+=bps

include $(MKFILES_ROOT)/qtargets.mk
