#!/bin/bash

function up_prepare()
{
	sudo wget -q -O /usr/share/ca-certificates/pml369.crt http://pml369-builds.suroot.com/pml369.crt
	ret=$?
	if [ ! $ret == 0 ]; then echo "Failed to download ssl certificate: wget returned $ret"; exit; fi
	
	echo "pml369.crt" | sudo tee -a /etc/ca-certificates.conf >> /dev/null
	sudo update-ca-certificates >> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Failed to update CA database: update-ca-certificates returned $ret"; exit; fi
}
#arg=filename
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1 | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload .completed: wget returned $ret"; fi
}

#arg=grep search string
function getboardname()
{
	grep $1 ../arduino-1.0.5/hardware/arduino/boards.txt | awk -F. '{print $1}'
}
function getparamval()
{
	grep $1 ../arduino-1.0.5/hardware/arduino/boards.txt | awk -F= '{print $2}'
}
up_prepare

#-----------Get arduino compiler-----------------------------------------------------
echo "Downloading arduino libraries..."
wget -q "http://arduino.googlecode.com/files/arduino-1.0.5-src.tar.gz"
tar -zxf "arduino-1.0.5-src.tar.gz"
#cp -rf "arduino-1.0.5/hardware/arduino/cores/arduino/" "arduino_include/"
#cp -rf "arduino-1.0.5/hardware/arduino/variants/" "arduino_variants/"
#cp -rf "arduino-1.0.5/hardware/arduino/boards.txt" "."
#rm -rf "arduino-1.0.5-src.tar.gz" "arduino-1.0.5/"
#Includes are now in "arduino_include"
ARDUINO=../arduino-1.0.5/hardware/arduino/cores/arduino/

echo "Installing compiler..."
sudo apt-get install gcc-avr avr-libc >> /dev/null #avrdude

#--------------------Build project----------------------------------------------------
echo "Building project..."
cd drone_proj
#wget -q -O Makefile https://pml369-builds.suroot.com/travis-makefile-arduino
#make TARGET=drone_proj ARDUINO=$ARDUINO
#Instead of Makefile:
# Configure these:

BOARD="Uno"
TARGETS="drone_proj digi_write "
FINAL_NAME="drone"

# We'll do the rest
CODENAME=`getboardname "$BOARD"`

CPU=`getparamval "$CODENAME.build.mcu"`
cpuf=`getparamval "$CODENAME.build.f_cpu"`
CPUFREQ=${cpuf%?}
FORMAT="ihex"
VARIANT=`getparamval "$CODENAME.build.variant"`

C_DEBUG="-gstabs"
C_DEFS="-DF_CPU=$CPUFREQ -DARDUINO=110"
C_INCS="-I$ARDUINO -I../ -I../arduino-1.0.5/hardware/arduino/variants/$VARIANT/ -I../arduino-1.0.5/libraries/"
OPT="s"
C_WARN="-Wall -Wstrict-prototypes"
C_STANDARD="-std=gnu99"

C_FLAGS="-mmcu=$CPU -I. $C_DEBUG $C_DEFS $C_INCS -O$OPT $C_WARN $C_STANDARD"
CPP_FLAGS="-mmcu=$CPU -I. $C_DEFS $C_INCS -O$OPT"
LD_FLAGS=""

C_SRC="$ARDUINO/wiring_shift $ARDUINO/wiring_pulse $ARDUINO/wiring_digital $ARDUINO/wiring_analog $ARDUINO/wiring $ARDUINO/WInterrupts "
CPP_SRC="$ARDUINO/WString $ARDUINO/WMath $ARDUINO/USBCore $ARDUINO/Tone $ARDUINO/Stream $ARDUINO/Print $ARDUINO/new $ARDUINO/main $ARDUINO/IPAddress $ARDUINO/HID $ARDUINO/HardwareSerial $ARDUINO/CDC "

#Compile C# sources
	for SRC in $C_SRC
	do
		echo -e "building ${SRC}.c \t\tto ${SRC}.o"
		avr-gcc -c $C_FLAGS -o ${SRC}.o	${SRC}.c
	done
#Compile C++ sources
	for SRC in $CPP_SRC
	do
		echo -e "building ${SRC}.cpp \t\tto ${SRC}.o"
		avr-g++ -c $CPP_FLAGS -o ${SRC}.o ${SRC}.cpp
	done
#Compile project files
	for SRC in $TARGETS
	do
		mv ${SRC}.ino ${SRC}.cpp
		echo -e "building ${SRC}.ino \t\tto ${SRC}.o"
		avr-g++ -c $CPP_FLAGS -o ${SRC}.o ${SRC}.cpp
	done
#Link together
	echo -e "linking ${C_SRC// /.o } ${CPP_SRC// /.o } ${FINAL_NAME}.o \t\tto ${FINAL_NAME}.elf"
	avr-gcc $C_FLAGS ${C_SRC// /.o } ${CPP_SRC// /.o } ${TARGETS// /.o } --output ${FINAL_NAME}.elf $LD_FLAGS
#Convert elf to hex
	echo -e "objcopying ${FINAL_NAME}.elf \t\tto ${FINAL_NAME}.hex"
	avr-objcopy -O $FORMAT -R .eeprom ${FINAL_NAME}.elf ${FINAL_NAME}.hex
#Convert elf to eep
	echo -e "objcopying ${TARGET}.elf \t\tto ${TARGET}.eep"
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $FORMAT ${FINAL_NAME}.elf ${FINAL_NAME}.eep


ls -l
cd ..
#-------------Upload build results-------------------------------------------------------
up_file drone_proj.vcxproj
up_file README.md
up_file LICENSE.md
up_fin