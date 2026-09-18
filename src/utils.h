
#ifdef TRAM_DEBUG
#define LOG_PRINT(...) Serial.print(__VA_ARGS__);
#define LOG_PRINTLN(...) Serial.println(__VA_ARGS__);
#define LOG_PRINTF(...) Serial.printf(__VA_ARGS__);
#else
#define LOG_PRINT(...)
#define LOG_PRINTLN(...)
#define LOG_PRINTF(...)
#endif