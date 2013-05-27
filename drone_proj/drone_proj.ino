/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include "digi_write.h"

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

	void setup()
	{
		lcd.addbuzzer(BUZZER);
		lcd.buzz(1000,0,1);
		batt.setup(MAX17043_ALERT_PERCENT);
		lcd.begin(16,2);
		lcd.write_row("DS CCF Drone",false) -> write_row("Initializing...",true);
		// Perform initial setup here

		bool gps_needed=joystick_t_f(&lcd,&pins,"Enable GPS?"," No        Yes "); // Maybe add in function processing?
		rf.activate(rf_is_up,&lcd,gps_needed); // Get it to set up the system


		lcd.write_row("Primed",false) -> write_row("Turn on Drone",true);
		delay(2000);
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
		//batt.update(); lcd.update(); delay(1000);
		char num[10];
		dtostrf((float)analogRead(0) * 0.0049,0,2,num);
		String mil(millis(),10);
		lcd.write_row((String)num + "V " + mil + "s",false); delay(10);

		batt._timer.update();
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
	void setup()
	{

	}

	void loop()
	{

	}
#endif