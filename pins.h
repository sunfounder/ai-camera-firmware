
#if defined(TTGO_CAMERA)
#define CAMERA_PIN_PWDN 26
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 32
#define CAMERA_PIN_SIOD 13
#define CAMERA_PIN_SIOC 12
#define CAMERA_PIN_VSYNC 27
#define CAMERA_PIN_HREF 25
#define CAMERA_PIN_PCLK 19
#define CAMERA_PIN_Y2 5
#define CAMERA_PIN_Y3 14
#define CAMERA_PIN_Y4 4
#define CAMERA_PIN_Y5 15
#define CAMERA_PIN_Y6 18
#define CAMERA_PIN_Y7 23
#define CAMERA_PIN_Y8 36
#define CAMERA_PIN_Y9 39

#elif defined(ESP32_CAM)
#define CAMERA_PIN_PWDN  32
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK   0
#define CAMERA_PIN_SIOD  26
#define CAMERA_PIN_SIOC  27
#define CAMERA_PIN_VSYNC 25
#define CAMERA_PIN_HREF  23
#define CAMERA_PIN_PCLK  22
#define CAMERA_PIN_Y2     5
#define CAMERA_PIN_Y3    18
#define CAMERA_PIN_Y4    19
#define CAMERA_PIN_Y5    21
#define CAMERA_PIN_Y6    36
#define CAMERA_PIN_Y7    39
#define CAMERA_PIN_Y8    34
#define CAMERA_PIN_Y9    35
#define CAMERA_PIN_FLASH   4
#define CAMERA_PIN_LED   33

#endif

