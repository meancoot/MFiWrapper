## Currently supported controllers
* DualShock 4
* Wii Remote
* Wii U Pro Controller

## Issues
* Controllers can be iffy with regard to pairing or connecting; it may be necessary to attempt it multiple times, but once the connection is established there should be no issues with usage.
* DualShock3 controllers are currently unsupported.

## Pairing
You can pair devices using the normal bluetooth pairing method. Currently it may be necessary to attempt pairing many times before it will stick.

DualShock 4: Press and hold the Share and PlayStation buttons until the light bar begins flashing rapidly.
Wii Remote / Wii U Pro: Press the red sync button located on the back of the controller.

## Connecting
You can connect a paired controller to your device using normal means.

DualShock 4: Press the PlayStation button.
Wii Remote / Wii U Pro: Press any button.

## Building
* iOS building is handled by theos. (http://iphonedevwiki.net/index.php/Theos)
* Set the THEOS environment variable to the path where you installed theos.
* Copy the Headers directory from an OS X version of IOKit.framework to MFiWrapper/IOKit/.
* Copy the file System/Library/Frameworks/IOKit.framework/Versions/A/IOKit from a iOS SDK to MFiWrapper/libIOKit.dylib.
* Run 'make'

## Installing
* Run 'make package' and install the resulting .deb file on your device.
* As the 'mobile' user on the device, run 'killall BTServer'.

## TODO
* Fix odd timeouts that occur when pairing or connecting a controller.
* Test BTServer patches on older versions of iOS.
* Allow pairing of DualShock 3 controllers.

## Technical
In order to facilitate device pairing and connection a cydia substrate tweak is loaded into the BTServer binary. The OS 'read' and 'write' functions are hooked to allow inspection and modification of bluetooth packets.

* HCI Inquiry Result with RSSI and Extended Inquiry Result events which define a device whose Class of Device major field is Peripheral will have its minor filed set to keyboard. If this patch isn't applied pairing will fail with a device not supported message.

* HCI PIN Code Request Reply commands set to Wii devices will replace the PIN with the iOS devices BT address.

* HCI Accept Connection Request commands will have their role parameter switched to master. This is only required for Wii U Pro controllers, but until better device tracking is implemented it will be applied to all devices.
