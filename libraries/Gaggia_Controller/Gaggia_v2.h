#ifndef Gaggia_v2_h
#define Gaggia_v2_h

#include "Arduino.h"

class Gaggia
{
	public:
		Gaggia(int PUMPPIN, int HEATPIN, int HEATWINDOW);
		void Gaggia::Control(int HEATLEVEL, byte BREWTIME);
		void Brew(void);		
		

	private:
		int _pumpPin, _heatPin;
		bool _heat_phase, _heat_state, _pump_state;
		int _heat_duration, _heat_window;
		byte _brew_duration;
		unsigned long _phase_time;
		unsigned long _pump_start_time;
		
		void _heater_on(void);
		void _heater_off(void);
		void _pump_on(void);
		void _pump_off(void);
		int _pre_brew_duration;
		int _pre_brew_cycle;
		bool _pre_brew_finished;

		
};


#endif
