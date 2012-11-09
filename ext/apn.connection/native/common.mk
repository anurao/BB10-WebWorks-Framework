ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=myvzwext
PLUGIN=yes
JSON=yes

include ../../../../meta.mk

SRCS+=routing_if_helper.cpp

include $(MKFILES_ROOT)/qtargets.mk
