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

Just run ```./dtdump```. It will dump the stream to ```dtdump-<time stamp>.wav```
(12 channels, 32 Bit integer, 48kHz srate) until you press Ctrl-C.

LICENSE: MIT

Stefan Rehm, 2019
droelfdroelf ( at ) gmail.com
