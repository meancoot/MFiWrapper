include $(THEOS)/makefiles/common.mk

TWEAK_NAME = mfiwrapper
mfiwrapper_FILES = GCController.xm \
                   MFiWrapper.mm \
                   btpad/btpad.cpp \
                   btpad/btpad_queue.cpp \
                   hidpad/wiimote.cpp \
                   hidpad/HIDPad_Interface.cpp \
                   hidpad/HIDPad_Playstation3.cpp \
                   hidpad/HIDPad_WiiMote.cpp \
                   hidpad/HIDPad.cpp
mfiwrapper_CCFLAGS = -std=c++11
mfiwrapper_LDFLAGS = -lBTstack

include $(THEOS_MAKE_PATH)/tweak.mk
