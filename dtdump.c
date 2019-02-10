/*

 dtdump
 ----------------------------------------------------------------
 An open Overbridge protocol implementation proof of concept.
 Dumps all channels to a multi channel WAV file.
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

// for O_DIRECT
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sndfile.h>
#include <time.h>
#include <fcntl.h>

int shtdwn = 0;

static libusb_device_handle* digitakt;

static uint8_t dummy_out_data[2112] = { 0 };
static uint8_t in_data[8832] = { 0 };
static int32_t wav_data[2016] = { 0 };

static uint16_t dummy_timestamp = 0;

static uint32_t xruns = 0;

static struct libusb_transfer *xfr_in;
static struct libusb_transfer *xfr_out;

SNDFILE *wavfile;
int wfd;

static int prepare_cycle_in();	// forward declaration
static int prepare_cycle_out();	// forward declaration

static int prepare_transfers() {
	xfr_in = libusb_alloc_transfer(0);
	if (!xfr_in) {
		return -ENOMEM;
	}
	xfr_out = libusb_alloc_transfer(0);
	if (!xfr_out) {
		return -ENOMEM;
	}
	return LIBUSB_SUCCESS;
}

static void free_transfers() {
	libusb_free_transfer(xfr_in);
	libusb_free_transfer(xfr_out);
}

static void fill_dummy_data() {
	int offs = 0;
	while (offs < sizeof(dummy_out_data)) {
		dummy_out_data[offs] = 0x07;
		dummy_out_data[offs + 1] = 0xFF;
		dummy_out_data[offs + 2] = (dummy_timestamp >> 8) & 0xFF;
		dummy_out_data[offs + 3] = (dummy_timestamp) & 0xFF;
		offs += 88;	// block length
		dummy_timestamp += 7;
	}
}

#define DT_VID 0x1935
#define DT_PID 0x000c

// initialization taken from sniffed session

int digitakt_init(libusb_device_handle* dt) {
	int ret;
	ret = libusb_set_configuration(dt, 1);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't set configuration 1!\n");
		return LIBUSB_ERROR_OTHER;
	}

	ret = libusb_set_configuration(dt, 1);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't set configuration 1 again!\n");
		return LIBUSB_ERROR_OTHER;
	}

	ret = libusb_claim_interface(dt, 2);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't clain IF 2!\n");
		return LIBUSB_ERROR_OTHER;
	}

	ret = libusb_claim_interface(dt, 1);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't clain IF 1!\n");
		return LIBUSB_ERROR_OTHER;
	}

	ret = libusb_set_interface_alt_setting(dt, 2, 2);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't set alt setting on dev 2!\n");
		return LIBUSB_ERROR_OTHER;
	}
	ret = libusb_set_interface_alt_setting(dt, 1, 3);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't set alt setting on dev 1!\n");
		return LIBUSB_ERROR_OTHER;
	}
	ret = libusb_clear_halt(dt, 131);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't clear halt ep 131!\n");
		return LIBUSB_ERROR_OTHER;
	}
	ret = libusb_clear_halt(dt, 3);
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't clear halt ep 3!\n");
		return LIBUSB_ERROR_OTHER;
	}

	ret = prepare_transfers();
	if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't prepare transfer!\n");
		return LIBUSB_ERROR_OTHER;
	}
	return LIBUSB_SUCCESS;
}

static void usb_shutdown() {
	if (digitakt) {
		libusb_close(digitakt);
	}
	free_transfers();
	libusb_exit(NULL);
}

static void save_data() {
	// store sample data in tmp buf
	uint32_t len = sizeof(in_data) / 4;
	uint32_t pos = 0;
	uint32_t wav_pos = 0;
	int i;
	static int first = 1;
	static uint16_t lastts;
	uint16_t ts;
	ts = ntohs(((uint16_t*)in_data)[1]);
	if(first)
	{
		first = 0;
	}
	else {
		if (((uint16_t)(ts-lastts)) > 168){
			xruns ++;
		}
	}
	lastts = ts;

	while (pos < len) {
		pos += 0x08;	// skip header (8 * uint32_t)
		// wav is LE
		for (i = 0; i < (12 * 7); i++) {
			wav_data[wav_pos++] = ntohl(((uint32_t*) in_data)[i + pos]);
		}
		pos += (12 * 7);
	}
}

static int receive_done = 0;

static void LIBUSB_CALL cb_xfr_in(struct libusb_transfer *xfr) {
	if (xfr->status == LIBUSB_TRANSFER_COMPLETED) {
		save_data();
		receive_done = 1; // mark receive done
	} else {
		printf("x");
	}
	// start new cycle even if this one did not succeed
	prepare_cycle_in();
}

static void LIBUSB_CALL cb_xfr_out(struct libusb_transfer *xfr) {
	// We have to make sure that the out cycle is always started after its callback
	// Race condition on slower systems!
	prepare_cycle_out();
}

static int prepare_cycle_out() {
	fill_dummy_data();
	libusb_fill_interrupt_transfer(xfr_out, digitakt, 0x03, dummy_out_data,
			sizeof(dummy_out_data), cb_xfr_out, NULL, 100);
	int r;
	r = libusb_submit_transfer(xfr_out);
	return r;
}

// sends  (dummy) data to the dt and receives
static int prepare_cycle_in() {
	libusb_fill_interrupt_transfer(xfr_in, digitakt, 0x83, in_data,
			sizeof(in_data), cb_xfr_in, NULL, 100);
	int r;
	r = libusb_submit_transfer(xfr_in);
	return r;
}

static void sighandler(int s) {
	printf("Shutting down ...\n");
	shtdwn = 1;
}

int main(int argc, char *argv[]) {

	// sighandler setup
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sighandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	// libusb setup
	int r;
	r = libusb_init(NULL);
	if (r != LIBUSB_SUCCESS) {
		printf("Could not init libusb!\n");
		goto doexit;
	}

	// wav file setup
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));
	sfinfo.channels = 12;
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
	sfinfo.samplerate = 48000;
	if (!sf_format_check(&sfinfo)) {
		printf("format check failed\n");
		goto doexit;
	};
	char wavfilename[200];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(wavfilename, sizeof(wavfilename) - 1, "obdump_%Y%m%d-%H%M%S.wav",
			t);

	wfd = open(wavfilename, O_WRONLY);
	wavfile = sf_open_fd(wfd, SFM_WRITE, &sfinfo, 1);
	fcntl(wfd, F_SETFL, O_DIRECT);

	int pid = fork();
	if (pid < 0) {
		printf("fork failed\n");
		goto doexit;
	} else {
		if (pid == 0) {
			// child
		} else {
			// parent
		}
	}


	// digitakt/ob setup
	digitakt = libusb_open_device_with_vid_pid(NULL, DT_VID, DT_PID);
	if (digitakt) {
		r = digitakt_init(digitakt);
		if (r != LIBUSB_SUCCESS) {
			printf("Could not init Digitakt!\n");
			goto doexit;
		}
	} else {
		printf("No Digitakt found!\n");
		goto doexit;
	}



	// prepare transfers
	prepare_cycle_out();
	prepare_cycle_in();
	printf("Recording to %s, Ctrl-C to stop.\n", wavfilename);

	// main loop
	unsigned long written_bytes = 0;
	while (!shtdwn) {
		libusb_handle_events(NULL);
		if (receive_done) {
			written_bytes += sf_write_int(wavfile, wav_data,
					sizeof(wav_data) / 4) * 4;
			receive_done = 0;
			printf("\r%i kB - xrun: %i", written_bytes / 1024, xruns);
			// printf("# %i\n", count);
		}
	}
	printf("\r\n\n");
	// gracefully shut down everything
	doexit: sf_write_sync(wavfile);
	sf_close(wavfile);
	usb_shutdown();
	return 0;
}
