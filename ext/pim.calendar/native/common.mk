ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=pimcalendar
PLUGIN=yes
JSON=yes

include ../../../../meta.mk

SRCS+=pim_calendar_qt.cpp \
      pim_calendar_js.cpp \
      timezone_utils.cpp \
      service_provider.cpp \
      account_folder_mgr.cpp \
	  thread_sync.cpp

ifeq ($(UNITTEST),yes)
NAME=test
SRCS+=test_main.cpp
LIBS+=img
USEFILE=
endif

include $(MKFILES_ROOT)/qtargets.mk

LIBS+=bbpim QtCore icuuc icudata icui18n
