#include "HIDPad.h"
#include "../MFiWrapper.h"


HIDPad::Interface::Interface(HIDManager::Connection* aConnection) :
    playerIndex(-1), connection(aConnection), stateManager(0)
{
    connection = aConnection;
}

HIDPad::Interface::~Interface()
{
    detach_tweak_controller(this);
}

void HIDPad::Interface::FinalizeConnection()
{
    attach_tweak_controller(this);
}