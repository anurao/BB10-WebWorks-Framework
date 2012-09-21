ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=pushjnext

include ../../../../meta.mk

SRCS+=push_js.cpp \
      push_ndk.cpp

include $(MKFILES_ROOT)/qtargets.mk

LIBS+=PushService
