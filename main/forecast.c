#include "forecast.h"

#include <string.h>

#include "cJSON.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "nvs_flash.h"

#define WEB_SERVER "api.apixu.com"
#define WEB_PORT CONFIG_APIXU_PORT
#define WEB_URL CONFIG_APIXU_URL
#define WEB_FILE_SIZE 65536

#define TAG "fc"

static const uint8_t* skip_headers(const uint8_t *s) {
  int nl = 0;
  while (*s) {
    if (*s == '\n') {
      if (nl) {
        ++s;
        break;
      } else {
        nl = 1;
      }
    } else if (*s != '\r') {
      nl = 0;
    }
    ++s;
  }
  return s;
}

static esp_err_t parse_forecast(const uint8_t *s, forecast_t *forecast) {
  cJSON *root = NULL;
  cJSON *cur, *data;

  root = cJSON_Parse((const char *)skip_headers(s));
  if (!cJSON_IsObject(root)) goto err;

  cur = cJSON_GetObjectItem(root, "forecast");
  if (!cJSON_IsObject(cur)) goto err;

  cur = cJSON_GetObjectItem(cur, "forecastday");
  if (!cJSON_IsArray(cur)) goto err;

  cur = cJSON_GetArrayItem(cur, 0);
  if (!cJSON_IsObject(cur)) goto err;

  cur = cJSON_GetObjectItem(cur, "day");
  if (!cJSON_IsObject(cur)) goto err;

  data = cJSON_GetObjectItem(cur, "mintemp_c");
  if (!cJSON_IsNumber(data)) goto err;
  forecast->temp_min = (int)(data->valuedouble+0.5);

  data = cJSON_GetObjectItem(cur, "maxtemp_c");
  if (!cJSON_IsNumber(data)) goto err;
  forecast->temp_max = (int)(data->valuedouble+0.5);

  cur = cJSON_GetObjectItem(cur, "condition");
  if (!cJSON_IsObject(cur)) goto err;

  data = cJSON_GetObjectItem(cur, "code");
  if (!cJSON_IsNumber(data)) goto err;
  forecast->code = data->valueint;

  // TODO:
  forecast->day = 1;

  cJSON_Delete(root);
  return ESP_OK;

err:
  ESP_LOGE(TAG, "Unable to parse JSON");
  if (root != NULL)
    cJSON_Delete(root);
  return ESP_FAIL;
}

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
  "Host: "WEB_SERVER"\r\n"
  "User-Agent: esp-idf/1.0 esp32\r\n"
  "\r\n";

esp_err_t get_forecast(forecast_t *forecast) {
  esp_err_t err;
  const struct addrinfo hints =
    {
     .ai_family = AF_INET,
     .ai_socktype = SOCK_STREAM,
    };
  struct addrinfo *res;
  struct in_addr *addr;
  int s, r, i;
  uint8_t* recv_buf;

  err = getaddrinfo(WEB_SERVER, "80", &hints, &res);
  if(err != 0 || res == NULL) {
    ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
    return ESP_FAIL;
  }

  /* Code to print the resolved IP.
   * Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
  addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
  ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

  s = socket(res->ai_family, res->ai_socktype, 0);
  if(s < 0) {
    ESP_LOGE(TAG, "... Failed to allocate socket.");
    freeaddrinfo(res);
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "... allocated socket");

  if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
    ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
    close(s);
    freeaddrinfo(res);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "... connected");
  freeaddrinfo(res);

  if (write(s, REQUEST, strlen(REQUEST)) < 0) {
    ESP_LOGE(TAG, "... socket send failed");
    close(s);
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "... socket send success");

  struct timeval receiving_timeout;
  receiving_timeout.tv_sec = 5;
  receiving_timeout.tv_usec = 0;
  if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                 sizeof(receiving_timeout)) < 0) {
    ESP_LOGE(TAG, "... failed to set socket receiving timeout");
    close(s);
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "... set socket receiving timeout success");

  /* Allocate buffer for the response */
  recv_buf = malloc(WEB_FILE_SIZE);
  if (recv_buf == NULL) {
    ESP_LOGI(TAG, "Could not allocate file buffer");
    return ESP_ERR_NO_MEM;
  }

  /* Read HTTP response */
  i = 0;
  while (i != WEB_FILE_SIZE-1) {
    r = read(s, recv_buf+i, WEB_FILE_SIZE-i-1);
    if (r <= 0)
      break;
    i += r;
  }
  recv_buf[i] = 0;

  ESP_LOGI(TAG, "... done reading from socket."
           "Last read return=%d errno=%d\r\n", r, errno);
  close(s);

  err = parse_forecast(recv_buf, forecast);
  free(recv_buf);
  return err;
}
