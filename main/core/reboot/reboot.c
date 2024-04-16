/*
 * Copyright (C) 2019  SuperGreenLab <towelie@supergreenlab.com>
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

#include "reboot.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "../log/log.h"
#include "../kv/kv.h"
#include "../ota/ota.h"

#define MAX_SHORT_REBOOTS 5
#define N_SHORT_REBOOTS "NSHRBTS"

static QueueHandle_t cmd;

static void autoreboot_task();
static void reboot_task();

static void reset_nvs() {
  ESP_ERROR_CHECK(nvs_flash_erase());
  esp_restart();
}

void reset_on_next_reboot() {
  seti8(N_SHORT_REBOOTS, MAX_SHORT_REBOOTS);
}

void init_reboot() {
  if (hasi32(N_SHORT_REBOOTS)) { // detect old version
    reset_nvs();
  }
  defaulti8(N_SHORT_REBOOTS, 0);
  int n = geti8(N_SHORT_REBOOTS);
  if (n >= MAX_SHORT_REBOOTS) {
    reset_nvs();
  }
  ESP_LOGI(SGO_LOG_EVENT, "@REBOOT N_SHORT_REBOOTS=%d", n);
  seti8(N_SHORT_REBOOTS, ++n);

  cmd = xQueueCreate(10, sizeof(unsigned char));
  if (cmd == NULL) {
    ESP_LOGE(SGO_LOG_NOSEND, "@REBOOT Unable to create reboot queue");
  }

  BaseType_t ret = xTaskCreatePinnedToCore(autoreboot_task, "AUTOREBOOT", 2048, NULL, 10, NULL, 1);
  if (ret != pdPASS) {
    ESP_LOGE(SGO_LOG_NOSEND, "@REBOOT Failed to create task");
  }

  BaseType_t ret2 = xTaskCreatePinnedToCore(reboot_task, "REBOOT", 2048, NULL, 10, NULL, 1);
  if (ret2 != pdPASS) {
    ESP_LOGE(SGO_LOG_NOSEND, "@REBOOT Failed to create task");
  }
}

static void autoreboot_task(void *args) {
  // reset n_short_reboots to zero
  vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
  ESP_LOGI(SGO_LOG_EVENT, "@REBOOT N_SHORT_REBOOTS=0");
  seti8(N_SHORT_REBOOTS, 0);

  vTaskDelete(NULL);
}

static void reboot_task(void *args) {
  unsigned char c;
  while (true) {
    if (xQueueReceive(cmd, &c, 10000 / portTICK_PERIOD_MS)) {
      // little wait to allow http response to go through
      vTaskDelay(500 / portTICK_PERIOD_MS);
      esp_restart();
    }
  }
}

void reboot_esp() {
  unsigned char c = 1;
  xQueueSend(cmd, &c, 0);
}

/*
 * http callback
 */

int on_set_reboot(int value) {
  reboot_esp();
  return value;
}
