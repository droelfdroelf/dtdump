/*

 dtdump
 ----------------------------------------------------------------
 An open Overbridge protocol implementation proof of concept.
 Dumps all channels to a multi channel WAV file.
 Note that everything is tested with a Digitakt,
 running OS 1.11_BETA6! Most certainly other Overbridge enabled
 devices won't work out of the box without modifications.
 See overbridge.h for protocol details.

 LICENSE: MIT

 Stefan Rehm, 2019
 droelfdroelf ( at ) gmail.com

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
#include "overbridge.h"

int shtdwn = 0;

SNDFILE *wavfile;
int wfd;



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

	overbridge_err_t ob;
	ob = overbridge_init();

	if (ob != OVERBRIDGE_OK) {
		printf("Error: %s\n", overbrigde_get_err_str(ob));
		exit(1);
	}

	// wav file setup
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));
	sfinfo.channels = 12;
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
	sfinfo.samplerate = 48000;
	if (!sf_format_check(&sfinfo)) {
		printf("format check failed\n");
	};
	char wavfilename[200];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(wavfilename, sizeof(wavfilename) - 1, "obdump_%Y%m%d-%H%M%S.wav",
			t);

	//wfd = open(wavfilename, O_WRONLY);
	wavfile = sf_open(wavfilename, SFM_WRITE, &sfinfo);
	//wavfile = sf_open_fd(wfd, SFM_WRITE, &sfinfo, 1);
	// fcntl(wfd, F_SETFL, O_DIRECT);
	printf("Recording to %s, Ctrl-C to stop.\n", wavfilename);

	overbridge_start_streaming();

	// main loop
	unsigned long written_bytes = 0;
	while (!shtdwn) {
		overbridge_do_work();
		if (overbridge_is_data_available()) {
			written_bytes += sf_write_int(wavfile, get_overbridge_wav_data(),
					get_overbridge_wav_data_size() / 4) * 4;
			printf("\r%i kB - xrun: %i", written_bytes / 1024,
				overbridge_get_xrun());
		}
	};

	printf("\r\n\n");
	sf_write_sync(wavfile);
	sf_close(wavfile);
	overbridge_shutdown();
	return 0;
}
