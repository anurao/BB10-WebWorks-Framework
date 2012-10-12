ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=timezone
PLUGIN=yes
JSON=yes

include ../../../../meta.mk

SRCS+=timezone_qt.cpp \
      timezone_js.cpp \
      timezone_utils.cpp

include $(MKFILES_ROOT)/qtargets.mk

LIBS+=bbpim QtCore icuuc icudata icui18n