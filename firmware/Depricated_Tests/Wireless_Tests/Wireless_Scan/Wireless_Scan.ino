#include <WiFi.h> // Include the Wi-Fi library for ESP32

void setup() {
  // Initialize serial communication for debugging and output
  Serial.begin(115200);
  Serial.println("\n"); // Add a newline for cleaner output

  Serial.println("ESP32-S3 Wi-Fi Network Scanner");
  Serial.println("Starting Wi-Fi scan...");

  // Set Wi-Fi to station mode (client mode)
  WiFi.mode(WIFI_STA);
  // Disconnect from any previously connected network
  WiFi.disconnect();
  delay(100); // Small delay to ensure disconnection
}

void loop() {
  Serial.println("------------------------------------");
  Serial.println("New Wi-Fi Scan Cycle:");

  // WiFi.scanNetworks will return the number of networks found
  // It returns -1 on error, 0 if no networks found, or a positive integer
  // representing the number of networks found.
  int n = WiFi.scanNetworks();

  // Print the number of networks found
  if (n == 0) {
    Serial.println("No Wi-Fi networks found.");
  } else {
    Serial.print(n);
    Serial.println(" Wi-Fi networks found:");
    // Iterate through all found networks and print their details
    for (int i = 0; i < n; ++i) {
      // Print network index
      Serial.print(i + 1);
      Serial.print(": ");
      // Print SSID (network name)
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI: ");
      // Print RSSI (Received Signal Strength Indicator)
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");
      // Print encryption type
      Serial.print(" - Enc: ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
          Serial.println("Open");
          break;
        case WIFI_AUTH_WEP:
          Serial.println("WEP");
          break;
        case WIFI_AUTH_WPA_PSK:
          Serial.println("WPA PSK");
          break;
        case WIFI_AUTH_WPA2_PSK:
          Serial.println("WPA2 PSK");
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          Serial.println("WPA/WPA2 PSK");
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.println("WPA2 Enterprise");
          break;
        case WIFI_AUTH_WPA3_PSK:
          Serial.println("WPA3 PSK");
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          Serial.println("WPA2/WPA3 PSK");
          break;
        default:
          Serial.println("Unknown");
          break;
      }
      delay(10); // Small delay to prevent watchdog timer issues during printing
    }
  }

  // Wait for 10 seconds before performing another scan
  Serial.println("Scan complete. Waiting 10 seconds for next scan...");
  delay(10000);
}

