/*
 * Copyright (C) 2023  SuperGreenLab <towelie@supergreenlab.com>
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

#include "bitmaps.h"

#include "m5tft.h"
#include "tftspi.h"
#include "bitmaps_definitions.h"
#include "math.h"

#include "../core/log/log.h"

void scaled_draw_bitmap(const bitmap_data *img, int x, int y, float scale, RenderOpt *opts) {
  // Calculate the new scaled dimensions
  int scaled_width = ceilf(img->width * scale);
  int scaled_height = ceilf(img->height * scale);

  // Calculate the starting draw positions to ensure the image is anchored at its center
  int draw_start_x = x + ((img->width - scaled_width) / 2);
  int draw_start_y = y + ((img->height - scaled_height) / 2);

  for (int i = 0; i < scaled_width; i++) {
    for (int j = 0; j < scaled_height; j++) {

      // Calculate the corresponding source pixel based on scaling
      int src_x = (int)(i / scale);
      int src_y = (int)(j / scale);

      // Fetch the palette color from the original bitmap, not the scaled one
      color_t color = img->palette[img->bitmap[src_x + src_y * img->width]];

      // Calculate the positions on the frame where the pixel should be drawn
      int frame_x = draw_start_x + i;
      int frame_y = draw_start_y + j;

      // Check boundaries to avoid overwriting the frame and ignore transparent pixels
      if (frame_x < DEFAULT_TFT_DISPLAY_HEIGHT && frame_y < DEFAULT_TFT_DISPLAY_WIDTH 
          && frame_x >= 0 && frame_y >= 0 
          && !(color.r == 0xff && color.g == 0x00 && color.b == 0xff)) {

        frame[frame_x + frame_y * DEFAULT_TFT_DISPLAY_HEIGHT] = color;
      }
    }
  }
}

void draw_bitmap(const bitmap_data *img, int x, int y, RenderOpt *opts) {
  // Iterate through the image's pixels
  for (int i = 0; i < img->width; i++) {
    for (int j = 0; j < img->height; j++) {

      if (!(x + i < DEFAULT_TFT_DISPLAY_HEIGHT && y + j < DEFAULT_TFT_DISPLAY_WIDTH && x + i >= 0 && y + j >= 0)) {
        continue;
      }

      color_t color = img->palette[img->bitmap[i + j * img->width]];

      // Check for transparent color
      if (color.r == 0xff && color.g == 0x00 && color.b == 0xff) {
        continue;
      }

      // If grayscale (but not white), interpolate with target color
      if (opts != NULL && color.r == color.g && color.g == color.b && color.b != 0xff) {
        float grayValue = 1.0f - ((float)color.r / 255.0f); // invert grayscale value
        color.r = (1 - grayValue) * 0xff + grayValue * (float)opts->targetColor.r;
        color.g = (1 - grayValue) * 0xff + grayValue * (float)opts->targetColor.g;
        color.b = (1 - grayValue) * 0xff + grayValue * (float)opts->targetColor.b;
      }

      if (opts != NULL && opts->transparency < 1) {
        color_t current_pixel = frame[(x + i) + (y + j) * DEFAULT_TFT_DISPLAY_HEIGHT];

        float alpha = opts->transparency;
        color.r = current_pixel.r * (1.0f - alpha) + color.r * alpha;
        color.g = current_pixel.g * (1.0f - alpha) + color.g * alpha;
        color.b = current_pixel.b * (1.0f - alpha) + color.b * alpha;
      }

      frame[(x + i) + (y + j) * DEFAULT_TFT_DISPLAY_HEIGHT] = color;
    }
  }
}
