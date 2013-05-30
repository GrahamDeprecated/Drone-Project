/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// See here for bit changing: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c
#define USEWIRE 1 /*0 for on*/

#include "digi_write.h"
#include "statics.h"

#include "Shifter/Shifter.h"
#include "Shifter/Shifter.cpp"
#include "QueueList/QueueList.h"
#include "Timer/Timer.h"
#include "Timer/Event.h"
#include "Timer/Timer.cpp"
#include "Timer/Event.cpp"

int				change_bit(int val, short bit_num, bool bitval)
{
    return (val & ~(1 << bit_num)) | (bitval << bit_num);
}
// b0=MSB
uint8_t			bits2int(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7)
{
	// Uses LSB first
	uint8_t ret=0;
	ret=change_bit(ret,7,b0);
	ret=change_bit(ret,6,b1);
	ret=change_bit(ret,5,b2);
	ret=change_bit(ret,4,b3);
	ret=change_bit(ret,3,b4);
	ret=change_bit(ret,2,b5);
	ret=change_bit(ret,1,b6);
	ret=change_bit(ret,0,b7);
	return ret;
}
// start with MSB
unsigned int	bits2int(bool *bitarr, int num_to_pass, int offset)
{
	int ret=0;
	int total=num_to_pass;
	for (; num_to_pass > 0; num_to_pass--)
	{
		ret=change_bit(ret, num_to_pass-1, bitarr[(offset + (total - num_to_pass)) ]);
	}
	return ret;
}
void			int2bits(unsigned int start, int num_to_pass, bool *out, int offset)
{
	for (int x=num_to_pass; x > 0; x--)
	{
		out[offset + x -1]= (start & ( 1 << (num_to_pass-x) )) >> (num_to_pass-x);
	}
}

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
	_shifts=shift;

	_shifts->clear();
	_shifts->write();

	#ifdef need_tune_support
		tune_init();
	#endif
}
digi_pins	*	digi_pins::set(short pin_id, bool ishigh /*false=low*/, bool write_out)
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
void			digi_pins::write()
{
		for (int j=0; j < NUM_PINS; j++)
		{
			if (j <= NUM_DIGITAL_PINS)
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
bool			digi_pins::read(short pin_id)
{
	if (pin_id >= 60)
	{
		return false;
	}
	return (digitalRead(pin_id) == 1);
}
bool			digi_pins::interrupt(short pin, void (*func)(), short type)
{
	short inter=pin-2;
	if (func == NULL)
	{
		detachInterrupt(inter);
		return true;
	}
	else
	{
		if (((String)ARD_INTERRUPTS).indexOf((String)pin) == -1)
		{
			return false;
		}
		else
		{
			attachInterrupt(inter, func, type);
			return true;
		}
	}
		
}
digi_pins	*	digi_pins::setio(short pin_id, bool input)
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
#ifdef need_tune_support
#include <pnew.cpp>
void			digi_pins::tune_init()
{
	#ifdef need_dads_army
		_tunes["Dad's Army"]=(String)"250 A4;2 C4;1 D4;1 E4;2 E4;1 F4;1 A4;1 G4;1 A4;1 G4;1 A4;1 G4;2" +
			/*Bar 9 :*/	" A4;1 G4;1 Gb4;1 G4;1 A4;2 G4;2 C4;6 RT;2 G4;2 F4;1 E4;1" +
			/*Bar 14:*/ " G4;2 F4;1 D4;1 F4;1 E4;1 E4;1 Eb4;1 E4;4 A4;2 G4;1 Gb4;1 G4;2 A4;1 B4;1" +
			/*Bar 19:*/ " D5;1 C5;1 C5;1 B4;1 C5;3 Ab4;1 A4;2 C4;1 D4;1 E4;1 E4;1 F4;1 A4;1 G4;1 A4;1 G4;1" +
			/*Bar 24:*/ " A4;1 G4;3 E5;1 D5;1 C5;1 B4;1 Bb4;2 E4;2 F4;5 RT;3";
			#ifndef need_oct_4
			#define need_oct_4
			#endif
			#ifndef need_oct_5
			#define need_oct_5
			#endif
	#endif
	#ifdef need_god_save_the_queen
		_tunes["God Save the Queen"]=(String)"125 Eb3;4 Eb3;4 F3;4 D3;6 Eb3;2 F3;4 G3;4 G3;4 Ab3;4 G3;6 F3;2 Eb3;4" +
			/*Bar 5 :*/ " F3;4 Eb3;4 D3;4 Eb3;4 Eb3;2 F3;2 G3;2 Ab3;2 Bb3;4 Bb3;4 Bb3;4 Bb3;6 Ab3;2 G3;4" +
			/*Bar 9 :*/ " Ab3;4 Ab3;4 Ab3;4 Ab3;6 G3;2 Eb3;4 G3;4 Ab3;2 G3;2 Eb3;2 F3;2 G3;6 Ab3;2 Bb3;4" +
			/*Bar 13:*/ " C4;2 Bb3;1 Ab3;1 G3;4 Eb3;4 F3;12";
			#ifndef need_oct_3
			#define need_oct_3
			#endif
			#ifndef need_oct_4
			#define need_oct_4
			#endif
	#endif
	// Only include required octaves
	#ifdef need_oct_0
	_notes["C0"]=  16;
	_notes["Db0"]= 17;
	_notes["D0"]=  18;
	_notes["Eb0"]= 19;
	_notes["E0"]=  21;
	_notes["F0"]=  22;
	_notes["Gb0"]= 23;
	_notes["G0"]=  25;
	_notes["Ab0"]= 26;
	_notes["A0"]=  28;
	_notes["Bb0"]= 29;
	_notes["B0"]=  31;
	#endif
	#ifdef need_oct_1
	_notes["C1"]=  33;
	_notes["Db1"]= 35;
	_notes["D1"]=  38;
	_notes["Eb1"]= 39;
	_notes["E1"]=  41;
	_notes["F1"]=  44;
	_notes["Gb1"]= 46;
	_notes["G1"]=  49;
	_notes["Ab1"]= 52;
	_notes["A1"]=  55;
	_notes["Bb1"]= 58;
	_notes["B1"]=  62;
	#endif
	#ifdef need_oct_2
	_notes["C2"]=  65;
	_notes["Db2"]= 69;
	_notes["D2"]=  73;
	_notes["Eb2"]= 78;
	_notes["E2"]=  82;
	_notes["F2"]=  87;
	_notes["Gb2"]= 93;
	_notes["G2"]=  98;
	_notes["Ab2"]= 104;
	_notes["A2"]=  110;
	_notes["Bb2"]= 117;
	_notes["B2"]=  123;
	#endif
	#ifdef need_oct_3
	_notes["C3"]=  131;
	_notes["Db3"]= 139;
	_notes["D3"]=  147;
	_notes["Eb3"]= 156;
	_notes["E3"]=  165;
	_notes["F3"]=  175;
	_notes["Gb3"]= 185;
	_notes["G3"]=  196;
	_notes["Ab3"]= 208;
	_notes["A3"]=  220;
	_notes["Bb3"]= 233;
	_notes["B3"]=  247;
	#endif
	#ifdef need_oct_4
	_notes["C4"]=  262;
	_notes["Db4"]= 277;
	_notes["D4"]=  294;
	_notes["Eb4"]= 311;
	_notes["E4"]=  330;
	_notes["F4"]=  349;
	_notes["Gb4"]= 370;
	_notes["G4"]=  392;
	_notes["Ab4"]= 415;
	_notes["A4"]=  440;
	_notes["Bb4"]= 466;
	_notes["B4"]=  494;
	#endif
	#ifdef need_oct_5
	_notes["C5"]=  523;
	_notes["Db5"]= 554;
	_notes["D5"]=  587;
	_notes["Eb5"]= 622;
	_notes["E5"]=  660;
	_notes["F5"]=  698;
	_notes["Gb5"]= 740;
	_notes["G5"]=  784;
	_notes["Ab5"]= 831;
	_notes["A5"]=  880;
	_notes["Bb5"]= 932;
	_notes["B5"]=  988;
	#endif
	#ifdef need_oct_6
	_notes["C6"]=  1047;
	_notes["Db6"]= 1109;
	_notes["D6"]=  1175;
	_notes["Eb6"]= 1245;
	_notes["E6"]=  1319;
	_notes["F6"]=  1397;
	_notes["Gb6"]= 1480;
	_notes["G6"]=  1568;
	_notes["Ab6"]= 1661;
	_notes["A6"]=  1760;
	_notes["Bb6"]= 1865;
	_notes["B6"]=  1976;
	#endif
	#ifdef need_oct_7
	_notes["C7"]=  2093;
	_notes["Db7"]= 2217;
	_notes["D7"]=  2349;
	_notes["Eb7"]= 2489;
	_notes["E7"]=  2637;
	_notes["F7"]=  2794;
	_notes["Gb7"]= 2960;
	_notes["G7"]=  3136;
	_notes["Ab7"]= 3322;
	_notes["A7"]=  3520;
	_notes["Bb7"]= 3729;
	_notes["B7"]=  3951;
	#endif
	#ifdef need_oct_8
	_notes["C8"]=  4186;
	_notes["Db8"]= 4435;
	_notes["D8"]=  4699;
	_notes["Eb8"]= 4978;
	#endif
}
void			digi_pins::tune_worker()
{
	if ((_tuneindex+1) < _tunes[_tunename].length())
	{
		ushort tmpindex=_tunes[_tunename].indexOf(' ', _tuneindex);
		_tunetmpstr=_tunes[_tunename].substring(_tuneindex, tmpindex);
		ushort semi=_tunetmpstr.indexOf(";");
			
		//Serial.println(_tunetmpstr.substring(0, semi) + " for " + _tunetmpstr.substring(semi +1) + " index=" + _tuneindex + " adding:" + _tunetmpstr.length());
		Serial.println((String)tmpindex + "  " + _tunetmpstr + "   " + semi + "   " + _tunetmpstr.substring(0, semi) + "   " + _tunetmpstr.substring(semi +1));
		if (_tunetmpstr.substring(0, semi) == "RT")
		{
			noTone(_tunepin);
		}
		else
		{
			tone(_tunepin, _notes[_tunetmpstr.substring(0, semi)]);
		}
		_tunetimer->after(_tune_delay_ms * _tunetmpstr.substring(semi +1).toInt(),tune_worker);
		_tuneindex=tmpindex/*+ _tunetmpstr.length() -2*/;
	}
	else
	{
		noTone(_tunepin);
		_tuneindex=0;
	}
}
void			digi_pins::playtune(String tune_name, int pin, Timer *timer)
{
	_tunename=tune_name;
	_tunepin=pin;
	_tunetimer=timer;
	_tuneindex=_tunes[_tunename].indexOf(' ') + 1;
	_tune_delay_ms=_tunes[_tunename].substring(0, _tuneindex -1).toInt();
	tune_worker();
}
#endif

				digi_serial::digi_serial(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter, short tx_inter, bool activate_now)
{
	_tx_inter=tx_inter;
	setup(pins_class, tx_pin, rx_pin, rx_inter);
	if (activate_now) { activate(); }
}
				digi_serial::digi_serial(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter, bool activate_now)
{
	setup(pins_class, tx_pin, rx_pin, rx_inter);
	if (activate_now) { activate(); }
}
void			digi_serial::setup(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter)
{
	_pins=pins_class;
	_tx_pin=tx_pin;
	_rx_pin=rx_pin;
	_rx_inter=rx_inter;
	_write_busy=false;
}
void			digi_serial::activate()
{
	_pins->interrupt(_rx_inter,this->inter,RISING);
}
void			digi_serial::write(int data)
{
	if (_write_busy == false)
	{
		_write_busy=true;
		for (unsigned short mask = 0x80; mask != 0; mask >>= 1)
		{
			_pins->set(_tx_pin,!(data & mask),false);
			_pins->set(_tx_pin,true,true);
			/*if (data & mask)
			{
				// bit is 1
			}
			else
			{
				// bit is 0
			}*/
		}
		_write_busy=false;
	}
	else
	{
		delayMicroseconds(1); // wait for other operation to complete
		write(data);
	}
}
void			digi_serial::write(String data)
{
	for (unsigned short y=0; y < data.length(); y++)
	{
		write(data[y]);
	}
}
void			digi_serial::inter()
{
	/*_in_buf_pos++;
	_in_buffer[_in_buf_pos]=_pins->read(_rx_pin);*/
	_in_queue.push(_pins->read(_rx_pin));
}
void			digi_serial::read(int *container, short num_bits)
{
	for (short n=0; n < num_bits; n++)
	{
		*container |= _in_queue.pop() << n; // Set nth bit of container to the value in the queue
	}
}
void			digi_serial::read(String *container, short num_chars=0) // Null-terminated string
{
	for (int ch; ch != '\0'; read(&ch, 8))
	{
		container+=ch;
	}
	container+='\0';
}

#if (DRONE == 0)
#else
	// Crack to make "Wire" object work. It probably wasn't linking properly, but this seems to fix it
	extern "C"
	{
		#include <twi.h>
		#include <twi.c>
	}
	#include <Wire.h>
	#include <Wire.cpp>
				digi_batt::digi_batt(digi_pins *pins, short alert_pin, void (*onalert)())
	{
		_onalert=onalert;
		init(pins, alert_pin);
	}
				digi_batt::digi_batt(digi_pins *pins, short alert_pin)
	{
		_onalert=NULL;
		init(pins, alert_pin);
	}
	void		digi_batt::init(digi_pins *pins, short alert_pin)
	{
		_alert_pin=alert_pin;
		_pins=pins;
		_current_drone='?';
		_current_contr='?';

		_pins->setio(alert_pin,true);
		_pins->set(alert_pin,true);
		//attachInterrupt(alert_pin,alert_inter,FALLING);
		_pins->interrupt(alert_pin,alert_inter,FALLING);
	}
	void		digi_batt::setup(short alert_percent)
	{
		#if (USEWIRE == 0)
		_pins->set(13,true); delay(1000); _pins->set(13,false);
			Wire.begin();  // Start I2C
			delay(100);
			_pins->set(13,true); delay(1000); _pins->set(13,false);
		#endif
		configMAX17043(alert_percent);  // Configure the MAX17043's alert percentage
		qsMAX17043();  // restart fuel-gauge calculations

		// Arrange symbol boundaries
		short ap=alert_percent;
		_bound_ch0=ap;					// 1 bar
		_bound_ch1=ap + (100-ap)/4 * 1;	// 2 bar
		_bound_ch2=ap + (100-ap)/4 * 2;	// 3 bar
		_bound_ch3=ap + (100-ap)/4 * 3;	// 4 bar
		_bound_ch4=ap + (100-ap)/4 * 4;	// 5 bar

		update();
		_timer.every(5000,update);
	}
	void		digi_batt::alert_inter()
	{
		_current_contr='*';


		if (_onalert != NULL)
		{
			_onalert();
		}
	}
	void		digi_batt::update()
	{
		_percentage_contr = percentMAX17043();
		_voltage_contr = (float) vcellMAX17043() * 1/800;  // vcell reports battery in 1.25mV increments

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
		#if (USEWIRE == 0)
			unsigned int vcell;
			vcell = i2cRead16(0x02);
			vcell = vcell >> 4;  // last 4 bits of vcell are nothing
			return vcell;
		#else
			return 12960;
		#endif
	}
	/*
	percentMAX17043() returns a float value of the battery percentage
	reported from the SOC register of the MAX17043.
	*/
	float		digi_batt::percentMAX17043()
	{
		#if (USEWIRE == 0)
			unsigned int soc;
			float percent;
			soc = i2cRead16(0x04);  // Read SOC register of MAX17043
			percent = (byte) (soc >> 8);  // High byte of SOC is percentage
			percent += ((float)((byte)soc))/256;  // Low byte is 1/256%
			return percent;
		#else
			int x=millis()/600;
			if (x > 100) { return 100; } else { return x; }
		#endif
	}
	/* 
	configMAX17043(byte percent) configures the config register of
	the MAX170143, specifically the alert threshold therein. Pass a 
	value between 1 and 32 to set the alert threshold to a value between
	1 and 32%. Any other values will set the threshold to 32%.
	*/
	void		digi_batt::configMAX17043(byte percent)
	{
		#if (USEWIRE == 0)
			if ((percent >= 32)||(percent == 0))  // Anything 32 or greater will set to 32%
			{
				i2cWrite16(0x9700, 0x0C);
			}
			else
			{
				byte percentBits = 32 - percent;
				i2cWrite16((0x9700 | percentBits), 0x0C);
			}
		#endif
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
		#if (USEWIRE == 0)
			i2cWrite16(0x4000, 0x06);  // Write a 0x4000 to the MODE register
		#endif
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


#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

				digi_lcd::digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(pins_class, batt, 0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}
				digi_lcd::digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(pins_class, batt, 0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}
				digi_lcd::digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	init(pins_class, batt, 1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}
				digi_lcd::digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs,  uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	init(pins_class, batt, 1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}
void			digi_lcd::init(digi_pins *pins_class, digi_batt *batt, uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	_pins_class=pins_class;
	_batt=batt;
	_rs_pin = rs;
	_rw_pin = rw;
	_enable_pin = enable;
	_data_set=true; // allows switch_data() to work
  
	_data_pins[0] = d0;
	_data_pins[1] = d1;
	_data_pins[2] = d2;
	_data_pins[3] = d3; 
	_data_pins[4] = d4;
	_data_pins[5] = d5;
	_data_pins[6] = d6;
	_data_pins[7] = d7; 

  //---pinMode(_rs_pin, OUTPUT);
	_pins_class->setio(_rs_pin,false);
	// we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
	if (_rw_pin != 255)
	{
		//---pinMode(_rw_pin, OUTPUT);
		_pins_class->setio(_rw_pin,false);
	}
  //---pinMode(_enable_pin, OUTPUT);
	_pins_class->setio(_enable_pin,false);
  
	if (fourbitmode)
		_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	else 
		_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
	begin(16, 1);

	// Create battery chars----------------------------------------
	// 0=1bar, 1=2bar, 2=3bar, 3=4bar, 4=5bar
	uint8_t nchar[8];
	// 1st 3 bits are irrelevant - 5x8 pixels
	nchar[0]=bits2int(0,0,0, 0,1,1,1,0);
	nchar[1]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[2]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[3]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[4]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[5]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[6]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[7]=bits2int(0,0,0, 1,1,1,1,1);
	createChar(SP_CHR_0 - 8,nchar);

	nchar[0]=bits2int(0,0,0, 0,1,1,1,0);
	nchar[1]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[2]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[3]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[4]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[5]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[6]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[7]=bits2int(0,0,0, 1,1,1,1,1);
	createChar(SP_CHR_1 - 8,nchar);

	nchar[0]=bits2int(0,0,0, 0,1,1,1,0);
	nchar[1]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[2]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[3]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[4]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[5]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[6]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[7]=bits2int(0,0,0, 1,1,1,1,1);
	createChar(SP_CHR_2 - 8,nchar);

	nchar[0]=bits2int(0,0,0, 0,1,1,1,0);
	nchar[1]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[2]=bits2int(0,0,0, 1,0,0,0,1);
	nchar[3]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[4]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[5]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[6]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[7]=bits2int(0,0,0, 1,1,1,1,1);
	createChar(SP_CHR_3 - 8,nchar);

	nchar[0]=bits2int(0,0,0, 0,1,1,1,0);
	nchar[1]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[2]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[3]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[4]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[5]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[6]=bits2int(0,0,0, 1,1,1,1,1);
	nchar[7]=bits2int(0,0,0, 1,1,1,1,1);
	createChar(SP_CHR_4 - 8,nchar);
}
digi_lcd	*	digi_lcd::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
	if (lines > 1)
	{
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;
	_currline = 0;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1))
	{
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delayMicroseconds(50000); 
	// Now we pull both RS and R/W low to begin commands
		//---digitalWrite(_rs_pin, LOW);
	_pins_class->set(_rs_pin,false,false);
		//---digitalWrite(_enable_pin, LOW);
	_pins_class->set(_enable_pin,false,false);
	if (_rw_pin != 255)
	{
			//---digitalWrite(_rw_pin, LOW);
		_pins_class->set(_rw_pin,false,false);
	}
	_pins_class->write();
  
	//put the LCD into 4 bit or 8 bit mode
	if (! (_displayfunction & LCD_8BITMODE))
	{
		// this is according to the hitachi HD44780 datasheet
		// figure 24, pg 46

		// we start in 8bit mode, try to set 4 bit mode
		write4bits(0x03);
		delayMicroseconds(4500); // wait min 4.1ms

		// second try
		write4bits(0x03);
		delayMicroseconds(4500); // wait min 4.1ms
    
		// third go!
		write4bits(0x03); 
		delayMicroseconds(150);

		// finally, set to 4-bit interface
		write4bits(0x02); 
	}
	else
	{
		// this is according to the hitachi HD44780 datasheet
		// page 45 figure 23

		// Send function set command sequence
		command(LCD_FUNCTIONSET | _displayfunction);
		delayMicroseconds(4500);  // wait more than 4.1ms

		// second try
		command(LCD_FUNCTIONSET | _displayfunction);
		delayMicroseconds(150);

		// third go
		command(LCD_FUNCTIONSET | _displayfunction);
	}

	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for romance languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	return this;
}

/********** high level commands, for the user! */
digi_lcd	*	digi_lcd::clear()
{
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
	return this;
}
digi_lcd	*	digi_lcd::home()
{
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
	return this;
}
digi_lcd	*	digi_lcd::setCursor(uint8_t col, uint8_t row)
{
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row >= _numlines )
	{
		row = _numlines-1;    // we count rows starting w/0
	}
  
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
	return this;
}
// Turn the display on/off (quickly)
digi_lcd	*	digi_lcd::noDisplay()
{
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
digi_lcd	*	digi_lcd::display()
{
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
// Turns the underline cursor on/off
digi_lcd	*	digi_lcd::noCursor()
{
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
digi_lcd	*	digi_lcd::cursor()
{
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
// Turn on and off the blinking cursor
digi_lcd	*	digi_lcd::noBlink()
{
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
digi_lcd	*	digi_lcd::blink()
{
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
	return this;
}
// These commands scroll the display without changing the RAM
digi_lcd	*	digi_lcd::scrollDisplayLeft(void)
{
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
	return this;
}
digi_lcd	*	digi_lcd::scrollDisplayRight(void)
{
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
	return this;
}
// This is for text that flows Left to Right
digi_lcd	*	digi_lcd::leftToRight(void)
{
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
	return this;
}
// This is for text that flows Right to Left
digi_lcd	*	digi_lcd::rightToLeft(void)
{
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
	return this;
}
// This will 'right justify' text from the cursor
digi_lcd	*	digi_lcd::autoscroll(void)
{
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
	return this;
}
// This will 'left justify' text from the cursor
digi_lcd	*	digi_lcd::noAutoscroll(void)
{
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
	return this;
}
// Allows us to fill the first 8 CGRAM locations
// with custom characters
digi_lcd	*	digi_lcd::createChar(uint8_t location, uint8_t charmap[])
{
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++)
	{
		write(charmap[i]);
	}
	return this;
}

/*********** mid level commands, for sending data/cmds */
inline void		digi_lcd::command(uint8_t value)
{
	send(value, LOW);
}
inline size_t	digi_lcd::write(uint8_t value)
{
	send(value, HIGH);
	return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void			digi_lcd::send(uint8_t value, uint8_t mode)
{
		//---digitalWrite(_rs_pin, mode);
	_pins_class->set(_rs_pin,(mode == HIGH),false);
	// if there is a RW pin indicated, set it low to Write
	if (_rw_pin != 255)
	{
			//---digitalWrite(_rw_pin, LOW);
		_pins_class->set(_rw_pin,false);
	}
	_pins_class->write();
  
	if (_displayfunction & LCD_8BITMODE)
	{
		write8bits(value); 
	}
	else
	{
		write4bits(value>>4);
		write4bits(value);
	}
}
void			digi_lcd::pulseEnable(void)
{
		//---digitalWrite(_enable_pin, LOW);
	_pins_class->set(_enable_pin,false);
	delayMicroseconds(1);
		//---digitalWrite(_enable_pin, HIGH);
	_pins_class->set(_enable_pin,true);
	delayMicroseconds(1);    // enable pulse must be >450ns
		//---digitalWrite(_enable_pin, LOW);
	_pins_class->set(_enable_pin,false);
	delayMicroseconds(100);   // commands need > 37us to settle
}
void			digi_lcd::write4bits(uint8_t value)
{
	for (int i = 0; i < 4; i++)
	{
			//---pinMode(_data_pins[i], OUTPUT);
		_pins_class->setio(_data_pins[i],false);
			//---digitalWrite(_data_pins[i], (value >> i) & 0x01);
		_pins_class->set(_data_pins[i],(value >> i) & 0x01);
	}

	pulseEnable();
}
void			digi_lcd::write8bits(uint8_t value)
{
	for (int i = 0; i < 8; i++)
	{
			//---pinMode(_data_pins[i], OUTPUT);
		_pins_class->setio(_data_pins[i],false);
			//---digitalWrite(_data_pins[i], (value >> i) & 0x01);
		_pins_class->set(_data_pins[i],(value >> i) & 0x01);
	}
  
  pulseEnable();
}

/************ extra functions written by me *************/
digi_lcd	*	digi_lcd::write_row(String text,bool bottom_row,bool print_batt)
{
	_rows[_data_set][bottom_row]=text;
	_print_batt[bottom_row]=print_batt;
	update();
	return this;
}
digi_lcd	*	digi_lcd::update()
{
	for (short n=0; n < 2; n++)
	{
		setCursor(0,n);
		if (_print_batt[n])
		{
			print(pad_str(_rows[_data_set][n],15) + _batt->current_char(n));
		}
		else
		{
			print(pad_str(_rows[_data_set][n],16));
		}
	}
	return this;
}
digi_lcd	*	digi_lcd::switch_data()
{
	_data_set=!_data_set;
	update();
	return this;
}
String			digi_lcd::pad_str(String start,int length)
{
	int str_len=start.length();
	if (str_len > length)
	{
		return start.substring(0,length-1);
	}
	else if (str_len == length)
	{
		return start;
	}
	else
	{
		for (int n=(length-str_len); n != 0; n--)
		{
			start+=" ";
		}
		return start;
	}
}
digi_lcd	*	digi_lcd::addbuzzer(short pin)
{
	_buzzer=pin;
	_pins_class->setio(_buzzer,false);
	return this;
}
digi_lcd	*	digi_lcd::buzz(int ms_on, int ms_off, int num_repeats)
{
	for (int n=0; n < num_repeats; n++)
	{
		_pins_class->set(_buzzer, true);
		delay(ms_on);
		_pins_class->set(_buzzer, false);
		delay(ms_off);
	}
	return this;
}
digi_lcd	*	digi_lcd::buzz(int tune_id)
{
	switch (tune_id)
	{

	}
	return this;
}
#endif

				digi_rf::digi_rf(digi_pins *pins, int in_d0, int in_d1, int in_d2, int in_d3, int in_inter, int out_d0, int out_d1, int out_d2, int out_d3)
{
	_pins=pins;
}
#if (DRONE == 0)
	void		digi_rf::activate(void (*onready)(bool gps_needed))
	{

	}
	void		digi_rf::negotiate()
	{

	}
#else
	void		digi_rf::activate(void (*onready)(), digi_lcd *screen, bool gps_needed)
	{
		_onload=onready;
		_screen=screen;
		_gps_needed=gps_needed;
		_pins->interrupt(_inter_in,negotiate,RISING);
	}
	void		digi_rf::negotiate()
	{
		switch (_neg_status) // also represents # of transmission
		{
			case 1:
				{
					// Initial "Hi" message
					read4(_neg_tmp_data,&_neg_status_2);
					if ((_neg_tmp_data[0] == true) && (_neg_tmp_data[1] == true) && (_neg_tmp_data[2] == false) && (_neg_tmp_data[3] == true))
					{
						bool resp[4]={ true, false, false, true };
						write4(resp);
						_neg_status=2;
						_neg_status_2=0;
					}
					break;
				}
			case 2:
				{
					// Do you need GPS?
					read4(_neg_tmp_data,&_neg_status_2);
					if ((_neg_tmp_data[0] == true) && (_neg_tmp_data[1] == false) && (_neg_tmp_data[2] == true) && (_neg_tmp_data[3] == true))
					{
						write4( (_gps_needed ? rf_yes : rf_no) );
					}
					_neg_status=3;
					_neg_status_2=0;
					break;
				}
			case 3:
			case 4:
				{
					// XOR system to check data is ok
					read4(_neg_tmp_data,&_neg_status_2);
					if (_neg_status_2 == 16) //transmission complete
					{
						bool out[16];
						int2bits(0b1110010011010110 ^ bits2int(_neg_tmp_data,16), 16, out);  //binary only works in gcc, but we're using it
						for (int off=0; off < 16;)
						{
							write4(out,&off);
						}
						_neg_status++;
						_neg_status_2=0;
					}
					break;
				}
			case 5:
				{
					// Decision from drone about matching data
					read4(_neg_tmp_data,&_neg_status_2);
					if ((_neg_tmp_data[0] == rf_yes[0]) && (_neg_tmp_data[1] == rf_yes[1]) && (_neg_tmp_data[2] == rf_yes[2]) && (_neg_tmp_data[3] == rf_yes[3]))
					{
						// Communication is up and running
						_onload();
					}
					else
					{
						// It didn't work. Throw error
						_screen->write_row("Coms error!",false) ->write_row("Restart Drone",true);
						_neg_status=1; // Reset process for re-start of drone
						_neg_status_2=0;
					}
					break;
				}
		}
	}
#endif
void			digi_rf::read4(bool *out, int *offset)
{
	for (int x=0; x < 4; x++)
	{
		*(out + *offset)=_pins->read(_in_pins[x]);
		(*offset)++;
	}
}
void			digi_rf::write4(bool *in, int *offset)
{
	for (int x=0; x < 4; x++)
	{
		_pins->set(_out_pins[x],*(in + *offset),false);
		(*offset)++;
	}
	_pins->write();
}
void			digi_rf::write4(bool *in)
{
	for (int x=0; x < 4; x++)
	{
		_pins->set(_out_pins[x],*(in + x),false);
	}
	_pins->write();
}
