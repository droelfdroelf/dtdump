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
int multiplefiles = 0;

SNDFILE *wavfile;
SNDFILE *wavfiles[12];
int wfd;
uint32_t minbuffers = UINT32_MAX;

#define SYNCDELAYINMS	200
#define SYNCDELAY	((CLOCKS_PER_SEC/1000)*SYNCDELAYINMS)


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

	if (argc == 2) {
		if (!strcmp(argv[1], "-s")) {
			multiplefiles = 1;
		}
	}

	if (multiplefiles) {
		// wav file setup
		SF_INFO sfinfo;
		memset(&sfinfo, 0, sizeof(sfinfo));
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
		sfinfo.samplerate = 48000;
		if (!sf_format_check(&sfinfo)) {
			printf("format check failed\n");
		};
		char wavfilename[200];
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		for (int i = 0; i < 12; i++) {
			strftime(wavfilename, sizeof(wavfilename) - 1,
					"obdump_%Y%m%d-%H%M%S", t);
			char ext[10];
			sprintf(ext, "-%02d.wav", i);
			strcat(wavfilename, ext);
			wavfiles[i] = sf_open(wavfilename, SFM_WRITE, &sfinfo);
			printf("Recording to %s, Ctrl-C to stop.\n", wavfilename);
		}
	} else {
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
		strftime(wavfilename, sizeof(wavfilename) - 1,
				"obdump_%Y%m%d-%H%M%S.wav", t);

		wavfile = sf_open(wavfilename, SFM_WRITE, &sfinfo);

		printf("Recording to %s, Ctrl-C to stop.\n", wavfilename);
	}

	overbridge_start_streaming();

	// main loop
	unsigned long written_bytes = 0;
	clock_t oldclk = 0;
	clock_t newclk = 0;
	int32_t wavd[TRANSFER_WAV_DATA_SIZE];
	while (!shtdwn) {
		get_overbridge_wav_data(wavd);
		if (multiplefiles) {
			int32_t deinterleaved[TRANSFER_WAV_DATA_SIZE];
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 168; j++) {	// 168 samples per transfer per channel
					deinterleaved[j + (i * 168)] = wavd[(j * 12) + i];
				}
				written_bytes += (sf_write_int(wavfiles[i],
						deinterleaved + (i * 168),
						TRANSFER_WAV_DATA_SIZE / 12)) * 4;
			}

		} else {
		written_bytes += (sf_write_int(wavfile, wavd, TRANSFER_WAV_DATA_SIZE))
				* 4;
		}
		newclk = clock();
		if ((newclk - oldclk) > SYNCDELAY) {

			printf("%i kB - buff: %i - xrun: %i\n", written_bytes / 1024,
					overbridge_get_qlen(),
					overbridge_get_xrun());
			oldclk = newclk;
			if (multiplefiles) {
				for (int i = 0; i < 12; i++) {
					sf_write_sync(wavfiles[i]);
				}
			} else {
				// we sync the file in short intervals to prevent load/io/whatever spikes
				sf_write_sync(wavfile);
			}
		}
		uint32_t qlen = overbridge_get_qlen();
		if (qlen < minbuffers) {
			minbuffers = qlen;
		}
	};

	printf("\r\n\n");
	printf("Total stats: Buffers low: %i - Xruns: %i\n", minbuffers,
			overbridge_get_xrun());
	if (multiplefiles) {
		for (int i = 0; i < 12; i++) {
			sf_write_sync(wavfiles[i]);
			sf_close(wavfiles[i]);
		}
	} else {
		sf_write_sync(wavfile);
		sf_close(wavfile);
	}
	overbridge_shutdown();
	return 0;
}
