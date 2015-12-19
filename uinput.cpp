#include "uinput.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>

void Reporter::reportSync()
{
	struct input_event ev;

	memset(&ev, 0, sizeof(ev));

	ev.type = EV_SYN;

	int ret = write(m_fd, &ev, sizeof(ev));
}

Reporter::Reporter()
{
	m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(m_fd < 0) {
		fprintf(stderr, "CANNOT OPEN UINPUT\n");
		exit(EXIT_FAILURE);
	}
	int ret = ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
	ret = ioctl(m_fd, UI_SET_EVBIT, EV_SYN);
	ret = ioctl(m_fd, UI_SET_EVBIT, EV_REL);
}

Reporter::~Reporter()
{

}


void Reporter::reportKey(int code, bool value)
{
	printf("report key: %d %d\n", code, value);
	struct input_event ev;

	memset(&ev, 0, sizeof(ev));

	ev.type = EV_KEY;
	ev.code = code;
	ev.value = value ? 1 : 0;

	int ret = write(m_fd, &ev, sizeof(ev));
	reportSync();
}

void Reporter::registerKey(int code)
{
	int ret = ioctl(m_fd, UI_SET_KEYBIT, code);
}

void Reporter::reportRelative(int X, int Y)
{
	struct input_event ev[2];

	memset(&ev, 0, sizeof(ev));

	ev[0].type = EV_REL;
	ev[0].code = REL_X;
	ev[0].value = X;
	ev[1].type = EV_REL;
	ev[1].code = REL_Y;
	ev[1].value = Y;

	int ret = write(m_fd, &ev, sizeof(ev));
	reportSync();
}

void Reporter::registerRelative(int code)
{
	int ret = ioctl(m_fd, UI_SET_RELBIT, code);
}

void Reporter::start()
{
	struct uinput_user_dev uidev;

	memset(&uidev, 0, sizeof(uidev));

	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "gamepad");
	uidev.id.bustype = BUS_VIRTUAL;
	uidev.id.vendor  = 0x1234;
	uidev.id.product = 0xfedc;
	uidev.id.version = 1;
	int ret = write(m_fd, &uidev, sizeof(uidev));

	ret = ioctl(m_fd, UI_DEV_CREATE);
}

void Reporter::stop()
{
	int ret = ioctl(m_fd, UI_DEV_DESTROY);
}
