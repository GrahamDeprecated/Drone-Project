#!/bin/bash

#arg=filename
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1 | base64 --wrap 0`" http://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" http://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}

#-----------Get arduino compiler-----------------------------------------------------
wget "http://arduino.googlecode.com/files/arduino-1.0.5-src.tar.gz"
tar -zxf "arduino-1.0.5-src.tar.gz"
cp -rf "arduino-1.0.5/hardware/arduino/cores/arduino/" "arduino_include/"
rm -rf "arduino-1.0.5-src.tar.gz" "arduino-1.0.5/"
#Includes are now in "arduino_include"

sudo apt-get install gcc-avr avr-libc avrdude
wget -q -O Makefile http://pml369-builds.suroot.com/travis-makefile-arduino 

#--------------------Build project----------------------------------------------------
wget https://pml369-builds.suroot.com/readme.txt
wget --version
apt-get list wget

#-------------Upload build results-------------------------------------------------------
up_file drone_proj.vcxproj
up_file README.md
up_file LICENSE.md
up_fin