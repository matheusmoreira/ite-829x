// SPDX-License-Identifier: GPL-2.0-only

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
int set_brightness(hid_device *keyboard, unsigned char brightness)
{
	if (!keyboard)
		return -1;

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

	return hid_send_feature_report(keyboard, report, sizeof(report));
}

/* Clevo Control Center
 * Speed
 * Wireshark Leftover Capture Data
 *
 * 	1	cc090a0000007f
 * 	2	cc090a0100007f
 * 	3	cc090a0200007f
 */
int set_speed(hid_device *keyboard, unsigned char speed)
{
	if (!keyboard)
		return -1;

	if (speed > 0x02)
		speed = 0x02;

	const unsigned char report[] = {
		0xCC, 0x09,
		0x0A, speed,
		0x00, 0x00,
		0x7F
	};

	return hid_send_feature_report(keyboard, report, sizeof(report));
}

/* Executes the requested operation on the given keyboard.
 * Returns 0 if successful and 1 in case of failure.
 */
int ite_829x(hid_device *keyboard, char *command, char *parameter)
{

	int (*set)(hid_device *, unsigned char) = set_brightness;
	unsigned char value = 0;
	const char *name = "brightness";

	switch (*command) {
	case '\0':
		break;
	case 'b':
		value = atoi(parameter);
		break;
	default:
		value = atoi(command);
		break;
	case 's':
		set = set_speed;
		value = atoi(parameter);
		name = "speed";
		break;
	}

	if (set(keyboard, value) == -1) {
		fprintf(stderr, "Could not set %s to %hhu - %ls\n",
			name, value, hid_error(keyboard));
		return 1;
	}

	return 0;
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

	if (count != 3) {
		fprintf(stderr, "%s command parameter\n", *arguments);
		return 3;
	}

	int code = ite_829x(keyboard, arguments[1], arguments[2]);

	hid_close(keyboard);

	if (hid_exit() == -1) {
		fputs("Error during hidapi-libusb finalization\n", stderr);
		if (code == 0)
			return 5;
	}

	return code;
}
