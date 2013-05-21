#!/bin/bash

#arg=filename
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1 | base64 --wrap 0`" http://pml369-builds.suroot.com/UPLOAD.php >> /dev/null
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" http://pml369-builds.suroot.com/UPLOAD.php >> /dev/null
}

up_file drone_proj.vcxproj
up_fin