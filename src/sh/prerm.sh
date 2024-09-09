#! /bin/sh
#
# prerm.sh
# Copyright (C) 2024 dongbin <dongbin0625@163.com>
#
# Distributed under terms of the MIT license.
#

progress_name="esvcpm"

if pgrep -f "$progress_name" > /dev/null
then
	echo "程序正在运行"
else
	echo "程序尚未运行"
fi
