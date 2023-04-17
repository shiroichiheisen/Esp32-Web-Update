#include "web_update.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>

int
    totalLength,
    debug,
    time_out,
    currentLength = 0;

unsigned long
    delai;

web_update::web_update(String host, String directory, int debugger, int https, int read_buffer, int timeout_seconds)
{
    Host = host;
    buffer = read_buffer;
    debug = debugger;
    time_out = timeout_seconds;
    dir = directory;
    Https = https;
}

void web_update::host(String host)
{
    Host = host;
}
void web_update::directory(String Dir)
{
    dir = Dir;
}
void web_update::debugger(int debugger)
{
    debug = debugger;
}
void web_update::https(int https)
{
    Https = https;
}
void web_update::buffer_size(int Buffer)
{
    buffer = Buffer;
}
void web_update::timeout(int timeout)
{
    time_out = timeout;
}

void updateFirmware(uint8_t *data, size_t len)
{
    Update.write(data, len);
    currentLength += len;
    if (debug)
        Serial.println(String(currentLength) + "/" + String(totalLength));
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
    String host = Host + dir;
    if (Https)
        host = "https://" + host;
    else
        host = "http://" + host;

    if (WiFi.status() != WL_CONNECTED)
    {
        if (debug)
            Serial.println("WiFi not connected");
        return 1;
    }
    if (debug)
        Serial.println("Conecting to HOST: " + host);
    wifi_client.begin(host);
    int response = wifi_client.GET();
    if (debug)
        Serial.println("Response: " + String(response));
    if (response != 200)
    {
        if (debug)
            Serial.println("Check host destination and internet conection.");
        wifi_client.end();
        return 2;
    }
    totalLength = wifi_client.getSize();
    int len = totalLength;
    Update.begin(UPDATE_SIZE_UNKNOWN);
    if (debug)
        Serial.println("Update size: " + String(totalLength));
    uint8_t buff[buffer] = {0};
    WiFiClient *stream = wifi_client.getStreamPtr();
    if (debug)
        Serial.println("Starting Update");
    delai = millis();
    time_out *= 1000;
    while (wifi_client.connected() && (len > 0 || len == -1))
    {
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
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            updateFirmware(buff, c);
            if (len > 0)
                len -= c;
        }
    }
    return 0; // to dont show warning on vscode using platformio
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
    int str_len = Host.length() + 1;
    char serverName[str_len];
    Host.toCharArray(serverName, str_len);
    strcpy(serverName, Host.c_str());
    if (debug)
        Serial.println("Conecting to Host: " + Host);
    if (ethernet_client.connect(serverName, 80)) // starts ethernet_client connection, checks for connection
    {
        if (debug)
            Serial.println("Connected to Host, now getting firmware, on: " + Host + dir);
        ethernet_client.println("GET " + dir + " HTTP/1.1");
        ethernet_client.println("Host: " + Host);
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
    int bb = 0;
    int resp_header = 0; // get the response header out of the payload
    Update.begin(UPDATE_SIZE_UNKNOWN);
    delai = millis();
    time_out *= 1000;
    while (ethernet_client.connected() && !ethernet_client.available())
        delay(1); // waits for data
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
                int c = ethernet_client.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                Update.write(buff, c);
                bb += c;
                if (debug)
                    Serial.println(bb);
            }
            resp_header += 1;
        }
    }
    ethernet_client.stop(); // stop ethernet_client
    Update.end(true);
    ESP.restart();
    return 0; // to dont show warning on vscode using platformio
}
#endif