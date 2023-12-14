# Esp32 Web Update

This library allows you to update the firmware of your ESP32 microcontroller over WiFi and Ethernet.

**Features:**

* **Simple and intuitive API:** easy to integrate into your existing ESP32 projects.
* **Broad connectivity:** supports updates over both WiFi and Ethernet networks.
* **Detailed feedback:** provides clear debug messages and error codes for troubleshooting.
* **Customization options:** configure buffer size, update timeout, and server details.
* **Secure updates:** supports HTTPS for secure communication with the update server (https only on wifi).

**Installation:**

1. **Download the library files:**
    * **Arduino IDE:** Download the ZIP file and install it through the "Sketch" > "Include Library" > "Add .ZIP Library..." menu.
    * **PlatformIO IDE:** Place the library folder within the "lib" directory of your project.
2. **Include the library header:**
    * `#include <web_update.h>`

**Usage:**

**1. Initialization:**

* Create a `web_update` object.
* Optionally, configure its settings using the available methods:
    * `host`: Update server hostname or IP address.
    * `hostPort`: Update server port (default: 80 for HTTP, 443 for HTTPS).
    * `directory`: Directory on the server containing the update file.
    * `debugger`: Enable/disable debug messages (default: false). 
    * `https`: Use HTTPS for secure updates (default: true) (https only on wifi).
    * `buffer_size`: Buffer size for reading update data (default: 16384).
    * `timeout`: Update timeout in seconds (default: 60).

**2. Update process:**

* Start the update for your chosen network:
    * `updater.update_wifi();` (WiFi)
    * `updater.update_ethernet();` (Ethernet)
* Use `updater.isUpdating()` to check if the update is still ongoing.

**3. Error handling:**

* Error codes are provided for troubleshooting:
    * 1: WiFi not connected / Ethernet hardware not found.
    * 2: Host not found.
    * 3: Update timeout.
    * 4: Buffer size too large (exceeds available memory).
    * 5: Update failed.

**Examples:**

* Examples demonstrating various usage scenarios will be added soon.

**Additional notes:**

* You can only use Arduino or PlatformIO to compile your firmware (if you program with one and try to update with the other, it will fail).
* With servers with high latency (>200ms), the update may fail due to garbage data being read from the server.
* Requires the following Arduino libraries:
    * HTTPClient
    * Update
    * AsyncDelay
    * (Ethernet if using `UpdateOverEthernet`)
* Refer to the source code comments for detailed information about the library.
* Contributions are welcome! Please submit bug reports and pull requests to improve the library.

**Special thanks to @kurimawxx00/webota-esp32: [https://github.com/kurimawxx00/webota-esp32](https://github.com/kurimawxx00/webota-esp32) for their contribution to the WiFi update functionality.**