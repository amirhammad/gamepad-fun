#include "gamepad.h"

#include <iostream>
#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <stdint.h>
#include <math.h>
#include "libgamepad/gamepad.h"

static const char* button_names[] = {
	"d-pad up",
	"d-pad down",
	"d-pad left",
	"d-pad right",
	"start",
	"back",
	"left thumb",
	"right thumb",
	"left shoulder",
	"right shoulder",
	"???",
	"???",
	"A",
	"B",
	"X",
	"Y"
};

int main(int argc, char * argv[])
{
	GamepadInit();
	if (!GamepadIsConnected(GAMEPAD_0)) {
		fprintf(stderr, "Gamepad is not connected\n");
		return 1;
	}
	int fd = ::open("/dev/pi-blaster", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open pi-blaster\n");
		return 1;
	}
	while (1) {
		GamepadUpdate();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		int i,j;
		for (i = 0; i != GAMEPAD_COUNT; ++i) {
			if (GamepadIsConnected(static_cast<GAMEPAD_DEVICE>(i))) {
				for (j = 0; j != BUTTON_COUNT; ++j) {
					bool value;
					bool trig = false;
					if (GamepadButtonTriggered((GAMEPAD_DEVICE)i, (GAMEPAD_BUTTON)j)) {
						printf("[%d] button triggered: %s\n", i, button_names[j]);
						value = true;
						trig = true;

					} else if (GamepadButtonReleased((GAMEPAD_DEVICE)i, (GAMEPAD_BUTTON)j)) {
						printf("[%d] button released:  %s\n", i, button_names[j]);
						value = false;
						trig = true;
					}
					if (trig) {
//						reporter.reportKey(gpToInputKeyMap[j], value);
						if (j == BUTTON_START && value) {
//							reporter.stop();
							return 0;
						}
					}
				}
				for (j = 0; j != TRIGGER_COUNT; ++j) {
					if (GamepadTriggerTriggered((GAMEPAD_DEVICE)i, (GAMEPAD_TRIGGER)j)) {
						printf("[%d] trigger pressed:  %d\n", i, j);
					} else if (GamepadTriggerReleased((GAMEPAD_DEVICE)i, (GAMEPAD_TRIGGER)j)) {
						printf("[%d] trigger released: %d\n", i, j);
					}
				}
				fflush(stdout);
//				for (j = 0; j != STICK_COUNT; ++j) {
//					for (int k = 0; k != STICKDIR_COUNT; ++k) {
//						GamepadSetRumble(static_cast<GAMEPAD_DEVICE>(i), 0.5, 0.5);
//						if (GamepadStickDirTriggered(	static_cast<GAMEPAD_DEVICE>(i),
//														static_cast<GAMEPAD_STICK>(j),
//														static_cast<GAMEPAD_STICKDIR>(k))) {
////							printf("[%d] stick direction:  %d -> %d", i, j, k);
////							printf("haha");
//						}
//					}
//				}
//				float f = GamepadStickAngle(static_cast<GAMEPAD_DEVICE>(i), STICK_LEFT);
//				float f2 = GamepadStickLength(static_cast<GAMEPAD_DEVICE>(i), STICK_LEFT);
				int x,y = 0;
				GamepadStickXY(static_cast<GAMEPAD_DEVICE>(i), STICK_RIGHT, &x, &y);
				x = -x;
				if (x >= 0) {
					const uint32_t norm = 1 << 16;
					float pwmVal = x/(float)norm;
					char setupString[128];
					const int numberOfGpioPin = 18;
					int count = sprintf(setupString, "%d=%5.4f\n", numberOfGpioPin, pwmVal);
					int ret = ::write(fd, setupString, count);
					if (ret < 0) {
						fprintf(stderr, "Error writing to pi-blaster \nerrno=%d\n", errno);
						return 1;
					}
				}
//				printf("angle: %d %d %f %f\n", x, y, (float)x/norm, (float)y/norm);

			}
		}

	}
	return 0;
}
