include $(THEOS)/makefiles/common.mk

TWEAK_NAME = mfiwrapper
mfiwrapper_FILES =  src/frontend/frontend.mm \
                    src/frontend/GCController_Hook_iOS.xm \
                    \
                    src/backend/backend.cpp \
                    src/backend/hidmanager/HIDManager_iOS.cpp \
                    src/backend/hidmanager/btstack_queue.cpp \
                    src/backend/hidpad/wiimote.cpp \
                    src/backend/hidpad/HIDPad_Interface.cpp \
                    src/backend/hidpad/HIDPad_Playstation3.cpp \
                    src/backend/hidpad/HIDPad_WiiMote.cpp \
                    src/backend/hidpad/HIDPad.cpp

mfiwrapper_CCFLAGS += -std=c++11 -Iinclude -Isrc/frontend
mfiwrapper_CCFLAGS += -Isrc/backend -Isrc/backend/hidpad -Isrc/backend/hidmanager


mfiwrapper_LDFLAGS = -lBTstack

include $(THEOS_MAKE_PATH)/tweak.mk
