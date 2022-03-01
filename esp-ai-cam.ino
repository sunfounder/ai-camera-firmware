
#include "app_main.hpp"
#include "dl_detect_define.hpp"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "components/modules/ai/who_cat_face_detection.hpp"
#include "components/modules/ai/who_human_face_detection.hpp"
#include "components/modules/ai/who_motion_detection.hpp"
// #include "components/modules/camera/who_camera.h"
// #include "components/modules/web/app_httpd.hpp"
// #include "components/modules/web/app_mdns.h"
// #include "components/modules/web/app_wifi.h"
// #include "who_cat_face_detection.hpp"
// #include "who_human_face_detection.hpp"
// #include "who_motion_detection.hpp"
#include "who_camera.h"
#include "app_httpd.hpp"
#include "app_mdns.h"
#include "app_wifi.h"

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;
static QueueHandle_t xQueueResult = NULL;

uint8_t i2c_addresses[2] = {0x16, 0x17};
int ai_mode = AI_MODE_HUMAN_FACE_DETECTION;
nvs_handle_t storage_handle;

static esp_err_t i2c_slave_init(uint8_t address);
void i2c_read_handler();
void storage_init();
uint8_t storage_read_u8(const char* key, uint8_t default_value);
void storage_write_u8(const char* key, uint8_t value);
void ai_init(int mode);
void ai_handler();

void setup() {
  storage_init();
  ai_mode = storage_read_u8("AI_MODE", -1);
  ai_init(ai_mode);

  ESP_ERROR_CHECK(i2c_slave_init(i2c_addresses[0]));
}

void loop() {
  i2c_read_handler();
  ai_handler();
  delay(100);
}

static esp_err_t i2c_slave_init(uint8_t address) {
  int i2c_slave_port = I2C_SLAVE_NUM;

  i2c_config_t conf_slave = {
      .mode = I2C_MODE_SLAVE,
      .sda_io_num = I2C_SLAVE_SDA_IO,
      .scl_io_num = I2C_SLAVE_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .slave = {.addr_10bit_en = 0, .slave_addr = address},
      .clk_flags = 0,
  };

  esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
  if (err != ESP_OK) {
    return err;
  }
  return i2c_driver_install(i2c_slave_port, conf_slave.mode,
                            I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

void i2c_read_handler() {
  uint8_t command;
  uint8_t data;

  size_t size;
  size = i2c_slave_read_buffer(I2C_SLAVE_NUM, &data, 1, 10 / portTICK_RATE_MS);
  if (size == 1) {
    ESP_LOGI("I2C", "Received: %d", data);
    command = data >> 7 & 1;
    if (command == AI_MODE_CONFIG) {
      uint8_t mode = data & 0x0F;
      ESP_LOGI("I2C", "AI_MODE_CONFIG: %d", mode);
      storage_write_u8("AI_MODE", mode);
      esp_restart();
    }
  }
}

void storage_init() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

uint8_t storage_read_u8(const char* key, uint8_t result) {
  esp_err_t err = nvs_open(STORAGE_NAMESPADE, NVS_READWRITE, &storage_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    err = nvs_get_u8(storage_handle, key, &result);
    switch (err) {
      case ESP_OK:
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGW(TAG, "Value '%s' is not initialized yet!\n", key);
        break;
      default:
        ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  nvs_close(storage_handle);
  return result;
}

void storage_write_u8(const char* key, uint8_t value) {
  esp_err_t err = nvs_open(STORAGE_NAMESPADE, NVS_READWRITE, &storage_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    err = nvs_set_u8(storage_handle, key, value);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error (%s) writing!\n", esp_err_to_name(err));
    }
  }
  nvs_commit(storage_handle);
  nvs_close(storage_handle);
}

void ai_init(int mode) {
  app_wifi_main();
  app_mdns_main();

  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t*));
  xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t*));
  xQueueResult = xQueueCreate(2, sizeof(uint8_t) * 14);

  register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame, VFLIP);
  switch (mode) {
    case AI_MODE_CAT_FACE_DETECTION:
      ESP_LOGI(TAG, "AI mode: cat face detection");
      register_cat_face_detection(xQueueAIFrame, NULL, xQueueResult,
                                  xQueueHttpFrame, true);
      break;
    case AI_MODE_CODE_RECOGNITION:
      ESP_LOGI(TAG, "AI mode: code scanner");
      // register_code_recognition(xQueueAIFrame, NULL, NULL, xQueueHttpFrame,
      // true);
      ESP_LOGW(TAG, "Code recognition is not implemented yet");
      break;
    case AI_MODE_COLOR_DETECTION:
      ESP_LOGI(TAG, "AI mode: color detection");
      ESP_LOGW(TAG, "Color detection is not implemented yet");
      // register_color_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame,
      // true);
      break;
    case AI_MODE_HUMAN_FACE_DETECTION:
      ESP_LOGI(TAG, "AI mode: human face detection");
      register_human_face_detection(xQueueAIFrame, NULL, xQueueResult,
                                    xQueueHttpFrame, true);
      break;
    case AI_MODE_HUMAN_FACE_RECOGITION:
      ESP_LOGI(TAG, "AI mode: human face recognition");
      ESP_LOGW(TAG, "Human face recognition is not implemented yet");
      // register_human_face_recognition(xQueueAIFrame, NULL, NULL,
      // xQueueHttpFrame, true);
      break;
    case AI_MODE_MOTION_DETECTION:
      ESP_LOGI(TAG, "AI mode: motion detection");
      register_motion_detection(xQueueAIFrame, NULL, xQueueResult,
                                xQueueHttpFrame);
      break;
    default:
      ESP_LOGW(TAG, "AI mode is not implemented yet: %d", mode);
      break;
  }
  register_httpd(xQueueHttpFrame, NULL, true);
}

void human_face_detect_handler() {
  uint8_t result[14];
  if (xQueueReceive(xQueueResult, &result, portMAX_DELAY)) {
    ESP_LOGI("ai_handler", "Box: (%3d, %3d, %3d, %3d)", result[0], result[1],
             result[2], result[3]);
    ESP_LOGI("ai_handler",
             "    left eye: (%3d, %3d), right eye: (%3d, %3d), nose: (%3d, "
             "%3d), mouth left: (%3d, %3d), mouth right: (%3d, %3d)",
             result[4], result[5],     // left eye
             result[6], result[7],     // right eye
             result[8], result[9],     // nose
             result[10], result[11],   // mouth left corner
             result[12], result[13]);  // mouth right corner
    esp_err_t err = i2c_slave_write_buffer(I2C_SLAVE_NUM, (uint8_t*)result, 14,
                                           1000 / portTICK_RATE_MS);
    if (err == ESP_FAIL) {
      ESP_LOGE(TAG, "Error (%s) writing to I2C slave!\n", esp_err_to_name(err));
    }
  }
}

void cat_face_detect_handler() {}
void motion_detection_handler() {}
void code_recognition_handler() {}
void color_detection_handler() {}
void human_face_recognition_handler() {}

void ai_handler() {
  switch (ai_mode) {
    case AI_MODE_CAT_FACE_DETECTION:
      cat_face_detect_handler();
      break;
    case AI_MODE_CODE_RECOGNITION:
      code_recognition_handler();
      break;
    case AI_MODE_COLOR_DETECTION:
      color_detection_handler();
      break;
    case AI_MODE_HUMAN_FACE_DETECTION:
      human_face_detect_handler();
      break;
    case AI_MODE_HUMAN_FACE_RECOGITION:
      human_face_recognition_handler();
      break;
    case AI_MODE_MOTION_DETECTION:
      motion_detection_handler();
      break;
    default:
      ESP_LOGW(TAG, "AI mode is not implemented yet: %d", ai_mode);
      break;
  }
}