include $(THEOS)/makefiles/common.mk

TWEAK_NAME = BTServerFilter
BTServerFilter_FILES = src/BTServerFilter.cpp src/PacketLogger.cpp

BTServerFilter_CCFLAGS += -std=c++11


include $(THEOS_MAKE_PATH)/tweak.mk

after-install::
	install.exec "killall BTServer"
