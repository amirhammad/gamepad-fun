#pragma once
namespace std {
	class thread;
}
class Gamepad
{
public:
	Gamepad(int gamepadId);
	~Gamepad();

	bool isConnected() const;
	static void init();


private:
	static void update();
	static void threadFunction();

	int m_gamepadId;
	static std::thread *s_thread;

};

