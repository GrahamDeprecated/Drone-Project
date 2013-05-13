/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define NUM_PINS 100
#define ARD_PINS 13
//#define SER_BUF_BITS 8*20
#include "QueueList\QueueList.h"
#include "Shifter\Shifter.h"
#include "Timer\Timer.h"

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
class digi_serial
{
	public:
		digi_serial(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter, short tx_inter, bool activate_now=true);
		digi_serial(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter, bool activate_now=true);
		void activate();
		void write(int data);
		void write(String data);
		static void inter();
		void read(int *container, short num_bits);
		void read(String *container, short num_chars); // Null-terminated string
	private:
		void setup(digi_pins* pins_class, short tx_pin, short rx_pin, short rx_inter);
		static digi_pins* _pins;
		static short _tx_pin, _rx_pin, _tx_inter, _rx_inter;
		static bool _write_busy;
		static QueueList<bool> _in_queue;
		//bool _in_buffer[SER_BUF_BITS];
		//int _in_buf_pos;
};
#if (DRONE == 0)
	;
#else
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
#endif

// Adapted from LiquidCrystal.h (mostly copied)
#include <inttypes.h>
#include "Print.h"

#include "LCD_defs.h"

class digi_lcd : public Print
{
	public:
		digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
		digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
		digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
		digi_lcd(digi_pins *pins_class, digi_batt *batt, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
		void init(digi_pins *pins_class, digi_batt *batt, uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
		void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

		void clear();
		void home();

		void noDisplay();
		void display();
		void noBlink();
		void blink();
		void noCursor();
		void cursor();
		void scrollDisplayLeft();
		void scrollDisplayRight();
		void leftToRight();
		void rightToLeft();
		void autoscroll();
		void noAutoscroll();

		void createChar(uint8_t, uint8_t[]);
		void setCursor(uint8_t, uint8_t); 
		virtual size_t write(uint8_t);
		void command(uint8_t);

		digi_lcd *write_row(String text,bool top_row,bool print_batt=true);
		String pad_str(String start,int length);
  
		using Print::write;
	private:
		void send(uint8_t, uint8_t);
		void write4bits(uint8_t);
		void write8bits(uint8_t);
		void pulseEnable();

		uint8_t _rs_pin; // LOW: command.  HIGH: character.
		uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
		uint8_t _enable_pin; // activated by a HIGH pulse.
		uint8_t _data_pins[8];

		uint8_t _displayfunction;
		uint8_t _displaycontrol;
		uint8_t _displaymode;

		uint8_t _initialized;

		uint8_t _numlines,_currline;

		digi_pins *_pins_class;
		digi_batt *_batt;
};
class digi_rf
{
	public:
		digi_rf(digi_pins *pins, int in_d0, int in_d1, int in_d2, int in_d3, int in_inter, int out_d0, int out_d1, int out_d2, int out_d3);
		void activate(void (*onready)(), digi_lcd *screen);
	private:
		static void negotiate();
		static void read4(bool *out, int *offset);
		static void write4(bool *in, int *offset);
		static void write4(bool *in);

		static digi_pins	*_pins;
		static digi_lcd		*_screen;
		static void			(*_onload)();
		static int			_inter_in;
		static int			_neg_status;
		static int			_neg_status_2;
		static bool			_neg_tmp_data[20];
		static int			_in_pins[4];
		static int			_out_pins[4];
};

#include "statics.h"
