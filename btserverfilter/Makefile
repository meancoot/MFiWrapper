include $(THEOS)/makefiles/common.mk

TWEAK_NAME = BTServerFilter
BTServerFilter_FILES = src/BTServerFilter.cpp src/PacketLogger.cpp

include $(THEOS_MAKE_PATH)/tweak.mk

after-install::
	install.exec "killall BTServer"
