#include "web_update.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>

web_update::web_update(bool debugger, bool https, uint16_t read_buffer, uint8_t timeout_seconds)
{
    buffer = read_buffer;
    debug = debugger;
    time_out = timeout_seconds;
    Https = https;
}

void web_update::host(char *host)
{
    HostC = host;
}

void web_update::directory(char *Dir)
{
    dirC = Dir;
}

void web_update::debugger(bool debugger)
{
    debug = debugger;
}

void web_update::https(bool https)
{
    Https = https;
}

void web_update::buffer_size(uint16_t Buffer)
{
    buffer = Buffer;
}

void web_update::timeout(uint8_t timeout)
{
    time_out = timeout;
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

int web_update::update_wifi()
{
    char host[strlen(HostC) + strlen(dirC) + 20];
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
    uint8_t buff[buffer] = {0};
    WiFiClient *stream = wifi_client.getStreamPtr();
    if (debug)
        Serial.println("Starting Update");
    delai = millis();
    time_out *= 1000;
    while (wifi_client.connected() && (len > 0 || len == -1))
    {
        updatingFirmware = true;
        unsigned long currentMillis = millis();
        if ((unsigned long)(currentMillis - delai) >= time_out)
        {
            if (debug)
                Serial.println("Update timeout");
            Update.end(false);
            wifi_client.end();
            delai = currentMillis;
            return 3;
        }
        size_t size = stream->available();
        if (size)
        {
            uint16_t c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            updateFirmware(buff, c);
            if (len > 0)
                len -= c;
        }
        vTaskDelay(1);
    }
    return 0; // to dont show warning on vscode using platformio
}

bool web_update::isUpdating()
{
    return updatingFirmware;
}

#ifdef UpdateOverEthernet
int web_update::update_ethernet()
{
    if (!Ethernet.hardwareStatus)
    {
        if (debug)
            Serial.println("No ethernet hardware found!");
        return 1;
    }
    if (debug)
    {
        Serial.print("Conecting to Host: ");
        Serial.println(HostC);
    }
    if (ethernet_client.connect(HostC, 80)) // starts ethernet_client connection, checks for connection
    {
        if (debug)
        {
            Serial.print("Connected to Host, now getting firmware, on: ");
            Serial.print(HostC);
            Serial.println(dirC);
        }
        ethernet_client.print("GET ");
        ethernet_client.print(dirC);
        ethernet_client.println(" HTTP/1.1");
        ethernet_client.print("Host: ");
        ethernet_client.println(HostC);
        ethernet_client.println("Connection: close");
        ethernet_client.println();
    }
    else
    {
        if (debug)
            Serial.println("Check host destination and internet conection.");
        return 2;
    }

    uint8_t buff[buffer] = {0};
    uint32_t bb = 0;
    uint32_t resp_header = 0; // get the response header out of the payload
    Update.begin(UPDATE_SIZE_UNKNOWN);
    delai = millis();
    time_out *= 1000;
    while (ethernet_client.connected() && !ethernet_client.available())
        vTaskDelay(1); // waits for data
    while (ethernet_client.connected() || ethernet_client.available())
    {
        unsigned long currentMillis = millis();
        if ((unsigned long)(currentMillis - delai) >= time_out)
        {
            if (debug)
                Serial.println("Update timeout");
            Update.end(false);
            delai = currentMillis;
            return 4;
        }

        size_t size = ethernet_client.available();
        if (size)
        {
            if (resp_header < 10)
            {
                String header = ethernet_client.readStringUntil('\n');
                if (header.indexOf("404 Not Found") > 0) // If the firmware was not found in the dir url
                {
                    if (debug)
                        Serial.println("Check dir destination.");
                    return 3;
                }
                if (debug)
                    Serial.println(header); // get the response header out of the payload
            }
            else
            {
                uint16_t c = ethernet_client.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                Update.write(buff, c);
                bb += c;
                if (debug)
                    Serial.println(bb);
            }
            resp_header += 1;
        }
        vTaskDelay(1);
    }
    ethernet_client.stop(); // stop ethernet_client
    Update.end(true);
    ESP.restart();
    return 0; // to dont show warning on vscode using platformio
}
#endif