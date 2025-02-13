/*
 * Copyright (C) 2018  SuperGreenLab <towelie@supergreenlab.com>
 * Author: Constantin Clauzel <constantin.clauzel@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BLOWER_H_
#define BLOWER_H_

#define DEFAULT_BLOWER_FREQUENCY 150

void init_blower();
void refresh_blower();

/* KV Callbacks */

int on_set_box_blower_duty(int boxId, int value);
int on_set_box_blower_frequency(int boxId, int value);
int on_set_box_blower_day(int boxId, int value);
int on_set_box_blower_night(int boxId, int value);

#endif
