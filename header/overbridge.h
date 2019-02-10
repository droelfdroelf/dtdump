/*
 * overbridge.h
 *
 An open Overbridge protocol implementation proof of concept.
 Note that everything is tested with a Digitakt,
 running OS 1.11_BETA6! Most certainly other Overbridge enabled
 devices won't work out of the box without modifications.
 LICENSE: MIT

 Stefan Rehm, 2019
 droelfdroelf ( at ) gmail.com

 Protocol details known so far
 -----------------------------

 For USB configuration and transfer setup, please read the code :)

 All values are big-endian (MSB first)!
 Sample rate is 48kHz

 Data format TO Digitakt (USB interrupt transfer, EP 0x03)
 ----------------------------------------------------------------

 Raw data of 7 blocks (total 2112 Bytes). Structure of a
 single block:

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | fixed header: 0x07FF          | sample counter (uint16_t)     |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                                                               |
 +                            unknown                            +
 ...
 +                           (28 Bytes)                          +
 |                                                               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, Master Out 1, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, Master Out 2, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, Master Out 1, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, Master Out 2, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                                                               |
 +                        samples 3 .. 7                         +
 ...
 +                     (5*8 Bytes = 40 Bytes)                    +
 |                                                               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+



 Data format FROM Digitakt (USB interrupt transfer, EP 0x83)
 ----------------------------------------------------------------

 Raw data of 7 blocks (total 8832 Bytes). Structure of a
 single block:

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | fixed header: 0x0700          | sample counter (uint16_t)     |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                                                               |
 +                            unknown                            +
 ...
 +                           (28 Bytes)                          +
 |                                                               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, Master 1/FX1, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, Master 2/FX2, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH1, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH2, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH3, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH4, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH5, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH6, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH7, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, CH8, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, EXT IN 1, int32_t                                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  1, EXT IN 2, int32_t                                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, Master 1/FX1, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, Master 2/FX2, int32_t                              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH1, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH2, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH3, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH4, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH5, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH6, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH7, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, CH8, int32_t                                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, EXT IN 1, int32_t                                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | sample  2, EXT IN 2, int32_t                                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                                                               |
 +                        samples 3 .. 7                         +
 ...
 +                     (5*48 Bytes = 240 Bytes)                  +
 |                                                               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 */

#ifndef OVERBRIDGE_H_
#define OVERBRIDGE_H_
#include "stdbool.h"
#include "stdint.h"

typedef enum {
	OVERBRIDGE_OK = 0,
	OVERBRIDGE_LIBUSB_INIT_FAILED,
	OVERBRIDGE_NO_USB_DEV_FOUND,
	OVERBRIDGE_CANT_SET_USB_CONFIG,
	OVERBRIDGE_CANT_CLAIM_IF,
	OVERBRIDGE_CANT_SET_ALT_SETTING,
	OVERBRIDGE_CANT_CLEAR_EP,
	OVERBRIDGE_CANT_PREPARE_TRANSFER
} overbridge_err_t;


int32_t* get_overbridge_wav_data();
uint32_t get_overbridge_wav_data_size();
const char* overbrigde_get_err_str(overbridge_err_t errcode);
overbridge_err_t overbridge_init();
void overbridge_start_streaming();
void overbridge_do_work();
bool overbridge_is_data_available();
uint32_t overbridge_get_xrun();
void overbridge_shutdown();

#endif /* OVERBRIDGE_H_ */
