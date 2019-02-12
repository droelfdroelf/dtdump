/*
 * overbridge.c
 *
 *  Created on: Feb 10, 2019
 *      Author: stefan
 */

#include "overbridge.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "message_queue.h"

#define OB_QUEUESIZE	1024

// we support only the Digitakt for now ...
#define DT_VID 0x1935
#define DT_PID 0x000c

#define TRANSFER_OUT_DATA_SIZE 2112
#define TRANSFER_IN_DATA_SIZE 8832


static libusb_device_handle* digitakt;
static uint8_t dummy_out_data[TRANSFER_OUT_DATA_SIZE] = { 0 };
static uint8_t in_data[TRANSFER_IN_DATA_SIZE] = { 0 };
int32_t overbridge_wav_data[TRANSFER_WAV_DATA_SIZE];

struct message_queue queue;

static uint16_t dummy_timestamp = 0;
static volatile uint32_t xruns = 0;

static struct libusb_transfer *xfr_in;
static struct libusb_transfer *xfr_out;

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

static void save_data() {
	// store sample data in tmp buf
	uint32_t len = sizeof(in_data) / 4;
	uint32_t pos = 0;
	uint32_t wav_pos = 0;
	int32_t* qdata;
	int i;
	static int first = 1;
	static uint16_t lastts;
	uint16_t ts;
	ts = ntohs(((uint16_t*) in_data)[1]);
	if (first) {
		first = 0;
	} else {
		if (((uint16_t) (ts - lastts)) > 168) {
			xruns++;
		}
	}
	lastts = ts;
	qdata = message_queue_message_alloc(&queue);
	if (qdata) {
	while (pos < len) {
			pos += 0x08;	// skip header (8 * uint32_t)
			// wav is LE
			for (i = 0; i < (12 * 7); i++) {
				qdata[wav_pos++] = ntohl(((uint32_t*) in_data)[i + pos]);
			}
			pos += (12 * 7);
		}
		message_queue_write(&queue, qdata);
	}
}

static int receive_done = 0;

static void LIBUSB_CALL cb_xfr_in(struct libusb_transfer *xfr) {
	if (xfr->status == LIBUSB_TRANSFER_COMPLETED) {
		save_data();
		// receive_done = 1; // mark receive done
	} else {
		//	printf("x");
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

// initialization taken from sniffed session

static int overbridge_init_priv() {
	// libusb setup
	int ret;
	ret = libusb_init(NULL);
	if (ret != LIBUSB_SUCCESS) {
		return OVERBRIDGE_LIBUSB_INIT_FAILED;
	}
	digitakt = libusb_open_device_with_vid_pid(NULL, DT_VID, DT_PID);
	if (!digitakt) {
		return OVERBRIDGE_NO_USB_DEV_FOUND;
	}
	ret = libusb_set_configuration(digitakt, 1);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_SET_USB_CONFIG;
	}
	ret = libusb_set_configuration(digitakt, 1);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_SET_USB_CONFIG;
	}
	ret = libusb_claim_interface(digitakt, 2);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_CLAIM_IF;
	}
	ret = libusb_claim_interface(digitakt, 1);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_CLAIM_IF;
	}
	ret = libusb_set_interface_alt_setting(digitakt, 2, 2);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_SET_ALT_SETTING;
	}
	ret = libusb_set_interface_alt_setting(digitakt, 1, 3);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_SET_ALT_SETTING;
	}
	ret = libusb_clear_halt(digitakt, 131);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_CLEAR_EP;
	}
	ret = libusb_clear_halt(digitakt, 3);
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_CLEAR_EP;
	}
	ret = prepare_transfers();
	if (LIBUSB_SUCCESS != ret) {
		return OVERBRIDGE_CANT_PREPARE_TRANSFER;
	}
	if (message_queue_init(&queue, sizeof(overbridge_wav_data), OB_QUEUESIZE)
			!= 0) {
		return OVERBRIDGE_CANT_SETUP_QUEUE;
	}
	return OVERBRIDGE_OK;
}

static void usb_shutdown() {
	if (digitakt) {
		libusb_close(digitakt);
	}
	free_transfers();
	libusb_exit(NULL);
}

static const char* ob_err_strgs[] = { "ok", "libusb init failed",
		"no matching usb device found", "can't set usb config",
		"can't claim usb interface", "can't set usb alt setting",
		"can't cleat endpoint", "can't prepare transfer" };

// public interface

const char* overbrigde_get_err_str(overbridge_err_t errcode) {
	return ob_err_strgs[errcode];
}

overbridge_err_t overbridge_init() {
	int r;
	r = overbridge_init_priv();
	if (r != OVERBRIDGE_OK) {
		usb_shutdown();
	}
	return r;
}

void overbridge_start_streaming() {
	prepare_cycle_out();
	prepare_cycle_in();
}

void overbridge_do_work() {
	libusb_handle_events(NULL);
}

uint32_t overbridge_get_xrun() {
	return xruns;
}
void overbridge_shutdown() {
	message_queue_destroy(&queue);
	usb_shutdown();
}

void get_overbridge_wav_data(int32_t* data) {
	int32_t* qp;
	qp = message_queue_read(&queue);
	memcpy((void*) data, (void*) qp, TRANSFER_WAV_DATA_SIZE * 4);
	message_queue_message_free(&queue, qp);
}

uint32_t get_overbridge_qlen() {
	return queue.queue.entries;
}
