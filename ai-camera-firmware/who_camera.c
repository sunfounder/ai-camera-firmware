#include "who_camera.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "who_camera";
static QueueHandle_t xQueueFrameO = NULL;

static void task_process_handler(void *arg) {
  while (true) {
    camera_fb_t *frame = esp_camera_fb_get();
    if (frame) {
      xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
      // xQueueOverwrite(xQueueFrameO, &frame);
    }
  }
}

void register_camera(const pixformat_t pixel_fromat,
                     const framesize_t frame_size, const uint8_t fb_count,
                     const QueueHandle_t frame_o, const int vflip, const int hflip,
                     const int d0, const int d1, const int d2, const int d3,
                     const int d4, const int d5, const int d6, const int d7,
                     const int xclk, const int pclk, const int vsync,
                     const int href, const int sda, const int scl,
                     const int pwdn, const int reset) {

#if CONFIG_CAMERA_MODULE_ESP_EYE || CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD
  /* IO13, IO14 is designed for JTAG by default,
   * to use it as generalized input,
   * firstly declair it as pullup input */
  gpio_config_t conf;
  conf.mode = GPIO_MODE_INPUT;
  conf.pull_up_en = GPIO_PULLUP_ENABLE;
  conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  conf.intr_type = GPIO_INTR_DISABLE;
  conf.pin_bit_mask = 1LL << 13;
  gpio_config(&conf);
  conf.pin_bit_mask = 1LL << 14;
  gpio_config(&conf);
#endif

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = d0;
  config.pin_d1 = d1;
  config.pin_d2 = d2;
  config.pin_d3 = d3;
  config.pin_d4 = d4;
  config.pin_d5 = d5;
  config.pin_d6 = d6;
  config.pin_d7 = d7;
  config.pin_xclk = xclk;
  config.pin_pclk = pclk;
  config.pin_vsync = vsync;
  config.pin_href = href;
  config.pin_sscb_sdapin_sscb_sda = sda;
  config.pin_sccb_scl = scl;
  config.pin_pwdn = pwdn;
  config.pin_reset = reset;

  config.xclk_freq_hz = XCLK_FREQ_HZ;
  config.pixel_format = pixel_fromat;
  config.frame_size = frame_size;
  config.jpeg_quality = JPEG_QUALITY;
  config.fb_count = fb_count;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, vflip);  // flip it back
  s->set_hmirror(s, hflip);
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_brightness(s, 1);   // up the blightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }

  xQueueFrameO = frame_o;
  xTaskCreatePinnedToCore(task_process_handler, TAG, 1 * 1024, NULL, 5, NULL,
                          1);
}
