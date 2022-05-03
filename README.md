# Esp32 Web Update


## ☕ Using this library

To use this library, you need to include the following files in your project:

```
#include <web_update.h>
```

After that, you declare the web_update object with the folowing customizations:

HOST - The URL to the firmware file. - Mandatory
Debbuger - If you want to know what's happening on the update via Serial, set this to 1. - Opcional
Buffer Size - The size of the buffer to store the firmware. - Opcional
Time Out - Timeout for the update process. - Opcional

```
WebUpdate webUpdate(String HOST, int Debbuger, int BUFFER_SIZE, int TIME_OUT);
```

When you want to start the update process, you call the following method:

```
webUpdate.update();
```

If any error occurs, the method will return the error code:

-1 - Wifi not connected.
-2 - File or host not found.
-3 - Update Timeout.

Future updates will suport ethernet and other network types.

Library created from the archives of [@kurimawxx00/webota-esp32](https://github.com/kurimawxx00/webota-esp32).