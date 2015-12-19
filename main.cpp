#include "gamepad.h"
#include "uinput.h"

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

static const int gpToInputKeyMap[BUTTON_COUNT] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0, 0, KEY_PAGEUP, KEY_PAGEDOWN};

int main(int argc, char * argv[])
{
	int err;
//	jack_init();
	GamepadInit();
//	std::thread thread;
//	Gamepad::init();
//	Gamepad gp0(0);
	Reporter reporter;
	reporter.registerKey(KEY_UP);
	reporter.registerKey(KEY_DOWN);
	reporter.registerKey(KEY_LEFT);
	reporter.registerKey(KEY_RIGHT);
	reporter.registerKey(KEY_PAGEDOWN);
	reporter.registerKey(KEY_PAGEUP);
	reporter.start();
	while (1) {
//		Gamepad::update();
		GamepadUpdate();
//		update(GAMEPAD_0);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
//		std::cout << gp0.isConnected() << std::endl;

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
						reporter.reportKey(gpToInputKeyMap[j], value);
						if (j == BUTTON_START && value) {
							reporter.stop();
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
				for (j = 0; j != STICK_COUNT; ++j) {
					for (int k = 0; k != STICKDIR_COUNT; ++k) {
						GamepadSetRumble(static_cast<GAMEPAD_DEVICE>(i), 0.5, 0.5);
						if (GamepadStickDirTriggered(	static_cast<GAMEPAD_DEVICE>(i),
														static_cast<GAMEPAD_STICK>(j),
														static_cast<GAMEPAD_STICKDIR>(k))) {
//							logevent("[%d] stick direction:  %d -> %d", i, j, k);
//							printf("haha");
						}
					}
				}
				float f = GamepadStickAngle(static_cast<GAMEPAD_DEVICE>(i), STICK_LEFT);
				float f2 = GamepadStickLength(static_cast<GAMEPAD_DEVICE>(i), STICK_LEFT);
				int x,y = 0;
				GamepadStickXY(static_cast<GAMEPAD_DEVICE>(i), STICK_LEFT, &x, &y);
//				printf("angle: %d %d %f %f\n", x, y, f, f2);

			}
		}

	}
	return 0;
}
