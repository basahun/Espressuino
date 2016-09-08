#include "Arduino.h"
#include "Gaggia_v2.h"

Gaggia::Gaggia(int PUMPPIN, int HEATPIN, int HEATWINDOW)
{
	_pumpPin = PUMPPIN;
	_heatPin = HEATPIN;
	pinMode(_pumpPin,OUTPUT);
	pinMode(_heatPin,OUTPUT);
	digitalWrite(_pumpPin,0);
	digitalWrite(_heatPin,0);
	_heat_phase = true;
	_heat_state = false;
	_heat_duration = 0;
	_heat_window = HEATWINDOW;
	_phase_time = 0;
	_pump_state = false;
	_brew_duration = 27;
	_pre_brew_duration = 100;
	_pre_brew_cycle = 5;
	_pre_brew_finished = false;
	
	
}

void Gaggia::Control(int HEATLEVEL, byte BREWTIME) 
{
	//int _heat_level = LEVEL;
	//if( _heat_level<0 ) { _heat_level = 0; }
	//if( _heat_level>100 ) { _heat_level = 100; }
	//_heat_duration = int((_heat_level * _heat_window ) / 100);
	_brew_duration = BREWTIME;
	_heat_duration = HEATLEVEL;
	
	int _pause_time = _heat_window-_heat_duration;
	
	// Boiler Control
	
	if( _heat_phase && ! _heat_state && _heat_duration > 30 && millis() > _phase_time ) 
	{
		_heater_on();
		_phase_time = millis()+_heat_duration;
	}
	
	if(_heat_phase && _heat_state && millis() > _phase_time )
	{
		if( _pause_time > 30 )
			{
			_heater_off();			
			_phase_time = millis()+_pause_time;			
			_heat_phase = false;
			}
		else
			{
			_phase_time = millis()+_heat_phase;
			}
	}
	
	if( ! _heat_phase && millis() > _phase_time ) 
	{
		_heat_phase = true;
		
	}
	
	// Pump Control
	
	if( _pump_state && millis > _pump_start_time + (_brew_duration*1000))
	{
		_pump_off();
	}
	
}

void Gaggia::Brew(void)
{
	if( ! _pump_state && _pre_brew_finished)
	{
		_pump_on();
		_pump_start_time = millis();
	}
	else
	{
		if( millis() > _pump_start_time + 500 )
		{
			_pump_off();
		}
	}
}


void Gaggia::_heater_on(void)
{
	digitalWrite(_heatPin,1);
	_heat_state = true;
}

void Gaggia::_heater_off(void)
{
	digitalWrite(_heatPin,0);
	_heat_state = false;
}

void Gaggia::_pump_on(void)
{
	digitalWrite(_pumpPin,1);
}

void Gaggia::_pump_off(void)
{
	digitalWrite(_pumpPin,0);
}
