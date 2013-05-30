/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <Arduino.h>
#include <pnew.cpp>
#include <stl_config.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "digi_write.h"
#include "notedefs.h"

Shifter shifts2(SHIFT_SERIAL, SHIFT_LATCH, SHIFT_CLOCK, 1 /*Number of shift registers in chain*/);
digi_pins pins(&shifts2,"0,1");
digi_rf rf(&pins, RF_IN_BIT_1, RF_IN_BIT_2, RF_IN_BIT_3, RF_IN_BIT_4, RF_IN_INTER, RF_OUT_BIT_1, RF_OUT_BIT_2, RF_OUT_BIT_3, RF_OUT_BIT_4);
digi_serial com(&pins, RF_OUT_BIT_1, RF_IN_BIT_1, RF_IN_INTER);

#if (DRONE == 1)
	digi_batt batt(&pins,BATT_ALERT);
	digi_lcd lcd(&pins,&batt,LCD_REG_SELECT,LCD_ENABLE,LCD_D4,LCD_D5,LCD_D6,LCD_D7);
	void rf_is_up();
	void launch();
	// Default is left (false)
	bool joystick_t_f(digi_lcd *screen, digi_pins *pins, String top_message, String options);
	void newdata();
	String tmp="";
	void newdata()
	{
		/*if ((pins.read(6) == pins.read(11)) && (pins.read(5) == pins.read(10)) && (pins.read(4) == pins.read(9)) && (pins.read(3) == pins.read(8)))
		{
			pins.set(13,true);
		}
		else
		{
			pins.set(13,false);
		}*/
		tmp+=(char)(48+pins.read(6));
		tmp+=(char)(48+pins.read(5));
		tmp+=(char)(48+pins.read(4));
		tmp+=(char)(48+pins.read(3));
		if (tmp.length() == 8)
		{
			Serial.println(tmp + " Done receiving");
			tmp="";
		}
	}

	void setup()
	{
		//lcd.addbuzzer(BUZZER);
		//lcd.buzz(1000,0,1);
		//batt.setup(MAX17043_ALERT_PERCENT);
		//lcd.begin(16,2);
		//lcd.write_row("DS CCF Drone",false) -> write_row("Initializing...",true);
		// Perform initial setup here

		//bool gps_needed=joystick_t_f(&lcd,&pins,"Enable GPS?"," No        Yes "); // Maybe add in function processing?
		//rf.activate(rf_is_up,&lcd,gps_needed); // Get it to set up the system


		//lcd.write_row("Primed",false) -> write_row("Turn on Drone",true);
		//delay(2000);
		pins.setio(6,true)->setio(5,true)->setio(4,true)->setio(3,true)->setio(2,true)->setio(11,true)->setio(10,true)->setio(9,true)->setio(8,true)->interrupt(2,newdata,RISING);
		pins.setio(13,false);
		Serial.begin(9600);
		Serial.print((char)27);
		Serial.print("[2J");
		Serial.println("Startup");
	}
	void rf_is_up()
	{
		com.activate();
		lcd.write_row("Connected",false) -> write_row("Press to launch",true);
		pins.interrupt(BUTTON_LAUNCH,launch,RISING);
	}
	void launch()
	{
		com.write("Launch");
		lcd.write_row("Launching...",true);
		String haslaunched;
		com.read(&haslaunched,8);
		lcd.write_row("Restrictions Enabled",false) -> write_row("Manual Control",true);
	}

	void loop()
	{
		/*//batt.update(); lcd.update(); delay(1000);
		char num[10];
		dtostrf((float)analogRead(0) * 0.0049,0,2,num);
		String mil(millis(),10);
		lcd.write_row((String)num + "V " + mil + "s",false); delay(10);

		batt._timer.update();*/
	}

	bool joystick_t_f(digi_lcd *screen, digi_pins *pins, String top_message, String options)
	{
		screen->switch_data() ->write_row(top_message,false) ->write_row(options,true) ->setCursor(true,1) ->cursor() ->blink();
		pinMode(JOYSTICK_1_CLICK,INPUT);
		bool ret=false;
		int an;
		while (1)
		{
			an=analogRead(JOYSTICK_1_H);
			if (an >= 816 /* 4V / 0.0049 */)
			{
				screen->setCursor(13,true);
				ret=true;
			}
			else if (an <= 204 /* 1V / 0.0049 */)
			{
				screen->setCursor(1,true);
				ret=false;
			}

			if (digitalRead(JOYSTICK_1_CLICK) == HIGH)
			{
				break;
			}
			delay(10);
		}
		screen->switch_data() ->noCursor() ->noBlink();
		return ret;
	}
#else
	//#include <initializer_list> // Allows maps to be initialised using a list in curly braces   - can do if we find libc++ for avr...
	Timer _tunetimer;
	unsigned char _tunepin;
	typedef unsigned short ushort;
	ushort _tuneindex;
	ushort _tune_delay_ms;
	String _tunename;
	String _tunetmpstr;
	std::map<String,String> tunes /*={{"Dad's Army", "250 A4;2 C4;1 D4;1 E4;2 E4;1"}}   - can do if we find libc++ for avr...*/;
	std::map<String,ushort> notes  /*={{"A4",440}, {"C4",262}, {"D4",294}, {"E4",330}}   - can do if we find libc++ for avr...*/;
	void tune_init()
	{
		tunes["Dad's Army"]=(String)"250 A4;2 C4;1 D4;1 E4;2 E4;1 F4;1 A4;1 G4;1 A4;1 G4;1 A4;1 G4;2" +
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
		tunes["God Save the Queen"]=(String)"125 Eb3;4 Eb3;4 F3;4 D3;6 Eb3;2 F3;4 G3;4 G3;4 Ab3;4 G3;6 F3;2 Eb3;4" +
			/*Bar 5 :*/ " F3;4 Eb3;4 D3;4 Eb3;4 Eb3;2 F3;2 G3;2 Ab3;2 Bb3;4 Bb3;4 Bb3;4 Bb3;6 Ab3;2 G3;4" +
			/*Bar 9 :*/ " Ab3;4 Ab3;4 Ab3;4 Ab3;6 G3;2 Eb3;4 G3;4 Ab3;2 G3;2 Eb3;2 F3;2 G3;6 Ab3;2 Bb3;4" +
			/*Bar 13:*/ " C4;2 Bb3;1 Ab3;1 G3;4 Eb3;4 F3;12";
			#ifndef need_oct_3
			#define need_oct_3
			#endif
			#ifndef need_oct_4
			#define need_oct_4
			#endif
		// Only include required octaves
		#ifdef need_oct_0
		notes["C0"]=  16;
		notes["Db0"]= 17;
		notes["D0"]=  18;
		notes["Eb0"]= 19;
		notes["E0"]=  21;
		notes["F0"]=  22;
		notes["Gb0"]= 23;
		notes["G0"]=  25;
		notes["Ab0"]= 26;
		notes["A0"]=  28;
		notes["Bb0"]= 29;
		notes["B0"]=  31;
		#endif
		#ifdef need_oct_1
		notes["C1"]=  33;
		notes["Db1"]= 35;
		notes["D1"]=  38;
		notes["Eb1"]= 39;
		notes["E1"]=  41;
		notes["F1"]=  44;
		notes["Gb1"]= 46;
		notes["G1"]=  49;
		notes["Ab1"]= 52;
		notes["A1"]=  55;
		notes["Bb1"]= 58;
		notes["B1"]=  62;
		#endif
		#ifdef need_oct_2
		notes["C2"]=  65;
		notes["Db2"]= 69;
		notes["D2"]=  73;
		notes["Eb2"]= 78;
		notes["E2"]=  82;
		notes["F2"]=  87;
		notes["Gb2"]= 93;
		notes["G2"]=  98;
		notes["Ab2"]= 104;
		notes["A2"]=  110;
		notes["Bb2"]= 117;
		notes["B2"]=  123;
		#endif
		#ifdef need_oct_3
		notes["C3"]=  131;
		notes["Db3"]= 139;
		notes["D3"]=  147;
		notes["Eb3"]= 156;
		notes["E3"]=  165;
		notes["F3"]=  175;
		notes["Gb3"]= 185;
		notes["G3"]=  196;
		notes["Ab3"]= 208;
		notes["A3"]=  220;
		notes["Bb3"]= 233;
		notes["B3"]=  247;
		#endif
		#ifdef need_oct_4
		notes["C4"]=  262;
		notes["Db4"]= 277;
		notes["D4"]=  294;
		notes["Eb4"]= 311;
		notes["E4"]=  330;
		notes["F4"]=  349;
		notes["Gb4"]= 370;
		notes["G4"]=  392;
		notes["Ab4"]= 415;
		notes["A4"]=  440;
		notes["Bb4"]= 466;
		notes["B4"]=  494;
		#endif
		#ifdef need_oct_5
		notes["C5"]=  523;
		notes["Db5"]= 554;
		notes["D5"]=  587;
		notes["Eb5"]= 622;
		notes["E5"]=  660;
		notes["F5"]=  698;
		notes["Gb5"]= 740;
		notes["G5"]=  784;
		notes["Ab5"]= 831;
		notes["A5"]=  880;
		notes["Bb5"]= 932;
		notes["B5"]=  988;
		#endif
		#ifdef need_oct_6
		notes["C6"]=  1047;
		notes["Db6"]= 1109;
		notes["D6"]=  1175;
		notes["Eb6"]= 1245;
		notes["E6"]=  1319;
		notes["F6"]=  1397;
		notes["Gb6"]= 1480;
		notes["G6"]=  1568;
		notes["Ab6"]= 1661;
		notes["A6"]=  1760;
		notes["Bb6"]= 1865;
		notes["B6"]=  1976;
		#endif
		#ifdef need_oct_7
		notes["C7"]=  2093;
		notes["Db7"]= 2217;
		notes["D7"]=  2349;
		notes["Eb7"]= 2489;
		notes["E7"]=  2637;
		notes["F7"]=  2794;
		notes["Gb7"]= 2960;
		notes["G7"]=  3136;
		notes["Ab7"]= 3322;
		notes["A7"]=  3520;
		notes["Bb7"]= 3729;
		notes["B7"]=  3951;
		#endif
		#ifdef need_oct_8
		notes["C8"]=  4186;
		notes["Db8"]= 4435;
		notes["D8"]=  4699;
		notes["Eb8"]= 4978;
		#endif
	}
	void tune_worker()
	{
		if ((_tuneindex+1) < tunes[_tunename].length())
		{
			ushort tmpindex=tunes[_tunename].indexOf(' ', _tuneindex);
			_tunetmpstr=tunes[_tunename].substring(_tuneindex, tmpindex);
			ushort semi=_tunetmpstr.indexOf(";");
			
			Serial.println(_tunetmpstr.substring(0, semi) + " for " + _tunetmpstr.substring(semi +1));
			if (_tunetmpstr.substring(0, semi) == "RT")
			{
				noTone(_tunepin);
			}
			else
			{
				tone(_tunepin, notes[_tunetmpstr.substring(0, semi)]);
			}
			_tunetimer.after(_tune_delay_ms * _tunetmpstr.substring(semi +1).toInt(),tune_worker);
			_tuneindex=tmpindex+ _tunetmpstr.length() -1;
		}
		else
		{
			noTone(_tunepin);
			_tuneindex=0;
		}
	}
	void playtune(String tune_name, int pin)
	{
		_tunename=tune_name;
		_tunepin=pin;
		_tuneindex=tunes[_tunename].indexOf(' ') + 1;
		_tune_delay_ms=tunes[_tunename].substring(0, _tuneindex -1).toInt();
		tune_worker();
	}

	void setup()
	{
		tune_init();
		pins.setio(31,false)->setio(35,false)->setio(39,false)->setio(43,false);
		Serial.begin(9600);
		Serial.print((char)27);
		Serial.print("[2J");
		Serial.println("Startup");
		playtune("Dad's Army",4);
		for (int x=0; x < (30*100); x++)
		{
			_tunetimer.update();
			delay(10);
		}
		playtune("God Save the Queen",4);
		for (int x=0; x < (30*100); x++)
		{
			_tunetimer.update();
			delay(10);
		}
		Serial.println("End of loop");
		noTone(4);
	}

	char nextval[10];
	int x, wait=200;
	void loop()
	{
		x=Serial.readBytesUntil('\n',nextval,10);
		if (x >= 8)
		{
			pins.set(43,(nextval[0] == 49));
			pins.set(39,(nextval[1] == 49));
			pins.set(35,(nextval[2] == 49));
			pins.set(31,(nextval[3] == 49));
			Serial.println("Written data part 1: "+(String)(nextval[0]-48)+(String)(nextval[1]-48)+(String)(nextval[2]-48)+(String)(nextval[3]-48));
			delay(wait);
			pins.set(43,false);
			pins.set(39,false);
			pins.set(35,false);
			pins.set(31,false);

			delay(wait);
			pins.set(43,(nextval[4] == 49));
			pins.set(39,(nextval[5] == 49));
			pins.set(35,(nextval[6] == 49));
			pins.set(31,(nextval[7] == 49));
			Serial.println("Written data part 2: "+(String)(nextval[4]-48)+(String)(nextval[5]-48)+(String)(nextval[6]-48)+(String)(nextval[7]-48));
			delay(wait);
			pins.set(43,false);
			pins.set(39,false);
			pins.set(35,false);
			pins.set(31,false);
		}
	}
#endif