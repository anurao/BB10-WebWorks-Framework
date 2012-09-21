ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=netstatus

# Add your required library names, here
LIBS+=bps

EXTRA_INCVPATH+=../../../../../dependencies/jnext_1_0_8_3/jncore/jnext-extensions/common

EXTRA_SRCVPATH+=../../../../../dependencies/jnext_1_0_8_3/jncore/jnext-extensions/common

include $(MKFILES_ROOT)/qtargets.mk
