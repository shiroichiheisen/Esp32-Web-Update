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

web_update::web_update(String host, int debugger, int read_buffer, int timeout_seconds)
{
    Host = host;
    buffer = read_buffer;
    debug = debugger;
    time_out = timeout_seconds;
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

int web_update::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (debug)
            Serial.println("WiFi not connected");
        return 1;
    }
    if (debug)
        Serial.println("Conecting to HOST.");
    client.begin(Host);
    int response = client.GET();
    if (debug)
        Serial.println("Response: " + String(response));
    if (response != 200)
    {
        if (debug)
            Serial.println("Check host destination and internet conection.");
        client.end();
        return 2;
    }
    totalLength = client.getSize();
    int len = totalLength;
    Update.begin(UPDATE_SIZE_UNKNOWN);
    if (debug)
        Serial.println("Update size: " + String(totalLength));
    uint8_t buff[buffer] = {0};
    WiFiClient *stream = client.getStreamPtr();
    if (debug)
        Serial.println("Starting Update");
    delai = millis();
    time_out *= 1000;
    while (client.connected() && (len > 0 || len == -1))
    {
        unsigned long currentMillis = millis();
        if ((unsigned long)(currentMillis - delai) >= time_out)
        {
            if (debug)
                Serial.println("Update timeout");
            Update.end(false);
            client.end();
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