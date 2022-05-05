# Esp32 Web Update

Library for updating the firmware of an Esp32 device over the web with wifi and ethernet.

## ☕ Using this library

To use this library, you need to download, install the library, and include the following file in your project:

```
#include <web_update.h>
```

After that, you declare the web_update object with the following customizations:

HOST - DNS of the server where the firmware is - Mandatory

Directory - The URL to the firmware file inside the server. - Mandatory

Debugger - If you want to know what's happening on the update via Serial, set this to 1. - Opcional (Default: 0)

HTTPS - If you're using SSL or not on your website, 0 to HTTP and 1 to HTTPS - Opcional (Default: 1)

The ethernet update part only works with http links (without ssl)!!!!!!!!

Buffer Size - The size of the buffer to store the firmware. - Optional (Default: 64)

Time Out - Timeout for the update process. - Optional (Default: 60)

```
web_update webUpdate(String HOST, String Directory, int Debbuger, int HTTPS, int BUFFER_SIZE, int TIME_OUT);

web_update webUpdate("www.google.com", "/firmware_file.bin", 1, 1, 128, 120);
```

When you want to start the update process, you call the following methods:

Update via WIFI:

```
webUpdate.update_wifi();
```

Update via Ethernet:

```
webUpdate.update_ethernet();
```

If any error occurs, the method will return the error code:

For WIFI:

1 - Wifi not connected.

2 - File or host not found.

3 - Update Timeout.

For Ethernet:

1 - Ethernet Hardware not found.

2 - Host not found.

3 - Dir directory with error 404 (file not found)

4 - Update Timeout.


I strongly recommend to use the reset pin on the ethernet hardware, they have so many problems! and with the reset pin you can reset if any error occurs!


## ❌ Some errors I have found and solutions:

If you create the firmware file (.bin) on Arduino and make the update firmware (.bin) on PlatformIO or vice versa, the update process will complete but when the esp32 tries to load the firmware will show that the firmware is corrupted! The firmware and the update firmware MUST BE ON THE SAME PLATFORM!

The ethernet update part when compiled on PlatformIO will not work on the Arduino 2.0 beta, meaning only work with the esp32, not with esp32-s2,esp32-c3,esp32-s3, and so on, I recommend using the ethernet part on Arduino ide or wait for the final update for the Arduino 2.0 framework for the PlatformIO to use other versions of the esp32, on the Arduino ide the ethernet part work 100% for the esp32 and other versions also!

Wifi update part based on [@kurimawxx00/webota-esp32](https://github.com/kurimawxx00/webota-esp32).