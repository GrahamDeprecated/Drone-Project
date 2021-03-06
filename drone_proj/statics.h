/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define rf_yes	0xF /*1111*/
bool rf_yes[]={ true ,true ,true ,true };
//#define rf_no	0x9 /*1001*/
bool rf_no[]= { true ,false,false,true };

#ifdef need_tune_support
	Timer	*	digi_pins::_tunetimer;
	unsigned char digi_pins::_tunepin;
	ushort		digi_pins::_tuneindex;
	ushort		digi_pins::_tune_delay_ms;
	String		digi_pins::_tunename;
	String		digi_pins::_tunetmpstr;
	bool		digi_pins::_play_tone;
	std::map<String,String> digi_pins::_tunes /*={{"Dad's Army", "250 A4;2 C4;1 D4;1 E4;2 E4;1"}}   - can do if we find libc++ for avr...*/;
	std::map<String,ushort> digi_pins::_notes  /*={{"A4",440}, {"C4",262}, {"D4",294}, {"E4",330}}   - can do if we find libc++ for avr...*/;
#endif

digi_pins*	digi_serial::_pins;
short		digi_serial::_tx_pin;
short		digi_serial::_rx_pin;
short		digi_serial::_tx_inter;
short		digi_serial::_rx_inter;
bool		digi_serial::_write_busy;
QueueList<bool> digi_serial::_in_queue;

#if (DRONE == 0)
#else
char		digi_batt::_current_contr;
float		digi_batt::_percentage_contr;
float		digi_batt::_voltage_contr;
short		digi_batt::_bound_ch0;
short		digi_batt::_bound_ch1;
short		digi_batt::_bound_ch2;
short		digi_batt::_bound_ch3;
short		digi_batt::_bound_ch4;
void		(*digi_batt::_onalert)();
digi_pins	*digi_batt::_pins;
#endif

// My special chars locations
#define SP_CHR_0 (char)8
#define SP_CHR_1 (char)9
#define SP_CHR_2 (char)10
#define SP_CHR_3 (char)11
#define SP_CHR_4 (char)12
#define SP_CHR_5 (char)13
#define SP_CHR_6 (char)14
#define SP_CHR_7 (char)15
digi_pins	*	digi_rf::_pins;
void		(*	digi_rf::_onload)();
int				digi_rf::_inter_in;
int				digi_rf::_neg_status=1;
int				digi_rf::_neg_status_2=0;
bool			digi_rf::_neg_tmp_data[20];
int				digi_rf::_in_pins[4];
int				digi_rf::_out_pins[4];
#if (DRONE == 1)
digi_lcd	*	digi_rf::_screen;
bool			digi_rf::_gps_needed;
#else
#endif