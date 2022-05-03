#ifndef web_update_h
#define web_update_h

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>

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
    web_update(String Host, int debuger = 0, int read_buffer = 124, int timeout_seconds = 60);
    int update();

private:
    HTTPClient client;
    String Host;
    int buffer;
};
#endif
