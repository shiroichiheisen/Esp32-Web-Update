#ifndef web_update_h
#define web_update_h

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>

// #define UpdateOverEthernet //uncomment this line if you want to use ethernet instead of wifi

#ifdef UpdateOverEthernet
#include <Ethernet.h>
#endif

/*

Its very simple to use, just create a web_update object with the host, debug messages,
buffer size and timeout for the update.
Then call the update method.

When calling the update method, if any error occurs, the method will return the error codes:

1 - WiFi not connected
2 - Host not found
3 - Update Timeout (1 minute, customizable in the web_update object with seconds)

*/
class web_update
{
public:
    web_update(bool debugger = false, bool https = true, uint16_t read_buffer = 64, uint8_t timeout_seconds = 60);
    void host(char *host);
    void directory(char *Dir);
    void debugger(bool debugger);
    void https(bool https);
    void buffer_size(uint16_t Buffer);
    void timeout(uint8_t timeout);
    int update_wifi();
    bool isUpdating();

#ifdef UpdateOverEthernet
    int update_ethernet();
#endif

private:
    void updateFirmware(uint8_t *data, size_t len);

    uint32_t
        totalLength,
        currentLength = 0,
        time_out;

    uint16_t
        buffer;

    bool
        debug,
        Https,
        updatingFirmware = false;

    unsigned long
        delai;

    HTTPClient
        wifi_client;

    char
        *HostC,
        *dirC;

#ifdef UpdateOverEthernet
    EthernetClient
        ethernet_client;
#endif
};
#endif
