#include "camera_server.h"

#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "sdkconfig.h"
#include "camera.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define TAG ""
#else
#include "esp_log.h"
static const char *TAG = "camera_server";
#endif

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static bool gReturnFB = true;

static int8_t detection_enabled = 0;
static int8_t recognition_enabled = 0;
static int8_t is_enrolling = 0;

typedef struct {
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: "
    "%d.%06d\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t mjpg_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

static size_t jpg_encode_stream(void *arg, size_t index, const void *data,
                                size_t len) {
  jpg_chunking_t *j = (jpg_chunking_t *)arg;
  if (!index) {
    j->len = 0;
  }
  if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
    return 0;
  }
  j->len += len;
  return len;
}

static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t *frame = NULL;
  esp_err_t res = ESP_OK;

  if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition",
                       "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char ts[32];
    snprintf(ts, 32, "%ld.%06ld", frame->timestamp.tv_sec,
             frame->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    // size_t fb_len = 0;
    if (frame->format == PIXFORMAT_JPEG) {
      // fb_len = frame->len;
      res = httpd_resp_send(req, (const char *)frame->buf, frame->len);
    } else {
      jpg_chunking_t jchunk = {req, 0};
      res = frame2jpg_cb(frame, 80, jpg_encode_stream, &jchunk) ? ESP_OK
                                                                : ESP_FAIL;
      httpd_resp_send_chunk(req, NULL, 0);
      // fb_len = jchunk.len;
    }

    if (xQueueFrameO) {
      xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
    } else if (gReturnFB) {
      esp_camera_fb_return(frame);
    } else {
      free(frame);
    }
  } else {
    ESP_LOGE(TAG, "Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  return res;
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *frame = NULL;
  struct timeval _timestamp;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[128];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  while (true) {
    if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {
      _timestamp.tv_sec = frame->timestamp.tv_sec;
      _timestamp.tv_usec = frame->timestamp.tv_usec;

      if (frame->format == PIXFORMAT_JPEG) {
        _jpg_buf = frame->buf;
        _jpg_buf_len = frame->len;
      } else if (!frame2jpg(frame, 80, &_jpg_buf, &_jpg_buf_len)) {
        ESP_LOGE(TAG, "JPEG compression failed");
        res = ESP_FAIL;
      }
    } else {
      res = ESP_FAIL;
    }

    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY,
                                  strlen(_STREAM_BOUNDARY));
    }

    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len,
                             _timestamp.tv_sec, _timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }

    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }

    if (frame->format != PIXFORMAT_JPEG) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }

    if (xQueueFrameO) {
      xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
    } else if (gReturnFB) {
      esp_camera_fb_return(frame);
    } else {
      free(frame);
    }

    if (res != ESP_OK) {
      break;
    }
  }

  return res;
}

void register_httpd(const QueueHandle_t frame_i, const QueueHandle_t frame_o,
                    const bool return_fb) {
  xQueueFrameI = frame_i;
  xQueueFrameO = frame_o;
  gReturnFB = return_fb;

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 12;

  httpd_uri_t capture_uri = {.uri = "/capture",
                             .method = HTTP_GET,
                             .handler = capture_handler,
                             .user_ctx = NULL};

  httpd_uri_t mjpg_uri = {.uri = "/mjpg",
                            .method = HTTP_GET,
                            .handler = stream_handler,
                            .user_ctx = NULL};

  config.server_port = 9000;
  config.ctrl_port = 9000;
  ESP_LOGI(TAG, "Starting mjpg server on port: '%d'", config.server_port);
  if (httpd_start(&mjpg_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(mjpg_httpd, &mjpg_uri);
    httpd_register_uri_handler(mjpg_httpd, &capture_uri);
  }
  
}
