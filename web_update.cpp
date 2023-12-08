#include "web_update.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>

web_update::web_update(bool debugger, uint16_t read_buffer, uint8_t timeout_seconds, bool https)
{
    if (buffer < 64)
        buffer = 64;

    if (timeout_seconds < 1)
        timeout_seconds = 1;

    this->buffer = read_buffer;
    this->debug = debugger;
    this->time_out = timeout_seconds;
    this->Https = https;
}

void web_update::host(char *host)
{
    this->HostC = host;
}

void web_update::host(IPAddress host)
{
    this->ip = host;
}

void web_update::hostPort(uint16_t port)
{
    this->hostPort_ = port;
}

void web_update::directory(char *Dir)
{
    this->dirC = Dir;
}

void web_update::debugger(bool debugger)
{
    this->debug = debugger;
}

void web_update::https(bool https)
{
    this->Https = https;
}

void web_update::buffer_size(uint16_t Buffer)
{
    if (buffer < 64)
        buffer = 64;

    this->buffer = Buffer;
}

void web_update::timeout(uint8_t timeout)
{
    if (timeout < 1)
        timeout = 1;
    this->time_out = timeout;
}

void web_update::updateFirmware(uint8_t *data, size_t len)
{
    Update.write(data, len);
    currentLength += len;
    if (debug)
    {
        Serial.print(currentLength);
        Serial.print("/");
        Serial.println(totalLength);
    }
    if (currentLength != totalLength)
        return;
    Update.end(true);
    if (debug)
    {
        Serial.println("Update complete");
        delay(3000);
    }
    ESP.restart();
}

#ifndef UpdateOverEthernet
uint8_t web_update::update_wifi()
{
    vPortEnterCritical;
    if (buffer > esp_get_free_heap_size())
    {
        if (debug)
            Serial.println("Buffer size is bigger than free heap size");
        updatingFirmware = false;
        vPortExitCritical;
        return 4;
    }

    if (buffer == 1)
        buffer = esp_get_free_heap_size() / 3;

    char *host = new char[strlen(HostC) + strlen(dirC) + 20];
    if (Https)
        strcpy(host, "https://");
    else
        strcpy(host, "http://");

    strcat(host, HostC);
    strcat(host, dirC);

    if (WiFi.status() != WL_CONNECTED)
    {
        if (debug)
            Serial.println("WiFi not connected");
        vPortExitCritical;
        return 1;
    }
    if (debug)
    {
        Serial.print("Conecting to HOST: ");
        Serial.println(host);
    }
    wifi_client.begin(host);
    uint16_t response = wifi_client.GET();
    if (debug)
    {
        Serial.print("Response: ");
        Serial.println(response);
    }
    if (response != 200)
    {
        if (debug)
            Serial.println("Check host destination and internet conection.");
        wifi_client.end();
        vPortExitCritical;
        return 2;
    }
    totalLength = wifi_client.getSize();
    uint32_t len = totalLength;
    Update.begin(UPDATE_SIZE_UNKNOWN);
    if (debug)
    {
        Serial.print("Update size: ");
        Serial.println(totalLength);
    }
    uint8_t *buff = new uint8_t[buffer];
    WiFiClient *stream = wifi_client.getStreamPtr();
    if (debug)
        Serial.println("Starting Update");
    time_out *= 1000;
    while (wifi_client.connected() && (len > 0 || len == -1))
    {
        updatingFirmware = true;
        if (updateAsyncDelay.isExpired())
        {
            if (debug)
                Serial.println("Update timeout");
            Update.end();
            wifi_client.end();
            updatingFirmware = false;
            vPortExitCritical;
            return 3;
        }

        size_t size = stream->available();
        if (size)
        {
            uint16_t c = stream->readBytes(buff, ((size > buffer) ? buffer : size));
            updateFirmware(buff, c);
            if (len > 0)
                len -= c;
        }
        vTaskDelay(1);
    }
    return 0; // to dont show warning on vscode using platformio
}
#endif

bool web_update::isUpdating()
{
    return updatingFirmware;
}

#ifdef UpdateOverEthernet
uint8_t web_update::update_ethernet()
{
    vPortEnterCritical;
    if (buffer > esp_get_free_heap_size())
    {
        if (debug)
            Serial.println("Buffer size is bigger than free heap size");
        updatingFirmware = false;
        vPortExitCritical;
        return 4;
    }

    if (buffer == 1)
        buffer = esp_get_free_heap_size() / 3;

    updatingFirmware = true;
    if (!Ethernet.hardwareStatus)
    {
        if (debug)
            Serial.println("No ethernet hardware found!");
        updatingFirmware = false;
        vPortExitCritical;
        return 1;
    }
    if (debug)
    {
        Serial.print("Conecting to Host: ");
        if (ip[0] == 0)
            Serial.print(HostC);
        else
            Serial.print(ip);
    }

    uint32_t connectionSuccess;

    if (ip[0] == 0)
        connectionSuccess = ethernet_client.connect(HostC, hostPort_);
    else
        connectionSuccess = ethernet_client.connect(ip, hostPort_);

    if (connectionSuccess) // starts ethernet_client connection, checks for connection
    {

        if (debug)
        {
            Serial.print("Connected to Host, now getting firmware, on: ");
            if (ip[0] == 0)
                Serial.print(HostC);
            else
                Serial.print(ip);
            Serial.print(" ");
            Serial.println(dirC);
            Serial.println();
        }
        ethernet_client.print("GET ");
        ethernet_client.print(dirC);
        ethernet_client.println(" HTTP/1.1");
        ethernet_client.print("Host: ");
        if (ip[0] == 0)
            ethernet_client.println(HostC);
        else
            ethernet_client.println(ip);
        ethernet_client.println("Connection: close");
        ethernet_client.println();
    }
    else
    {
        if (debug)
            Serial.println("Check host destination and internet conection.");
        updatingFirmware = false;
        vPortExitCritical;
        return 2;
    }

    uint8_t *buff = new uint8_t[buffer];

    uint32_t bb = 0;
    uint32_t resp_header = 0; // get the response header out of the payload
    Update.begin(UPDATE_SIZE_UNKNOWN);

    time_out *= 1000;
    updateAsyncDelay.start(time_out, AsyncDelay::MILLIS);

    char lastFourChars[4] = {0, 1, 2, 3};
    bool headerEnded = false;
    while (ethernet_client.connected() && !ethernet_client.available())
        vTaskDelay(1); // waits for data
    while (ethernet_client.connected() || ethernet_client.available())
    {
        if (updateAsyncDelay.isExpired())
        {
            if (debug)
                Serial.println("Update timeout");
            Update.end();
            ethernet_client.stop();
            updatingFirmware = false;
            return 3;
        }

        uint32_t size = ethernet_client.available();
        if (!headerEnded)
        {
            char c = ethernet_client.read();
            if (debug)
                Serial.print(c); // get the response header out of the payload

            lastFourChars[0] = lastFourChars[1];
            lastFourChars[1] = lastFourChars[2];
            lastFourChars[2] = lastFourChars[3];
            lastFourChars[3] = c;

            if (lastFourChars[0] == '\r' && lastFourChars[1] == '\n' && lastFourChars[2] == '\r' && lastFourChars[3] == '\n')
            {
                headerEnded = true;
                if (debug)
                    Serial.println("----- Header End -----");
                continue;
            }

            if (!headerEnded)
                continue;
        }

        memset(buff, 0, buffer);
        uint16_t c = ethernet_client.readBytes(buff, ((size > buffer) ? buffer : size));
        Update.write(buff, c);
        bb += c;
        if (debug)
            Serial.println(bb);
        delay(10);
    }
    ethernet_client.stop(); // stop ethernet_client
    Update.end(true);
    ESP.restart();
    return 0;
}
#endif