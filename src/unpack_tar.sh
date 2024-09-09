#! /bin/sh
#
# unpack_tar.sh
# Copyright (C) 2024 dongbin <dongbin0625@163.com>
#
# Distributed under terms of the MIT license.
#

if [ "$1" == "/root/ota/data.tar.gz" ];
then
	tar -zxf $1 -C "/opt"
else
	tar -zxf $1
fi
