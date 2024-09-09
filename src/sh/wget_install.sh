#! /bin/sh
#
# wget_install.sh
# Copyright (C) 2024 dongbin <dongbin0625@163.com>
#
# Distributed under terms of the MIT license.
#


echo "$MY_VAR"
file_url="$MY_VAR"
download_location="."

wget -O "$download_location/file.zip" "$file_url"

if [ $? -eq 0 ]; then
  echo "File downloaded successfully to $download_location/file.zip"
else
  echo "Failed to download file"
  exit 1
fi
