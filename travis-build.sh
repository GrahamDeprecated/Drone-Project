#!/bin/bash

#arg=filename
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1`" http://pml369-builds.suroot.com/UPLOAD.php
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date`" http://pml369-builds.suroot.com/UPLOAD.php
}

up_file drone_proj.v11.suo
up_fin