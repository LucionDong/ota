#! /bin/sh
#
# clean.sh
# Copyright (C) 2024 dongbin <dongbin0625@163.com>
#
# Distributed under terms of the MIT license.
#


rm -rf $(find . -name "*" ! -name "build.sh" ! -name "clean.sh")
