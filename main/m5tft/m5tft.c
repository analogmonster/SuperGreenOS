/* This program is free software: you can redistribute it and/or modify
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
#include "m5tft.h"

#include "spi_master_lobo.h"
#include "tftspi.h"
#include "tft.h"
#include "wire.h"
#include "AXP192.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../core/kv/kv.h"
#include "../core/log/log.h"

uint8_t axp192_init_list[28] = {
	'A', 'X', 'P',
	0x06, 0, 11,
	0x10, 0xff, //1
	0x28, 0xCC, //2
	0x82, 0xff, //3
	0x33, 0xC1, //4
	0xB8, 0x80, //5
	0x12, 0x4D, //6
	0x36, 0x0C, //7
	0x90, 0x02, //8
	0x30, 0xec, //9
	0x39, 0xFC, //10
	0x35, 0xA2, //11
};

static void m5tft_task(void *param);

void init_m5tft() {
  ESP_LOGI(SGO_LOG_EVENT, "@M5TFT Initializing m5tft module");

  esp_err_t e;

    ESP_LOGI(SGO_LOG_NOSEND, "Setting up I2C");
    e = InitI2CWire(&wire0);
    if (e == ESP_OK) {
        AxpInitFromList(&wire0, axp192_init_list);
    }
    else {
        ESP_LOGE(SGO_LOG_NOSEND, "Error setting up I2C: %s", esp_err_to_name(e));
				return;
    }

  max_rdclock = DEFAULT_SPI_CLOCK;

  TFT_PinsInit();

  spi_lobo_bus_config_t buscfg = {};
  buscfg.mosi_io_num = PIN_NUM_MOSI; // set SPI MOSI pin
  buscfg.sclk_io_num = PIN_NUM_CLK;  // set SPI CLK pin
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;

  spi_lobo_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = DEFAULT_SPI_CLOCK;		 // Initial clock out at 8 MHz
  devcfg.mode = 0;						 // SPI mode 0
  devcfg.spics_io_num = PIN_NUM_CS;	// external CS pin
  devcfg.flags = LB_SPI_DEVICE_HALFDUPLEX; // ALWAYS SET  to HALF DUPLEX MODE!! for display spi

  e = spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &disp_spi);
  if (e != ESP_OK)
  {
    ESP_LOGE(SGO_LOG_NOSEND, "Error adding display to SPI bus: %s", esp_err_to_name(e));
    return;
  }

  TFT_display_init();

  font_rotate = 0;
  text_wrap = 0;
  font_transparent = 0;
  font_forceFixed = 0;
  gray_scale = 0;

  //TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
  TFT_setRotation(LANDSCAPE);
  TFT_setFont(DEFAULT_FONT, NULL);
  //TFT_resetclipwin();

  xTaskCreatePinnedToCore(m5tft_task, "M5TFT", 4096, NULL, 10, NULL, 1);
}

static void m5tft_task(void *param) {
  vTaskDelay(500 / portTICK_PERIOD_MS);
  while (true) {
    char strbuff[100];

    int temp = get_box_temp(0);
    int humi = get_box_humi(0);
    int co2 = get_box_co2(0);

    TFT_fillScreen(TFT_BLACK);
    TFT_print("SuperGreenTips", 5, 10);
    sprintf((char *)strbuff, "Temp: %d\nHumi: %d\nco2: %d", temp, humi, co2);
    TFT_print((char *)strbuff, CENTER, 30);

    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
  }
}

