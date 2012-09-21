ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=bbm

include ../../../../meta.mk

SRCS+=bbm_bps.cpp \
      bbm_js.cpp

include $(MKFILES_ROOT)/qtargets.mk

LIBS+=bbmsp bps socket QtCore
