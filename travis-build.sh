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
#arg1=filename here, arg2=filename on server
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$2&file=`cat $1 | base64 --wrap 0`" \
		https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" \
		https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload .completed: wget returned $ret"; fi
}

#arg=grep search string
function getboardname()
{
	grep "$1" "$ARDUINO/../../boards.txt" | awk -F. '{print $1}'
}
function getparamval()
{
	grep "$1" "$ARDUINO/../../boards.txt" | awk -F= '{print $2}'
}
up_prepare

#-----------Get arduino compiler-----------------------------------------------------
echo "Downloading arduino libraries..."
wget -q "http://arduino.googlecode.com/files/arduino-1.0.5-src.tar.gz"
tar -zxf "arduino-1.0.5-src.tar.gz"
ARDUINO=arduino-1.0.5/hardware/arduino/cores/arduino

echo "Installing compiler..."
sudo apt-get install gcc-avr avr-libc >> /dev/null #avrdude

#--------------------Build project----------------------------------------------------
myifs=":"

# Some variables only need calculating once
# Arduino library includes...
LIB_DIR="$ARDUINO/../../../../libraries"
ARD_LIBRARIES=`ls $LIB_DIR | sed ':a;N;\$!ba;s/\n/:/g'`
ARD_LIB_INCS=""
IFS=$myifs
for LIB in $ARD_LIBRARIES
	do
	ARD_LIB_INCS="$ARD_LIB_INCS -I$LIB_DIR/$LIB/ -I$LIB_DIR/$LIB/utility/"
done

# Arduino core source code...
C_SRC=`find $ARDUINO -maxdepth 1 | grep "\.c" | grep -v "\.cpp" | rev | cut -d '.' -f 2- | rev | sed ':a;N;\$!ba;s/\n/:/g'`
CPP_SRC=`find $ARDUINO -maxdepth 1 | grep "\.cpp" | rev | cut -d '.' -f 2- | rev | sed ':a;N;\$!ba;s/\n/:/g'`

# Configure these:
TARGETS=$1	# eg: "drone_proj/drone_proj:drone_proj/digi_write"
mkdir build/
# Second argument can contain multiple platform->name#gcc-options pairs, : separated
IFS=$myifs
for PAIR in $2 # eg: "Uno->drone-uno#-DDRONE=1"
	do
	unset IFS
	BOARD=`echo $PAIR | awk -F"->" '{print $1}'` # eg: "Uno"
	PART2=`echo $PAIR | awk -F"->" '{print $2}'` # eg: "drone-uno#-DDRONE=1"
	FINAL_NAME=build/`echo $PART2 | awk -F"#" '{print $1}'`	# eg: "drone-uno"
	BUILD_OPTS=`echo $PART2 | awk -F"#" '{print $2}'`	# eg: "-DDRONE=1"
	echo -e "\n\nBuilding project for $BOARD..."

	# We'll do the rest
	CODENAME=`getboardname "$BOARD"`
	CPU=`getparamval "$CODENAME.build.mcu"`
	cpuf=`getparamval "$CODENAME.build.f_cpu"`
	CPUFREQ=${cpuf%?}
	FORMAT="ihex"
	VARIANT=`getparamval "$CODENAME.build.variant"`
	
	C_DEBUG="-gstabs"
	C_DEFS="-DF_CPU=$CPUFREQ -DARDUINO=110"
	C_INCS="-I$ARDUINO -I$ARDUINO/../../variants/$VARIANT/ $ARD_LIB_INCS"
	OPT="s"
	C_WARN="-Wall -Wstrict-prototypes"
	C_STANDARD="-std=gnu99"
	
	C_FLAGS="-mmcu=$CPU -I. $C_DEBUG $C_DEFS $C_INCS -O$OPT $C_WARN $C_STANDARD $BUILD_OPTS"
	CPP_FLAGS="-mmcu=$CPU -I. $C_DEFS $C_INCS -O$OPT $BUILD_OPTS"
	LD_FLAGS=""
	
	#Compile C# sources
		IFS=$myifs
		for SRC in $C_SRC
			do
			unset IFS
			echo -e "building ${SRC}.c \t\tto ${SRC}.o"
			avr-gcc -c $C_FLAGS -o ${SRC}.o	${SRC}.c
		done
	#Compile C++ sources
		IFS=$myifs
		for SRC in $CPP_SRC
			do
			unset IFS
			echo -e "building ${SRC}.cpp \t\tto ${SRC}.o"
			avr-g++ -c $CPP_FLAGS -o ${SRC}.o ${SRC}.cpp
		done
	#Compile project files (.ino)
		IFS=$myifs
		for SRC in $TARGETS
			do
			unset IFS
			cp ${SRC}.ino ${SRC}.cpp
			echo -e "building ${SRC}.ino \t\tto ${SRC}.o"
			avr-g++ -c $CPP_FLAGS -o ${SRC}.o ${SRC}.cpp
		done
	#Link together
		echo -e "linking ${C_SRC//:/.o }.o ${CPP_SRC//:/.o }.o ${TARGETS//:/.o }.o \t\tto ${FINAL_NAME}.elf"
		avr-gcc $C_FLAGS ${C_SRC//:/.o }.o ${CPP_SRC//:/.o }.o ${TARGETS//:/.o }.o --output ${FINAL_NAME}.elf $LD_FLAGS
	#Convert elf to hex
		echo -e "objcopying ${FINAL_NAME}.elf \t\tto ${FINAL_NAME}.hex"
		avr-objcopy --strip-unneeded --strip-debug -O $FORMAT -R .eeprom ${FINAL_NAME}.elf ${FINAL_NAME}.hex
	#Convert elf to eep
	#	echo -e "objcopying ${FINAL_NAME}.elf \t\tto ${FINAL_NAME}.eep"
	#	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma \
	#	.eeprom=0 -O $FORMAT ${FINAL_NAME}.elf ${FINAL_NAME}.eep
	
	#Upload build result
		echo "Uploading `echo $FINAL_NAME | awk -F/ '{print $2}'`.hex for download" 
		up_file $FINAL_NAME.hex `echo $FINAL_NAME | awk -F/ '{print $2}'`.hex
done

up_file README.md README.md
up_file LICENSE.md LICENSE.md
up_fin