ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=bbm

EXTRA_INCVPATH+=$(QNX_TARGET)/usr/include/qt4/QtCore \
                ../../../../../dependencies/jnext_1_0_8_3/jncore/jnext-extensions/common \
                ../../../../../dependencies/JsonCpp/jsoncpp-src-0.5.0/include

EXTRA_SRCVPATH+=../../../../../dependencies/jnext_1_0_8_3/jncore/jnext-extensions/common \
                ../../../../../dependencies/JsonCpp/jsoncpp-src-0.5.0/src/lib_json

LIBS+=bps socket

include $(MKFILES_ROOT)/qtargets.mk
