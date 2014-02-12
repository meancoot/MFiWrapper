This is in early stages of development.

Currently support Wii Remote/Classic Controller, DualShock 3 and DualShock 4 controllers.

Wii Remotes and DualShock 4 controllers must be paired with BTstack,
connections created by making the device discoverable are not supported.

The WIIPair (https://github.com/meancoot/WiiPair-iOS) app can be used to make such a pairing. (This is a work in progress, DS4 support isn't ready.)


## iOS

### Building
* iOS building is handled by theos. (http://iphonedevwiki.net/index.php/Theos)
* Set the THEOS environment variable to the path where you installed theos
* Copy libBTstack.dylib into $THEOS/lib
* Run 'make'

### Installing
* Run 'make package' and install the resulting .deb file on your device.

## OS X 

### Building
* Run 'make -f Makefile.osx'

### Using
Set the environment variable DYLD_INSERT_LIBRARIES to the path to libmfiwrapper.dylib, then run the app. For example:

```
DYLD_INSERT_LIBRARIES=/path/to/libmfiwrapper.dylib /Applications/AppName.app/Contents/MacOS/AppName
```
