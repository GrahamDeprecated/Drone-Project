/*
 ProblemTest - a cut-down version to try to show the problem more clearly
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define NUM_PINS 100
#define ARD_PINS 13
#define SP_CHR_0 (char)40
#define SP_CHR_1 (char)41
#define SP_CHR_2 (char)42
#define SP_CHR_3 (char)43
#define SP_CHR_4 (char)44
#define MAX17043_ADDRESS 0x36  // R/W =~ 0x6D/0x6C

#include "C:\Users\Peter\Documents\GitHub\Drone\Shifter\Shifter.h"
#include "C:\Users\Peter\Documents\GitHub\Drone\Shifter\Shifter.cpp"
#include <..\..\..\..\..\libraries\Wire\Wire.h>

class digi_pins
{
	public:
		digi_pins(Shifter *shift);
		digi_pins(Shifter *shift, String do_not_change);
		digi_pins *set(short pin_id, bool value /*false=low*/, bool write_out=true);
		void write();
		bool read(short pin_id);
		digi_pins *setio(short pin_id, bool input);
	private:
		bool _pinvals[NUM_PINS];
		bool _shift_change;
		String _no_changes;
		Shifter *_shifts;
		//Shifter _shifts(SHIFT_SERIAL, SHIFT_LATCH, SHIFT_CLOCK, 1 /*Number of shift regiusters in chain*/);
};
digi_pins::digi_pins(Shifter *shift)
{
	digi_pins(shift,"");
}
digi_pins::digi_pins(Shifter *shift, String do_not_change)
{

	for (int x=0; x < NUM_PINS; x++)
	{
		_pinvals[x]=false; // Set all low
	}
	_no_changes=do_not_change+",";
	//shifts=shift;
	_shifts=shift;

	_shifts->clear();
	_shifts->write();
}
digi_pins *digi_pins::set(short pin_id, bool ishigh /*false=low*/, bool write_out)
{
	if ((pin_id < NUM_PINS) && (_pinvals[pin_id] != ishigh))
	{
		_pinvals[pin_id]=ishigh;

		if (write_out)
		{
			write();
		}
	}
	return this;
}
void digi_pins::write()
{
		for (int j=0; j < NUM_PINS; j++)
		{
			if (j <= ARD_PINS)
			{
				if (_no_changes.indexOf(String(j)) == -1)
				{
					digitalWrite(j,_pinvals[j]);
				}
			}
			else if (j >= 60)
			{
				if (_shift_change)
				{
					_shifts->setPin(j-60, _pinvals[j]);
				}
			}
		}
		if (_shift_change)
		{
			_shifts->write();
		}
//	#endif
}
bool digi_pins::read(short pin_id)
{
	if (pin_id >= 60)
	{
		return false;
	}
	return (digitalRead(pin_id) == 1);
}
digi_pins *digi_pins::setio(short pin_id, bool input)
{
	if (pin_id < 60)
	{
		if (input)
		{
			pinMode(pin_id, INPUT);
			_no_changes+=String(pin_id)+",";
		}
		else
		{
			pinMode(pin_id, OUTPUT);
			_no_changes.replace(String(pin_id)+",", "");
		}
	}
	return this;
}
class digi_batt
{
	public:
		// CGRAM addresses 0-4 will be used
		digi_batt(digi_pins *pins, short alert_pin, short alert_percent, void (*onalert)());
		digi_batt(digi_pins *pins, short alert_pin, short alert_percent);
		void init(digi_pins *pins, short alert_pin, short alert_percent);
		char current_char(bool drone);
		static void update(float per);
	private:
		static unsigned int vcellMAX17043();
		static float		percentMAX17043();
		static void			configMAX17043(byte percent);
		static void			qsMAX17043();
		static unsigned int i2cRead16(unsigned char address);
		static void			i2cWrite16(unsigned int data, unsigned char address);

		static void			alert_inter();

		char			_current_drone;
		static char		_current_contr;
		short			_alert_pin;
		digi_pins		*_pins;
		static void		(*_onalert)();
		static float	_percentage_contr;
		static float	_voltage_contr;
		static short	_bound_ch0;
		static short	_bound_ch1;
		static short	_bound_ch2;
		static short	_bound_ch3;
		static short	_bound_ch4;
		//Timer			_timer;
};
char		digi_batt::_current_contr;
float		digi_batt::_percentage_contr;
float		digi_batt::_voltage_contr;
short		digi_batt::_bound_ch0;
short		digi_batt::_bound_ch1;
short		digi_batt::_bound_ch2;
short		digi_batt::_bound_ch3;
short		digi_batt::_bound_ch4;
void		(*digi_batt::_onalert)();

			digi_batt::digi_batt(digi_pins *pins, short alert_pin, short alert_percent, void (*onalert)())
{
	_onalert=onalert;
	init(pins, alert_pin, alert_percent);
}
			digi_batt::digi_batt(digi_pins *pins, short alert_pin, short alert_percent)
{
	_onalert=NULL;
	init(pins, alert_pin, alert_percent);
}
void		digi_batt::init(digi_pins *pins, short alert_pin, short alert_percent)
{
	_alert_pin=alert_pin;
	_pins=pins;
	_current_drone=SP_CHR_2;
	_current_contr=SP_CHR_4;

	_pins->setio(alert_pin,true);
	_pins->set(alert_pin,true);
	attachInterrupt(alert_pin,alert_inter,FALLING);

	Wire.begin();  // Start I2C
	delay(100);
	configMAX17043(alert_percent);  // Configure the MAX17043's alert percentage
	qsMAX17043();  // restart fuel-gauge calculations

	// Arrange symbol boundaries
	short ap=alert_percent;
	_bound_ch0=ap;					// 1 bar
	_bound_ch1=ap + (100-ap)/4 * 1;	// 2 bar
	_bound_ch2=ap + (100-ap)/4 * 2;	// 3 bar
	_bound_ch3=ap + (100-ap)/4 * 3;	// 4 bar
	_bound_ch4=ap + (100-ap)/4 * 4;	// 5 bar

	//_timer.every(5000,update);
}
void		digi_batt::alert_inter()
{
	_current_contr='*';


	if (_onalert != NULL)
	{
		_onalert();
	}
}
void		digi_batt::update(float per)
{
	_percentage_contr = per;/*percentMAX17043();*/
	//_voltage_contr = (float) vcellMAX17043() * 1/800;  // vcell reports battery in 1.25mV increments

	short pcnt=_percentage_contr;
	if		((pcnt <= _bound_ch4) && (pcnt > _bound_ch3))	{ _current_contr=SP_CHR_4; }
	else if ((pcnt <= _bound_ch3) && (pcnt > _bound_ch2))	{ _current_contr=SP_CHR_3; }
	else if ((pcnt <= _bound_ch2) && (pcnt > _bound_ch1))	{ _current_contr=SP_CHR_2; }
	else if ((pcnt <= _bound_ch1) && (pcnt > _bound_ch0))	{ _current_contr=SP_CHR_1; }
	else													{ _current_contr=SP_CHR_0; }
}
char		digi_batt::current_char(bool drone)
{
	if (drone)
	{
		return _current_drone;
	}
	else
	{
		return _current_contr;
	}
}
/*
vcellMAX17043() returns a 12-bit ADC reading of the battery voltage,
as reported by the MAX17043's VCELL register.
This does not return a voltage value. To convert this to a voltage,
multiply by 5 and divide by 4096.
*/
unsigned int digi_batt::vcellMAX17043()
{
	unsigned int vcell;
	vcell = i2cRead16(0x02);
	vcell = vcell >> 4;  // last 4 bits of vcell are nothing
	return vcell;
}
/*
percentMAX17043() returns a float value of the battery percentage
reported from the SOC register of the MAX17043.
*/
float		digi_batt::percentMAX17043()
{
	unsigned int soc;
	float percent;
	soc = i2cRead16(0x04);  // Read SOC register of MAX17043
	percent = (byte) (soc >> 8);  // High byte of SOC is percentage
	percent += ((float)((byte)soc))/256;  // Low byte is 1/256%
	return percent;
}
/* 
configMAX17043(byte percent) configures the config register of
the MAX170143, specifically the alert threshold therein. Pass a 
value between 1 and 32 to set the alert threshold to a value between
1 and 32%. Any other values will set the threshold to 32%.
*/
void		digi_batt::configMAX17043(byte percent)
{
	if ((percent >= 32)||(percent == 0))  // Anything 32 or greater will set to 32%
	{
		i2cWrite16(0x9700, 0x0C);
	}
	else
	{
		byte percentBits = 32 - percent;
		i2cWrite16((0x9700 | percentBits), 0x0C);
	}
}
/* 
qsMAX17043() issues a quick-start command to the MAX17043.
A quick start allows the MAX17043 to restart fuel-gauge calculations
in the same manner as initial power-up of the IC. If an application's
power-up sequence is very noisy, such that excess error is introduced
into the IC's first guess of SOC, the Arduino can issue a quick-start
to reduce the error.
*/
void		digi_batt::qsMAX17043()
{
	i2cWrite16(0x4000, 0x06);  // Write a 0x4000 to the MODE register
}
/* 
i2cRead16(unsigned char address) reads a 16-bit value beginning
at the 8-bit address, and continuing to the next address. A 16-bit
value is returned.
*/
unsigned int digi_batt::i2cRead16(unsigned char address)
{
	int data = 0;
	Wire.beginTransmission(MAX17043_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();
  
	Wire.requestFrom(MAX17043_ADDRESS, 2);
	while (Wire.available() < 2)
		;
	data = ((int) Wire.read()) << 8;
	data |= Wire.read();

	return data;
}
/*
i2cWrite16(unsigned int data, unsigned char address) writes 16 bits
of data beginning at an 8-bit address, and continuing to the next.
*/
void		digi_batt::i2cWrite16(unsigned int data, unsigned char address)
{
	Wire.beginTransmission(MAX17043_ADDRESS);
	Wire.write(address);
	Wire.write((byte)((data >> 8) & 0x00FF));
	Wire.write((byte)(data & 0x00FF));
	Wire.endTransmission();
}


Shifter shifts(10,11,12,1);
digi_pins pins(&shifts);
digi_batt batt(&pins, 2, 20);
void setup()
{

  /* add setup code here */

}

void loop()
{

  /* add main program code here */

}
