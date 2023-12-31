/*
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

#include <stdlib.h>
#include "remote_hx711.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../core/kv/kv.h"
#include "../core/log/log.h"

static void remote_hx711_task(void *param);

void init_remote_hx711() {
  ESP_LOGI(SGO_LOG_EVENT, "@REMOTE_HX711 Initializing remote_hx711 module");

  // TODO: write you setup code here

  xTaskCreatePinnedToCore(remote_hx711_task, "REMOTE_HX711", 4096, NULL, 10, NULL, 1);
}

static void remote_hx711_task(void *param) {
  while (true) {

    // TODO: write your loop code here

    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
  }
}

