include $(THEOS)/makefiles/common.mk

TWEAK_NAME = mfiwrapper
mfiwrapper_FILES = src/GCController_Hook_iOS.xm \
                   src/GCController.mm \
                   src/MFiWrapper.mm \
                   src/hidmanager/HIDManager_iOS.cpp \
                   src/hidmanager/btstack_queue.cpp \
                   src/hidpad/wiimote.cpp \
                   src/hidpad/HIDPad_Interface.cpp \
                   src/hidpad/HIDPad_Playstation3.cpp \
                   src/hidpad/HIDPad_WiiMote.cpp \
                   src/hidpad/HIDPad.cpp
mfiwrapper_CCFLAGS = -std=c++11 -Isrc -Isrc/hidmanager -Isrc/hidpad
mfiwrapper_LDFLAGS = -lBTstack

include $(THEOS_MAKE_PATH)/tweak.mk
