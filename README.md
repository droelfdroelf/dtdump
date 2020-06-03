# dtdump
An open [Overbridge](https://www.elektronauts.com/t/overbridge-2-0-public-beta-plugins-drivers-and-firmware/70486) protocol implementation proof of concept.

Dumps all channels to a multi-channel WAV file.

It will try both PIDs (first Digitakt, then Digitone) 
during USB init and use the first available one. So
it should work now on both devices.

Note that everything is tested with a Digitakt,
running OS 1.11_BETA6! 

Most certainly other Overbridge-enabled
devices won't work out of the box without modifications.
See source for details.

# Build instructions

(needs cmake, libusb, libsndfile. CDT project files are also in the repo)
```
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
make
```

On Mac OS X you might need to set env variable: `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`

# Usage

**Note** You have to configure the routing accordingly as in the picture below. **This is not the default setting**! It is likely that the data format, number of channels etc. differ without changing this, and dtdump will fail (since everything is hard coded so far).
![OB rounting](pics/obroute.jpg?raw=true "Routing setup")

Just run ```./dtdump```. It will dump the stream to ```dtdump-<time stamp>.wav```
(12 channels, 32 Bit integer, 48kHz srate) until you press Ctrl-C.

**New:** Record to 12 mono files with ```./dtdump -s```

## Real-time priority
The USB transfer thread tries to grab the highest possible real-time priority. Unless configured to do so, this is usually only allowed for the root user.
See also [http://jackaudio.org/faq/linux_rt_config.html](http://jackaudio.org/faq/linux_rt_config.html)

On Mac OS X, the following works to reduce Xruns:

```
sudo nice -n -20 ./dtdump
```

## USB udev rule
To access the Digitakt as non-root user, a udev file is provided in /udev.

# Dependencies on Raspbian
install deps: ```sudo apt install libsndfile1-dev libusb-1.0-0-dev cmake 
build-essential git```

Has to be run as root in default Raspbian (lite) installation.

# Storage requirements
About 2.3MB/s, a class 10 SD card is OK on a Rpi3.

# Thanks
Thanks to [Jeremy Pepper](https://github.com/LnxPrgr3) for the awesome [Message queue](https://github.com/LnxPrgr3/message_queue)

LICENSE: MIT

Stefan Rehm, 2019
droelfdroelf ( at ) gmail.com
