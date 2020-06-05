# USB-Botbase

USB-Botbase is based off of sys-botbase by olliz0r. This versions implements USB support and removes network support. If you would like to use the original version with network support, then you can download it through olliz0r's github page [here](https://github.com/olliz0r/sys-botbase).

# BotSwitchInterface
This sysmod is based on the USB-Botbase, where i added easy to add pointer peeking instead of only normal peeking, as well as peeking offsetting from main, which is infinitely better than normal peeking in my opinion, as well as the python being extremely easy to use.

## Dependencies
- Python3 is the required version of python needed to run scripts for USB-Botbase.
- Pyusb is necessary in order to communicate to the Nintendo Switch. You can install Pyusb by using the following pip command.

```bash
pip install pyusb
```
- A usb backend is necessary. Please use [Zadig](http://www.unitrunker.com/zadig.html) and install the libusbk driver to your Nintendo Switch by plugging it in while running the sys-module.
- Install libusb with [this](http://www.mediafire.com/file/wdx5lu4c37sm1cv/libusb-win32-devel-filter-1.2.6.0.exe/file).

## Warning, Please Read!

Using a hacked switch online CAN get you banned. The developers of sys-botbase and I are not responsible for any damages or bans that may occur when using this. You use this at your OWN risk.

## Installing
- Download the zip from the releases tab at the top and extract it to the root of your sd card.
- Connect your Switch to your PC with a USB-C to USB-A cable and install libusbk with Zadig.
- Run your script using python. (Example script is provided)

## Bugs
Please report any bugs if found. My discord is Shakkar23#7184

#  Beris was made thanks to:
FishGuy6564 (for porting BotBase to USB), and Analog Guy (for the python)