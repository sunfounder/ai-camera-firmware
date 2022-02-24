#include <esp_system.h>
#include <esp_http_server.h>

#define DEFAULT_PORT 80

class WebSocketServer {
    public:
        WebSocketServer(void);
        WebSocketServer(uint32_t port);
        void start();
        void stop();
        // void send(const char* data);
        // void onReceive(std::function<void(const char*)> callback);
    
    private:
        uint32_t _port;
        httpd_handle_t _server;
        struct _async_resp_arg {
            httpd_handle_t hd;
            int fd;
        };

        static const httpd_uri_t _ws;
};