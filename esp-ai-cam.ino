#include "who_camera.h"
#include "camera_server.hpp"
#include "who_human_face_detection.hpp"
#include "pins.h"
#include "ws_server.h"

// for production, uncomment this line
#define DEBUG

#define CAMERA_VERTICAL_FLIP 1

WS_Server ws_server = WS_Server();

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;
bool is_camera_started = false;

void setup() {
  Serial.begin(115200);
  // Serial.setTimeout(SERIAL_TIMEOUT);

  #ifdef DEBUG
  Serial.println("[DEBUG] Start!");
  #endif
  Serial.println("\r\n[OK]");
}

void loop() {
  ws_server.loop();
  if (ws_server.is_connected()){
    camera_init();
  }
}

void camera_init(){
  if (is_camera_started) {
    return;
  }
  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    
  register_camera(
      PIXFORMAT_RGB565, FRAMESIZE_QVGA, 1, xQueueAIFrame, CAMERA_VERTICAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_human_face_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame);
  register_httpd(xQueueHttpFrame, NULL, true);
  is_camera_started = true;
}