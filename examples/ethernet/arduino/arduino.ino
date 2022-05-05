#include <SPI.h>
#include <Ethernet.h>
#include <Update.h>
#include "web_update.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Ethernet Mac

String HOST = "www.host.com";
String Dir = "/firmware.bin";

#define w5x00_reset_pin 38
#define w5x00_ss_pin 34

web_update webUpdate(HOST, Dir, 1);

void setup()
{
    Serial.begin(115200);

    pinMode(w5x00_reset_pin, OUTPUT);

    Ethernet.init(w5x00_ss_pin); //Pin for the selection of the ethernet hardware via SPI

    digitalWrite(w5x00_reset_pin, LOW);  // This part is necessary for resetting whatever thing the ethernet hardware is doing
    delay(10);                           // Minimum delay for reset on low is 2us, you can use 1ms if you want less time, Don't erase this line! or the ethernet hardware will not fully reset!
    digitalWrite(w5x00_reset_pin, HIGH); // Enabling the ethernet hardware
    delay(300);                          // Minimum delay for the power on after reset is 150ms, if you want to use less time, I strongly recommend 200ms máx

    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        while (1)
            ;
    }

    webUpdate.update_ethernet();
}
void loop()
{
}