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
#include <jack/jack.h>
#include <jack/midiport.h>
#include "libgamepad/gamepad.h"

jack_port_t * output_port;
jack_port_t * midi_port;
jack_port_t * midiOutput_port;

jack_client_t * client;

static float freq=440;
static float phase = 0;	//0..2*M_PI
static float volume = 0.0f;
static uint32_t X = 0;
int process(jack_nframes_t nframes, void * arg)
{
	jack_position_t position;
	jack_transport_state_t  transport;
	static int tone = 0;
	float dt=2*M_PI*1.0f/jack_get_sample_rate (client);
	int i;
	jack_default_audio_sample_t * buffer = (jack_default_audio_sample_t*) jack_port_get_buffer(output_port, nframes);
	for(i=0;i<nframes;i++)
	{
		buffer[i] = volume*sinf(phase);
		phase += (freq*dt);
		while (phase > 2*M_PI) {
			phase -= 2*M_PI;
		}
	}
	//~ printf("process %f \n",buffer[0]);


	// MIDI


	 // get the port data
  void* midi_buf = jack_port_get_buffer( midi_port, nframes);

  void* midiOutput_buf = jack_port_get_buffer( midiOutput_port, nframes);


  // get the transport state of the JACK server
  transport = jack_transport_query( client, &position );

  // input: get number of events, and process them.
  jack_nframes_t event_count = jack_midi_get_event_count(midi_buf);
  if(event_count > 0)
  {
	for(int i=0; i<event_count; i++)
	{
		jack_midi_event_t in_event;
	  jack_midi_event_get(&in_event, midi_buf, i);

	  if (in_event.buffer[1] == 1) {
		  freq = ((uint8_t)in_event.buffer[2])*20.f;



	  }
	   uint8_t buf[3];
		  buf[0] = in_event.buffer[0];
		  buf[1] = in_event.buffer[1]+2;
		  buf[2] = in_event.buffer[2];
		  jack_midi_clear_buffer(midiOutput_buf);
		  jack_midi_event_write(midiOutput_buf, i*3, buf, 3);
//	  printf("process MIDI: %02X %02X %02X %i\n", in_event.buffer[0], in_event.buffer[1], in_event.buffer[2], in_event.size);
	  if (in_event.buffer[1] == 3) {
		volume = logf(in_event.buffer[2]/127.0f);
	 }
	  //~ // Using "cout" in the JACK process() callback is NOT realtime, this is
	  //~ // used here for simplicity.
	  //~ std::cout << "Frame " << position.frame << "  Event: " << i << " SubFrame#: " << in_event.time << " \tMessage:\t"
				//~ << (long)in_event.buffer[0] << "\t" << (long)in_event.buffer[1]
				//~ << "\t" << (long)in_event.buffer[2] << std::endl;
	}
  }
	return 0;
}

void jack_shutdown(void * arg)
{
	exit(-1);
}

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

static void jack_init()
{
	const char ** ports;
	const char * client_name = "simple";
	jack_options_t options = JackNullOption;
	jack_status_t status;

	client = jack_client_open(client_name, options, &status, NULL);
	if(client == NULL)
		fprintf(stderr, "ERROR: %s %d\n",__FILE__,__LINE__);
	if(status & JackServerStarted)
	{
		printf("Jack Server Started\n");
	}
	if (status & JackNameNotUnique)
	{
		printf("Client name not unique, assigned: %s\n",jack_get_client_name(client));
	}

	jack_set_process_callback(client, process, 0);
	jack_on_shutdown(client, jack_shutdown, 0);

	printf("SAMPLE RATE: %d\n", jack_get_sample_rate(client));

	midi_port = jack_port_register(client, "Input midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	midiOutput_port = jack_port_register(client, "Output midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	output_port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	//! activate JACK
	printf("ACTIVATE %d\n", jack_activate(client));

	//! connect ports
	ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if(ports==NULL)
	{
		fprintf(stderr, "ERROR %d\n", __LINE__);
	}
	uint32_t i = 0;
	while (ports[i]) {
		printf("%s\r\n", ports[i]);
		i++;
	}
	uint32_t err;
	err = jack_connect(client,  "XY-Controller:midi_out", jack_port_name (midi_port));
	err = jack_connect(client, jack_port_name (output_port), ports[0]);
	err = jack_connect(client, jack_port_name (output_port), ports[1]);
	if(err)
	{
		fprintf(stderr, "ERROR %d\n", __LINE__);
	}
//	while(1);
}

static float freq_lut[] = {
		440.0f, 466.16,493.88,523.25,554.37,587.33,622.25
};
static int mod = 0;
static int note = 0;
void noteOn(int noteId)
{
	note = noteId;
	freq = freq_lut[noteId+mod];
	volume = 1.0f;
}

static void noteSend(bool on, int noteId)
{
	const uint8_t buf[3] = {on ? 0x90 : 0x80, noteId, on ? 0x7f : 0x00};
	void* midiOutput_buf = jack_port_get_buffer( midiOutput_port, 1);
	jack_midi_clear_buffer(midiOutput_buf);
	jack_midi_event_write(midiOutput_buf, 0, buf, 3);

}

void noteOff(int noteId)
{
	if (freq_lut[noteId+mod] == freq && freq) {
		volume = 0.0f;
		freq = 0.0f;
	}
}
static void gpButtonTriggered(int buttonId)
{
	std::cout << buttonId << std::endl;
	float vol = 1.0f;
	switch (buttonId) {
	case 8:
		mod = 3 ;
		freq = freq_lut[note+mod];
//		return;
		break;
	case 12: //A
	case 13: //B
	case 14:
	case 15:
		break;

	default:
		return;
	}

//	noteOn(buttonId-12);
	noteSend(true, buttonId + mod + 50);
}

static void gpButtonReleased(int buttonId)
{
	float vol = 1.0f;
	switch (buttonId) {
	case 8:
		mod = 0;
		printf ("%d\n",note);
		freq = freq_lut[note+mod];
		return;
		break;
	case 12: //A
	case 13: //B
	case 14:
	case 15:
		break;

	default:
		return;
	}
//	volume = 1.0f;
//	noteOff(buttonId-12);
	noteSend(false, buttonId + mod + 50);
}


int main(int argc, char * argv[])
{
	int err;
	jack_init();
	GamepadInit();
//	std::thread thread;
//	Gamepad::init();
//	Gamepad gp0(0);
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
					if (GamepadButtonTriggered((GAMEPAD_DEVICE)i, (GAMEPAD_BUTTON)j)) {
						gpButtonTriggered(j);
						printf("[%d] button triggered: %s\n", i, button_names[j]);
					} else if (GamepadButtonReleased((GAMEPAD_DEVICE)i, (GAMEPAD_BUTTON)j)) {
						gpButtonReleased(j);
						printf("[%d] button released:  %s\n", i, button_names[j]);
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

	jack_client_close(client);
	return 0;
}
