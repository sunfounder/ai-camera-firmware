
#include "log.h"

void debug(String msg) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG)
  Serial.print(F("[CAM_D] "));
  Serial.println(msg);
#endif
}

void info(String msg) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO)
  Serial.print(F("[CAM_I] "));
  Serial.println(msg);
#endif
}

void error(String msg) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR)
  Serial.print(F("[CAM_E] "));
  Serial.println(msg);
#endif
}

void debug(String msg, String data) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG)
  Serial.print(F("[CAM_D] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}

void info(String msg, String data) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO)
  Serial.print(F("[CAM_I] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}

void error(String msg, String data) {
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR)
  Serial.print(F("[CAM_E] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}
