#!/bin/bash
# Copyright (C) 2019  SuperGreenLab <towelie@supergreenlab.com>
# Author: Constantin Clauzel <constantin.clauzel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CONFIG_PATH="$1"
CONFIG_FILE="$2"

if [ -z "$CONFIG_PATH" ] || [ -z "$CONFIG_FILE" ]; then
  echo "USAGE: $0 config_path config.json"
  exit
fi

pushd $CONFIG_PATH
cue export ./... > "../../../../$CONFIG_FILE"
popd
