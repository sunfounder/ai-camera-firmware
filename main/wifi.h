
#define MODE_STA 0
#define MODE_AP  1

class WiFi{
    public:
        WiFi();
        void begin(uint8_t mode, const char* ssid, const char* password);
    private:
        uint8_t _mode;
        char* _ssid;
        char* _password;
        void _sta_init(const char* ssid, const char* password);
        void _ap_init(const char* ssid, const char* password);
};