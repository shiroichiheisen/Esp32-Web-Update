#ifndef web_update_h
#define web_update_h

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <AsyncDelay.h>

// #define UpdateOverEthernet // uncomment this line if you want to use ethernet instead of wifi

#ifdef UpdateOverEthernet
#include <Ethernet.h>
#endif

class web_update
{
public:
    web_update(bool debugger = false, uint8_t timeout_seconds = 60, uint16_t read_buffer = 1, bool https = true);
    void host(char *host);
    void host(IPAddress host);
    void hostPort(uint16_t port);
    void directory(char *Dir);
    void debugger(bool debugger);
    void https(bool https);
    void buffer_size(uint16_t Buffer);
    void timeout(uint8_t timeout);
    uint8_t update_wifi();
    bool isUpdating();

#ifdef UpdateOverEthernet
    uint8_t update_ethernet();
#endif

private:
    uint32_t
        buffer,

        totalLength,
        currentLength = 0,
        time_out;

    uint16_t
        hostPort_;

    bool
        debug,
        Https,
        updatingFirmware = false;

    HTTPClient
        wifi_client;

    char
        *HostC,
        *dirC;

    IPAddress
        ip = IPAddress(0, 0, 0, 0);

    AsyncDelay
        updateAsyncDelay;

#ifdef UpdateOverEthernet
    EthernetClient
        ethernet_client;
#endif
};
#endif
