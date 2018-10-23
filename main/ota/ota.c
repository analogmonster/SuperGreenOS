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

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>

#include "ota.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_ota_ops.h"

#include "../log/log.h"
#include "../wifi/wifi.h"
#include "../kv/kv.h"
#include "../conf/ota_db.h"
#include "../conf/version.h"

#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;

/*read buffer by byte still delim ,return read bytes counts*/
static int read_until(char *buffer, char delim, int len)
{
  //  /*TODO: delim check,buffer check,further: do an buffer length limited*/
  int i = 0;
  while (buffer[i] != delim && i < len) {
    ++i;
  }
  return i + 1;
}

/* resolve a packet from http socket
 * return true if packet including \r\n\r\n that means http packet header finished,start to receive packet body
 * otherwise return false
 * */
static bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle)
{
  /* i means current position */
  int i = 0, i_read_len = 0;
  while (text[i] != 0 && i < total_len) {
    i_read_len = read_until(&text[i], '\n', total_len);
    // if we resolve \r\n line,we think packet header is finished
    if (i_read_len == 2) {
      int i_write_len = total_len - (i + 2);
      memset(ota_write_data, 0, BUFFSIZE);
      /*copy first http packet body to write buffer*/
      memcpy(ota_write_data, &(text[i + 2]), i_write_len);

      esp_err_t err = esp_ota_write( update_handle, (const void *)ota_write_data, i_write_len);
      if (err != ESP_OK) {
        ESP_LOGE(SGO_LOG_EVENT, "@OTA Error: esp_ota_write failed (%s)!", esp_err_to_name(err));
        return false;
      } else {
        ESP_LOGI(SGO_LOG_EVENT, "@OTA esp_ota_write header OK");
        binary_file_length += i_write_len;
      }
      return true;
    }
    i += i_read_len;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  return false;
}

static bool connect_to_http_server()
{
  ESP_LOGI(SGO_LOG_EVENT, "@OTA Server IP: %s Server Port:%s", OTA_SERVER_IP, OTA_SERVER_PORT);

  int  http_connect_flag = -1;
  struct sockaddr_in sock_info;

  socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Create socket failed!");
    return false;
  }

  // set connect info
  memset(&sock_info, 0, sizeof(struct sockaddr_in));
  sock_info.sin_family = AF_INET;
  sock_info.sin_addr.s_addr = inet_addr(OTA_SERVER_IP);
  sock_info.sin_port = htons(atoi(OTA_SERVER_PORT));

  // connect to http server
  http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
  if (http_connect_flag == -1) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Connect to server failed! errno=%d", errno);
    close(socket_id);
    return false;
  } else {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Connected to server");
    return true;
  }
 return false;
}

static bool check_new_version() {
  /*connect to http server*/
  if (connect_to_http_server()) {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Connected to http server");
  } else {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Connect to http server failed!");
    close(socket_id);
    return false;
  }

  /*send GET request to http server*/
  const char *GET_FORMAT =
    "GET %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n\r\n";

  char *http_request = NULL;
  int get_len = asprintf(&http_request, GET_FORMAT, OTA_VERSION_FILENAME, OTA_SERVER_HOSTNAME, OTA_SERVER_PORT);
  if (get_len < 0) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Failed to allocate memory for GET request buffer");
    close(socket_id);
    return false;
  }
  int res = send(socket_id, http_request, get_len, 0);
  free(http_request);

  if (res < 0) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Send GET request to server failed");
    close(socket_id);
    return false;
  } else {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Send GET request to server succeeded");
  }

  int i = 0;
  char buf;
  while (i < 4) {
    if (recv(socket_id, &buf, 1, 0) != 1) return false;
    if (buf == '\n' || buf == '\r') ++i;
    else i = 0;
  }

  ESP_LOGI(SGO_LOG_EVENT, "@OTA Skipped http header");

  char timestamp[15] = {0};
  while(recv(socket_id, &(timestamp[strlen(timestamp)]), sizeof(timestamp) - strlen(timestamp) - 1, 0) > 0);
  ESP_LOGI(SGO_LOG_EVENT, "@OTA OTA TIMESTAMP: %d (build: %lu)", atoi(timestamp), OTA_TIMESTAMP);
  return OTA_TIMESTAMP < atoi(timestamp);
}

static void try_ota()
{
  esp_err_t err;
  /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
  esp_ota_handle_t update_handle = 0 ;
  const esp_partition_t *update_partition = NULL;

  ESP_LOGI(SGO_LOG_EVENT, "@OTA Starting OTA example...");

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (configured != running) {
    ESP_LOGW(SGO_LOG_EVENT, "@OTA Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
        configured->address, running->address);
    ESP_LOGW(SGO_LOG_EVENT, "@OTA (This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
  }
  ESP_LOGI(SGO_LOG_EVENT, "@OTA Running partition type %d subtype %d (offset 0x%08x)",
      running->type, running->subtype, running->address);

  /*connect to http server*/
  if (connect_to_http_server()) {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Connected to http server");
  } else {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Connect to http server failed!");
    close(socket_id);
    return;
  }

  /*send GET request to http server*/
  const char *GET_FORMAT =
    "GET %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n\r\n";

  char *http_request = NULL;
  int get_len = asprintf(&http_request, GET_FORMAT, OTA_FILENAME, OTA_SERVER_HOSTNAME, OTA_SERVER_PORT);
  if (get_len < 0) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Failed to allocate memory for GET request buffer");
    close(socket_id);
    return;
  }
  int res = send(socket_id, http_request, get_len, 0);
  free(http_request);

  if (res < 0) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA Send GET request to server failed");
    close(socket_id);
    return;
  } else {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Send GET request to server succeeded");
  }

  update_partition = esp_ota_get_next_update_partition(NULL);
  ESP_LOGI(SGO_LOG_EVENT, "@OTA Writing to partition subtype %d at offset 0x%x",
      update_partition->subtype, update_partition->address);
  assert(update_partition != NULL);

  err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
  if (err != ESP_OK) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA esp_ota_begin failed (%s)", esp_err_to_name(err));
    close(socket_id);
    return;
  }
  ESP_LOGI(SGO_LOG_EVENT, "@OTA esp_ota_begin succeeded");

  bool resp_body_start = false, socket_flag = true, http_200_flag = false;
  /*deal with all receive packet*/
  while (socket_flag) {
    memset(text, 0, TEXT_BUFFSIZE);
    memset(ota_write_data, 0, BUFFSIZE);
    int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
    if (buff_len < 0) { /*receive error*/
      ESP_LOGE(SGO_LOG_EVENT, "@OTA Error: receive data error! errno=%d", errno);
      close(socket_id);
      return;
    } else if (buff_len > 0 && !resp_body_start) {  /*deal with response header*/
      // only start ota when server response 200 state code
      if (strstr(text, "200") == NULL && !http_200_flag) {
        ESP_LOGE(SGO_LOG_EVENT, "@OTA ota url is invalid or bin is not exist");
        close(socket_id);
        return;
      }
      http_200_flag = true;
      memcpy(ota_write_data, text, buff_len);
      resp_body_start = read_past_http_header(text, buff_len, update_handle);
    } else if (buff_len > 0 && resp_body_start) { /*deal with response body*/
      memcpy(ota_write_data, text, buff_len);
      err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
      if (err != ESP_OK) {
        ESP_LOGE(SGO_LOG_EVENT, "@OTA Error: esp_ota_write failed (%s)!", esp_err_to_name(err));
        close(socket_id);
        return;
      }
      binary_file_length += buff_len;
      ESP_LOGI(SGO_LOG_EVENT, "@OTA Have written image length %d", binary_file_length);
    } else if (buff_len == 0) {  /*packet over*/
      socket_flag = false;
      ESP_LOGI(SGO_LOG_EVENT, "@OTA Connection closed, all packets received");
      close(socket_id);
    } else {
      ESP_LOGE(SGO_LOG_EVENT, "@OTA Unexpected recv result");
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  ESP_LOGI(SGO_LOG_EVENT, "@OTA Total Write binary data length : %d", binary_file_length);

  if (esp_ota_end(update_handle) != ESP_OK) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA esp_ota_end failed!");
    close(socket_id);
    return;
  }
  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) {
    ESP_LOGE(SGO_LOG_EVENT, "@OTA esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
    close(socket_id);
    return;
  }
  ESP_LOGI(SGO_LOG_EVENT, "@OTA Prepare to restart system!");
  esp_restart();
  return ;
}

static void ota_task(void *pvParameter) {
  while (true) {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA OTA waiting for wifi");
    wait_connected();
    ESP_LOGI(SGO_LOG_EVENT, "@OTA Checking firmware update available");
    if (check_new_version()) {
      ESP_LOGI(SGO_LOG_EVENT, "@OTA Start OTA procedure");
      try_ota();
    } else {
      ESP_LOGI(SGO_LOG_EVENT, "@OTA Firmware is up-to-date");
    }
    vTaskDelay((1 * 3600 * 1000) / portTICK_PERIOD_MS);
  }
}

void init_ota() {
  if (OTA_TIMESTAMP == 0) {
    ESP_LOGI(SGO_LOG_EVENT, "@OTA OTA NOT INITIALIZING timestamp: %lu", OTA_TIMESTAMP);
    return;
  }
  ESP_LOGI(SGO_LOG_EVENT, "@OTA OTA initialization timestamp: %lu", OTA_TIMESTAMP);

  xTaskCreate(&ota_task, "OTA", 8192, NULL, 5, NULL);
}
