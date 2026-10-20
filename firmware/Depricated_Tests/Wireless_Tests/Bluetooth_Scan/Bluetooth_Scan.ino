#include <BLEDevice.h> // Include the ESP32 BLE library
#include <BLEUtils.h>  // Utilities for BLE
#include <BLEScan.h>   // For BLE scanning
#include <BLEAdvertisedDevice.h> // For handling advertised devices

// Define the scan duration in seconds.
// The scanner will run for this long, then stop and print results.
const int scanDuration = 5; // Scan for 5 seconds

// Pointer to the BLEScan object.
BLEScan* pBLEScan;

// This class will handle the results of the BLE scan.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    // This function is called every time a new BLE device is found during the scan.
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Print the address of the discovered device.
        Serial.printf("Advertised Device found: %s \n", advertisedDevice.getAddress().toString().c_str());

        // Check if the device has a name and print it.
        if (advertisedDevice.haveName()) {
            Serial.printf("  Name: %s \n", advertisedDevice.getName().c_str());
        }

        // You can add more checks here for service UUIDs, RSSI, etc.
        // For example, to check for a specific service:
        // if (advertisedDevice.hasServiceUUID() && advertisedDevice.getServiceUUID().equals(BLEUUID("YOUR_SERVICE_UUID"))) {
        //     Serial.println("  Found device with specific service!");
        // }
    }
};

// The setup function runs once when the ESP32-S3 starts up or resets.
void setup() {
    // Initialize serial communication for debugging.
    Serial.begin(115200);
    Serial.println("Starting ESP32-S3 BLE Device Scan...");

    // Initialize the BLE library.
    // This must be called before any other BLE operations.
    BLEDevice::init(""); // You can give your device a name here, e.g., "ESP32S3_Scanner"

    // Get a pointer to the BLE scan object.
    pBLEScan = BLEDevice::getScan();

    // Set the callback function to handle discovered devices.
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

    // Set the scan parameters:
    // setActiveScan(true) means the scanner will send scan requests and wait for scan responses,
    // which provides more information (like device name). If false, it's a passive scan.
    pBLEScan->setActiveScan(true);

    // Set the scan interval and window. These are advanced parameters for fine-tuning scan performance.
    // Interval: How often the scanner looks for advertisements (e.g., 100ms).
    // Window: How long the scanner actually scans during each interval (e.g., 90ms).
    // The window must be less than or equal to the interval.
    pBLEScan->setInterval(100); // Scan interval in milliseconds
    pBLEScan->setWindow(99);    // Scan window in milliseconds (must be <= interval)
}

// The loop function runs repeatedly after the setup function completes.
void loop() {
    Serial.print("Starting BLE scan for ");
    Serial.print(scanDuration);
    Serial.println(" seconds...");

    // Start the BLE scan.
    // The scan will run for 'scanDuration' seconds.
    // The 'false' parameter means it will not stop after the first device is found.
    // Fix for compilation error: Dereference the pointer returned by pBLEScan->start()
    BLEScanResults foundDevices = *pBLEScan->start(scanDuration, false);

    Serial.printf("Scan complete! Found %d devices.\n", foundDevices.getCount());

    // Clear the results to prepare for the next scan.
    pBLEScan->clearResults();

    // Wait for a few seconds before starting the next scan.
    // This prevents continuous scanning and allows you to read the output.
    delay(5000); // Wait 5 seconds before the next scan
}
