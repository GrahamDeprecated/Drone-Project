/*
 drone_proj - main code for controlling an aerial drone
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

#include <stdint.h>
#include <..\..\..\..\..\libraries\Wire\Wire.h>
#include "pindefs.h"
#include "digi_write.h"

Shifter shifts2(SHIFT_SERIAL, SHIFT_LATCH, SHIFT_CLOCK, 1 /*Number of shift regiusters in chain*/);
digi_pins pins(&shifts2,"0,1");
digi_rf rf(&pins, RF_IN_BIT_1, RF_IN_BIT_2, RF_IN_BIT_3, RF_IN_BIT_4, RF_IN_INTER, RF_OUT_BIT_1, RF_OUT_BIT_2, RF_OUT_BIT_3, RF_OUT_BIT_4);
digi_serial com(&pins, RF_OUT_BIT_1, RF_IN_BIT_1, RF_IN_INTER);

#if (DRONE == 1)
	digi_batt batt(&pins,BATT_ALERT, MAX17043_ALERT_PERCENT);
	digi_lcd lcd(&pins,&batt,LCD_REG_SELECT,LCD_ENABLE,LCD_D4,LCD_D5,LCD_D6,LCD_D7);
	void rf_is_up();

	void setup()
	{
		lcd.begin(16,2);
		lcd.write_row("DS CCF Drone",false) -> write_row("Initializing...",true);
		// Perform initial setup here
		rf.activate(rf_is_up,&lcd); // Get it to set up the system

		delay(5000);
		lcd.write_row("Primed",false) -> write_row("Turn on Drone",true);
		delay(2000);
	}
	void rf_is_up()
	{
		lcd.write_row("Connected",false) -> write_row("Press to launch",true);
		attachInterrupt(BUTTON_LAUNCH,launch,RISING);
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
		batt.update(100); delay(1000);
		batt.update(80); delay(1000);
		batt.update(60); delay(1000);
		batt.update(40); delay(1000);
		batt.update(20); delay(1000);
	}
#else
	void setup()
	{

	}

	void loop()
	{

	}
#endif