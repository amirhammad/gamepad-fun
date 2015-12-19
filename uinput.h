#pragma once
#include <linux/input.h>
class Reporter {
	int m_fd;
	void reportSync();

public:
	Reporter();
	~Reporter();
	void reportKey(int code, bool value);
	void registerKey(int code);
	void start();
	void stop();
};
