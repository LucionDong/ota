#! /bin/sh
#
# preinst.sh
# Copyright (C) 2024 dongbin <dongbin0625@163.com>
#
# Distributed under terms of the MIT license.
#


progress_name="esvcpm"
# progress_name="a.out"
signal_to_send=9

if pgrep -f "$progress_name" > /dev/null
then
	echo "脚本正在运行,正在终止..."
	pids=$(ps -ef | grep "$progress_name" | awk '{print $2}')
	for pid in $pids; do
		kill -$signal_to_send $pid
	done
	echo "脚本终止"
else
	echo "脚本未运行"
fi
