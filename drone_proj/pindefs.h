/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRONE
#define DRONE 0 /*0 for drone code, 1 for controller code*/
#endif

#if (DRONE == 0)

	#define RF_IN_BIT_1 26
	#define RF_IN_BIT_2 27
	#define RF_IN_BIT_3 28
	#define RF_IN_BIT_4 29
	#define RF_IN_INTER 1 /*pin 3*/
	#define RF_OUT_BIT_1 31
	#define RF_OUT_BIT_2 32
	#define RF_OUT_BIT_3 33
	#define RF_OUT_BIT_4 34

	#define MOTOR_A_TX 35
	#define MOTOR_A_RX 10
	#define MOTOR_B_TX 36
	#define MOTOR_B_RX 11

	#define GPS_ENABLE 25
	#define GPS_TX 16
	#define GPS_RX 17

	#define USB_HOST_RX 15
	#define USB_HOST_TX 14

	#define ACCEL_ALEEP 22
	#define ACCEL_G_SELECT 23
	#define ACCEL_SELF_TEST 24
	#define ACCEL_0G_INTER 2
	#define ACCEL_X 2 /*analog*/
	#define ACCEL_Y 3 /*analog*/
	#define ACCEL_Z 4 /*analog*/

	#define RANGE_SENSOR 1 /*analog*/

	#define BATT_LEVEL 0 /*analog*/

	#define RAND_SEED_PIN 15 /*analog*/

	#define ARD_INTERRUPTS "2,3,18,19,20,21" //end in ","

	#define SHIFT_SERIAL 50
	#define SHIFT_CLOCK  51
	#define SHIFT_LATCH  52

#else
	#define ARD_INTERRUPTS "2,3," //end in ","

	#define SHIFT_SERIAL 10
	#define SHIFT_CLOCK  11
	#define SHIFT_LATCH  12

	#define RF_OUT_BIT_1 0
	#define RF_OUT_BIT_2 0
	#define RF_OUT_BIT_3 0
	#define RF_OUT_BIT_4 0
	#define RF_IN_INTER 0
	#define RF_IN_BIT_1 0
	#define RF_IN_BIT_2 0
	#define RF_IN_BIT_3 0
	#define RF_IN_BIT_4 0

	#define LCD_D4 7
	#define LCD_D5 6
	#define LCD_D6 5
	#define LCD_D7 4
	#define LCD_ENABLE 8
	#define LCD_REG_SELECT 9

	#define BUTTON_LAUNCH 0

	#define MAX17043_ADDRESS 0x36  // R/W =~ 0x6D/0x6C
	#define MAX17043_ALERT_PERCENT 20 // max=32
	#define BATT_ALERT 2

	// Analog
	#define JOYSTICK_1_H 0
	// Digital
	#define JOYSTICK_1_CLICK 12

	#define BUZZER 3
#endif