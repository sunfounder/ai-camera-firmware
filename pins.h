
#if defined(ESP32_CAM)
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
#define CAMERA_PIN_FLASH  4
#define CAMERA_PIN_LED   33

#elif defined(ESP32_S3_CAM)
#define CAMERA_PIN_PWDN  -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK  15
#define CAMERA_PIN_SIOD   4
#define CAMERA_PIN_SIOC   5
#define CAMERA_PIN_VSYNC  6
#define CAMERA_PIN_HREF   7
#define CAMERA_PIN_PCLK  13
#define CAMERA_PIN_Y2    11
#define CAMERA_PIN_Y3     9
#define CAMERA_PIN_Y4     8
#define CAMERA_PIN_Y5    10
#define CAMERA_PIN_Y6    12
#define CAMERA_PIN_Y7    18
#define CAMERA_PIN_Y8    17
#define CAMERA_PIN_Y9    16

#define CAMERA_PIN_LED    2
#define CAMERA_PIN_FLASH  -1

#endif

