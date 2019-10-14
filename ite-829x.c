// SPDX-License-Identifier: GPL-2.0-only

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <hidapi/hidapi.h>

#define VID 0x048d /* Integrated Technology Express, Inc. */
#define PID 0x8910 /* ITE Device(829x) */

/* Clevo Control Center
 * Brightness
 * Wireshark Leftover Capture Data
 *
 * 	Off	cc09000000007f
 * 	  1	cc09020200007f
 * 	  2	cc09040200007f
 * 	  3	cc09060200007f
 * 	  4	cc090a0200007f
 *
 * Other brightness values are also valid: 1, 3, 5, 7, 8, 9.
 * Values above 0x0A seem to have the same effect as 0x0A.
 * Seems to be a scale from 1 to 10 with 0 being off.
 *
 * The byte that follows the brightness value differs in the captured output.
 * However, it doesn't seem to matter. During testing:
 *
 * 	cc09000200007f	turns the LEDs off
 * 	cc090a0000007f	turns the LEDs on at brightness 10
 *
 * It's purpose is unknown. The code reproduces the captured data.
 */
unsigned int set_brightness(const size_t count, const char **arguments, void *context)
{
	hid_device *keyboard = context;
	if (!keyboard)
		return 3;

	if (count < 1)
		return 2;

	unsigned char brightness = atoi(*arguments);

	if (brightness > 0x0A)
		brightness = 0x0A;

	unsigned char unknown = 0x00;

	if (brightness > 0)
		unknown = 0x02;

	const unsigned char report[] = {
		0xCC, 0x09,
		brightness, unknown,
		0x00, 0x00,
		0x7F
	};

	if (hid_send_feature_report(keyboard, report, sizeof(report)) == -1)
		return 1;

	return 0;
}

/* Clevo Control Center
 * Speed
 * Wireshark Leftover Capture Data
 *
 * 	1	cc090a0000007f
 * 	2	cc090a0100007f
 * 	3	cc090a0200007f
 */
unsigned int set_speed(const size_t count, const char **arguments, void *context)
{
	hid_device *keyboard = context;
	if (!keyboard)
		return 3;

	if (count < 1)
		return 2;

	unsigned char speed = atoi(*arguments);

	if (speed > 0x02)
		speed = 0x02;

	const unsigned char report[] = {
		0xCC, 0x09,
		0x0A, speed,
		0x00, 0x00,
		0x7F
	};

	if (hid_send_feature_report(keyboard, report, sizeof(report)) == -1)
		return 1;

	return 0;
}

/* Clevo Control Center
 * Effects
 * Wireshark Leftover Capture Data
 *
 * 	0	Wave    	cc00040000007f
 * 	1	Breathe 	cc0a000000007f
 * 	2	Scan    	cc000a0000007f
 * 	3	Blink   	cc0b000000007f
 * 	4	Random  	cc000900000000
 * 	5	Ripple  	cc070000000000
 * 	6	Snake   	cc000b00000053
 *
 * There seems to be no pattern to it. Does the value of the last byte matter?
 * My keyboard apparently doesn't support the ripple effect,
 * even though it is present in the Clevo Control Center interface.
 */
unsigned int set_effects(const size_t count, const char **arguments, void *context)
{
	hid_device *keyboard = context;
	if (!keyboard)
		return 3;

	if (count < 1)
		return 2;

	const unsigned char effect = atoi(*arguments);

	unsigned char effect1, effect2, last = 0x7F;

	switch (effect) {
	default:
		return -2;
	case 0:
		effect1 = 0x00;
		effect2 = 0x04;
		break;
	case 1:
		effect1 = 0x0A;
		effect2 = 0x00;
		break;
	case 2:
		effect1 = 0x00;
		effect2 = 0x0A;
		break;
	case 3:
		effect1 = 0x0B;
		effect2 = 0x00;
		break;
	case 4:
		effect1 = 0x00;
		effect2 = 0x09;
		last  = 0x00;
		break;
	case 5:
		effect1 = 0x07;
		effect2 = 0x00;
		last  = 0x00;
		break;
	case 6:
		effect1 = 0x00;
		effect2 = 0x0B;
		last  = 0x53;
		break;
	}

	const unsigned char report[] = {
		0xCC,
		effect1, effect2,
		0x00, 0x00, 0x00,
		last
	};

	if (hid_send_feature_report(keyboard, report, sizeof(report)) == -1)
		return 1;

	return 0;
}

/* Resets the keyboard back to a clean state.
 * Turns off all LEDs and clears their color configuration.
 * Stops any keyboard effects.
 *
 * Clevo Control Center sends this report when the user switches to normal mode
 * from effects mode. It follows up with over a hundred reports
 * that reconfigure the LED colors for each individual key.
 *
 * 	cc000c0000007f
 *
 */
unsigned int reset(const size_t count, const char **arguments, void *context)
{
	hid_device *keyboard = context;
	if (!keyboard)
		return 3;

	const unsigned char report[] = {
		0xCC,
		0x00, 0x0C,
		0x00, 0x00, 0x00,
		0x7F
	};

	if (hid_send_feature_report(keyboard, report, sizeof(report)) == -1)
		return 1;

	return 0;
}

/* Clevo Control Center
 * RGB color of LEDs
 * Wireshark Leftover Capture Data
 *
 * 	cc01000000007f
 * 	cc01llrrggbb7f
 *
 * ll = which LED to configure
 * rr = red
 * gg = green
 * bb = blue
 *
 * LED codes:
 *
 *   0 - 19 	Escape -> PageDown
 *  32 - 43 	Tilde -> Minus
 *  45 - 51 	Equals -> KeypadMinus
 *  64 - 83 	Tab1 -> KeypadPlus1
 *  96 - 108	CapsLock1 -> SingleQuote
 * 110 - 115	Enter1 -> KeypadPlus2
 * 128      	Shift1
 * 130 - 147	Shift2 -> KeypadEnter1
 * 160 - 165	Ctrl1 -> Spacebar1
 * 169 - 179	Spacebar2 -> KeypadEnter2
 *
 * 102 keys.
 * 114 LEDs.
 *
 * Sets the color of each individual LED.
 * Some keys are lit by more than one LED.
 * All of them can be controlled individually.
 * After resetting the keyboard, all keys must be reconfigured.
 */
unsigned int set_led_color(const size_t count, const char **arguments, void *context)
{
	hid_device *keyboard = context;
	if (!keyboard)
		return 3;

	if (count < 4)
		return 2;

	const unsigned char led = atoi(arguments[0]);
	const unsigned char r   = atoi(arguments[1]);
	const unsigned char g   = atoi(arguments[2]);
	const unsigned char b   = atoi(arguments[3]);

	const unsigned char report[] = {
		0xCC,
		0x01, led,
		r, g, b,
		0x7F
	};

	if (hid_send_feature_report(keyboard, report, sizeof(report)) == -1)
		return 1;

	return 0;
}

#include "cmd.c"

/* Executes the requested operation on the given keyboard.
 * Returns 0 if successful and 1 in case of failure.
 */
int ite_829x(hid_device *keyboard, FILE *input)
{
	struct command ite_829x_commands[] = {
		{ "brightness", set_brightness, keyboard },
		{ "speed",      set_speed,      keyboard },
		{ "effects",    set_effects,    keyboard },
		{ "reset",      reset,          keyboard },
		{ "led",        set_led_color,  keyboard },
		{ 0 }
	};

	switch (process_command_file(ite_829x_commands, input)) {
	case -2:
		fputs("Memory allocation error\n", stderr);
		return 4;
	case -1:
		fputs("Unknown command\n", stderr);
		return 3;
	case 0:
		return 0; // Success.
	case 1:
		fprintf(stderr,
		        "Could not send feature report: %ls\n",
		        hid_error(keyboard));
		return 1;
	case 2:
		fputs("Incorrect number of parameters\n", stderr);
		return 3;
	case 3:
		fputs("NULL keyboard hid_device\n", stderr);
		return 4;
	default:
		return -1; // Unknown return value.
	}
}

int main(int count, char **arguments)
{
	if (hid_init() == -1) {
		fputs("Error during hidapi-libusb initialization\n", stderr);
		return 4;
	}

	hid_device *keyboard = hid_open(VID, PID, NULL);
	if (keyboard == NULL) {
		fprintf(stderr, "Could not open keyboard [%04x:%04x]\n", VID, PID);
		return 2;
	}

	int code = ite_829x(keyboard, stdin);

	hid_close(keyboard);

	if (hid_exit() == -1) {
		fputs("Error during hidapi-libusb finalization\n", stderr);
		if (code == 0)
			return 4;
	}

	return code;
}
