#!/bin/bash

function up_prepare()
{
	wget -q -O ~/pml369.crt http://pml369-builds.suroot.com/pml369.crt
	ret=$?
	if [ ! $ret == 0 ]; then echo "Failed to download ssl certificate: wget returned $ret"; exit; fi
}
#arg=filename
function up_file()
{
	wget -q --ca-certificate=~/pml369.crt -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1 | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}
function up_fin()
{
	wget -q --ca-certificate=~/pml369.crt -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload .completed: wget returned $ret"; fi
}

up_prepare

#-----------Get arduino compiler-----------------------------------------------------
echo "Downloading arduino libraries..."
wget -q "http://arduino.googlecode.com/files/arduino-1.0.5-src.tar.gz"
tar -zxf "arduino-1.0.5-src.tar.gz"
cp -rf "arduino-1.0.5/hardware/arduino/cores/arduino/" "arduino_include/"
rm -rf "arduino-1.0.5-src.tar.gz" "arduino-1.0.5/"
#Includes are now in "arduino_include"

echo "Installing compiler..."
sudo apt-get install gcc-avr avr-libc >> /dev/null #avrdude

#--------------------Build project----------------------------------------------------
wget --ca-certificate=~/pml369.crt -O Makefile https://pml369-builds.suroot.com/travis-makefile-arduino
#openssl s_client -CAfile ~/pml369.crt -connect pml369-builds.suroot.com:443
wget --version


sudo cp ~/pml369.crt /usr/share/ca-certificates/
sudo dpkg-reconfigure ca-certificates
sudo update-ca-certificates

wget -O Makefile https://pml369-builds.suroot.com/travis-makefile-arduino

#-------------Upload build results-------------------------------------------------------
up_file drone_proj.vcxproj
up_file README.md
up_file LICENSE.md
up_fin