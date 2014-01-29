This is in early stages of development.

Currently supports WiiMote and Playstation 3 controllers.
On iOS WiiMotes can only be connected while the app is in wireless controller detection mode.

## iOS

### Building
iOS building is handled by theos. (http://iphonedevwiki.net/index.php/Theos)
Set the THEOS environment variable to the path where you installed theos, then run make.

### Installing
Run 'make package' and install the resulting .deb file on your device.

## OS X

### Building
Simply run 'make -f Makefile.osx'

### Using
Set the environment variable DYLD_INSERT_LIBRARIES to the path to libmfiwrapper.dylib, then run the app. For example:

```
DYLD_INSERT_LIBRARIES=/path/to/libmfiwrapper.dylib /Applications/AppName.app/Contents/MacOS/AppName
```
