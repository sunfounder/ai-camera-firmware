#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "app_wifi.h"
#include "app_httpd.hpp"
#include "app_mdns.h"

#include <list>
#include "dl_detect_define.hpp"
#include "who_camera.h"
#include "who_cat_face_detection.hpp"
#include "who_motion_detection.hpp"
#include "who_human_face_detection.hpp"
#include "esp_code_scanner.h"

#include "app_main.hpp"
#include "wifi.h"
// #include "ws_server.h"

// static QueueHandle_t xQueueAIFrame = NULL;
// static QueueHandle_t xQueueHttpFrame = NULL;
// static QueueHandle_t xQueueResult = NULL;
// static QueueHandle_t uart_queue;

// int ai_mode = AI_MODE_HUMAN_FACE_DETECTION;
// nvs_handle_t storage_handle;

WiFi wifi = WiFi();
// WebSocketServer ws = WebSocketServer(9000);

// void delay(uint32_t ms);
// void uart_init();
// void storage_init();
// uint8_t storage_read_u8(const char* key, uint8_t default_value);
// void storage_write_u8(const char* key, uint8_t value);
// void ai_init(int mode);
// void ai_handler();

extern "C" void app_main() {
    // storage_init();
    // ai_mode = storage_read_u8("AI_MODE", -1);
    // ai_init(ai_mode);

    // ESP_ERROR_CHECK(i2c_slave_init(i2c_addresses[0]));

    // while (1){
    //     ai_handler();
    //     delay(100);
    // }

    // wifi.begin(MODE_STA, "MakerStarsHall", "sunfounder");
    wifi.begin(MODE_AP, "ESP_AI_CAM", "12345678");
    // ws.start();
}

// void delay(uint32_t ms) {
//     vTaskDelay(ms / portTICK_PERIOD_MS);
// }

// void uart_init() {
//     const uart_port_t uart_num = UART_NUM;
//     uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
//         .rx_flow_ctrl_thresh = 122,
//     };
//     // Configure UART parameters
//     ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

//     ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

//     // Setup UART buffered IO with event queue
//     const int uart_buffer_size = (1024 * 2);
//     ESP_ERROR_CHECK(uart_driver_install(UART_NUM, uart_buffer_size,
//                                             uart_buffer_size, 10, &uart_queue, 0));
// }

// void uart_write(char* data) {
//     uart_write_bytes(uart_num, (const char*)data, strlen(data));
// }

// char* uart_read() {
//     uint8_t data[UART_BUFFER_LENGTH];
//     int length = 0;
//     ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t*)&length));
//     length = uart_read_bytes(uart_num, data, length, 100);
//     return (char*)data;
// }

// void uart_read_handler() {
//     char* data = uart_read();
// }

// void storage_init() {
//     esp_err_t err = nvs_flash_init();
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK( err );
// }

// uint8_t storage_read_u8(const char* key, uint8_t result) {
//     esp_err_t err = nvs_open(STORAGE_NAMESPADE, NVS_READWRITE, &storage_handle);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//     } else {
//         err = nvs_get_u8(storage_handle, key, &result);
//         switch (err) {
//             case ESP_OK:
//                 break;
//             case ESP_ERR_NVS_NOT_FOUND:
//                 ESP_LOGW(TAG, "Value '%s' is not initialized yet!\n", key);
//                 break;
//             default :
//                 ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
//         }
//     }
//     nvs_close(storage_handle);
//     return result;
// }

// void storage_write_u8(const char* key, uint8_t value) {
//     esp_err_t err = nvs_open(STORAGE_NAMESPADE, NVS_READWRITE, &storage_handle);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//     } else {
//         err = nvs_set_u8(storage_handle, key, value);
//         if (err != ESP_OK) {
//             ESP_LOGE(TAG, "Error (%s) writing!\n", esp_err_to_name(err));
//         }
//     }
//     nvs_commit(storage_handle);
//     nvs_close(storage_handle);
// }

// void ai_init(int mode) {
//     app_wifi_main();
//     app_mdns_main();

//     xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
//     xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
//     xQueueResult = xQueueCreate(2, sizeof(uint8_t) * 14);

//     register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame, VFLIP);
//     switch(mode) {
//         case AI_MODE_CAT_FACE_DETECTION:
//             ESP_LOGI(TAG, "AI mode: cat face detection");
//             register_cat_face_detection(xQueueAIFrame, NULL, xQueueResult, xQueueHttpFrame, true);
//             break;
//         case AI_MODE_CODE_RECOGNITION:
//             ESP_LOGI(TAG, "AI mode: code scanner");
//             // register_code_recognition(xQueueAIFrame, NULL, NULL, xQueueHttpFrame, true);
//             ESP_LOGW(TAG, "Code recognition is not implemented yet");
//             break;
//         case AI_MODE_COLOR_DETECTION:
//             ESP_LOGI(TAG, "AI mode: color detection");
//             ESP_LOGW(TAG, "Color detection is not implemented yet");
//             // register_color_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame, true);
//             break;
//         case AI_MODE_HUMAN_FACE_DETECTION:
//             ESP_LOGI(TAG, "AI mode: human face detection");
//             register_human_face_detection(xQueueAIFrame, NULL, xQueueResult, xQueueHttpFrame, true);
//             break;
//         case AI_MODE_HUMAN_FACE_RECOGITION:
//             ESP_LOGI(TAG, "AI mode: human face recognition");
//             ESP_LOGW(TAG, "Human face recognition is not implemented yet");
//             // register_human_face_recognition(xQueueAIFrame, NULL, NULL, xQueueHttpFrame, true);
//             break;
//         case AI_MODE_MOTION_DETECTION:
//             ESP_LOGI(TAG, "AI mode: motion detection");
//             register_motion_detection(xQueueAIFrame, NULL, xQueueResult, xQueueHttpFrame);
//             break;
//         default:
//             ESP_LOGW(TAG, "AI mode is not implemented yet: %d", mode);
//             break;
//     }
//     register_httpd(xQueueHttpFrame, NULL, true);
// }

// void human_face_detect_handler() {
//     uint8_t result[14];
//     if(xQueueReceive(xQueueResult, &result, portMAX_DELAY)){
//         ESP_LOGI("ai_handler", "Box: (%3d, %3d, %3d, %3d)", result[0], result[1], result[2], result[3]);
//         ESP_LOGI("ai_handler", "    left eye: (%3d, %3d), right eye: (%3d, %3d), nose: (%3d, %3d), mouth left: (%3d, %3d), mouth right: (%3d, %3d)",
//             result[4], result[5],  // left eye
//             result[6], result[7],  // right eye
//             result[8], result[9],  // nose
//             result[10], result[11],  // mouth left corner
//             result[12], result[13]); // mouth right corner
//     }
// }

// void cat_face_detect_handler(){}
// void motion_detection_handler(){}
// void code_recognition_handler(){}
// void color_detection_handler(){}
// void human_face_recognition_handler(){}

// void ai_handler() {
//     switch (ai_mode) {
//         case AI_MODE_CAT_FACE_DETECTION:
//             cat_face_detect_handler();
//             break;
//         case AI_MODE_CODE_RECOGNITION:
//             code_recognition_handler();
//             break;
//         case AI_MODE_COLOR_DETECTION:
//             color_detection_handler();
//             break;
//         case AI_MODE_HUMAN_FACE_DETECTION:
//             human_face_detect_handler();
//             break;
//         case AI_MODE_HUMAN_FACE_RECOGITION:
//             human_face_recognition_handler();
//             break;
//         case AI_MODE_MOTION_DETECTION:
//             motion_detection_handler();
//             break;
//         default:
//             ESP_LOGW(TAG, "AI mode is not implemented yet: %d", ai_mode);
//             break;
//     }
// }
