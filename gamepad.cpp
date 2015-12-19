#include "gamepad.h"

#include "libgamepad/gamepad.h"

#include <thread>
#include <chrono>

std::thread *Gamepad::s_thread;

Gamepad::Gamepad(int gamepadId)
:	m_gamepadId(gamepadId)
{

}

Gamepad::~Gamepad()
{

}

bool Gamepad::isConnected() const
{
	return GamepadIsConnected(static_cast<GAMEPAD_DEVICE>(m_gamepadId));
}

void Gamepad::init()
{
	GamepadInit();
//	s_thread = new std::thread(threadFunction);
}

void Gamepad::update()
{
	GamepadUpdate();
}

void Gamepad::threadFunction()
{
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		update();
	}
}

