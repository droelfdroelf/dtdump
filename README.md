# dtdump
An open [Overbridge](https://www.elektronauts.com/t/overbridge-2-0-public-beta-plugins-drivers-and-firmware/70486) protocol implementation proof of concept.

Dumps all channels to a multi channel WAV file.
Note that everything is tested with a Digitakt,
running OS 1.11_BETA6! Most certainly other Overbridge enabled
devices won't work out of the box without modifications.
See source for details.

# Build instructions

(needs cmake, libusb, libsndfile. CDT project files are also in the repo)
```
mkdir build
cd build
cmake ../
make
```
# Usage

**Note** You have to configure the routing accordingly as in the picture below. **This is not the default setting**! It is likely that the data format, number of channels etc. differ without changing this, and dtdump will fail (since everything is hard coded so far).
![OB rounting](pics/obroute.jpg?raw=true "Routing setup")

Just run ```./dtdump```. It will dump the stream to ```dtdump-<time stamp>.wav```
(12 channels, 32 Bit integer, 48kHz srate) until you press Ctrl-C.

# Building/running on Raspbian
install deps: ```sudo apt install libsndfile1-dev libusb-1.0-0-dev cmake 
build-essential git```
Has to be run as root in default Raspbian (lite) installation.

![Raspberry recording](pics/rpi.png?raw=true "Raspberry recording")

# Storage requirements
About 2.3MB/s, seems to be no problem with a class 10 SD card on a Rpi3.

LICENSE: MIT

Stefan Rehm, 2019
droelfdroelf ( at ) gmail.com
