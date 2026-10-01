#include <Arduino.h>

// BYU-I eBadge v4.1.1 AUG26
// Discrete status LED
constexpr int LED_PIN = 6;

bool ledState = false;
unsigned long heartbeat = 0;

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);

    // Give the UART terminal a moment to reconnect after reset.
    delay(1000);

    Serial.println();
    Serial.println("====================================");
    Serial.println("BYU-I eBadge v4.1.1 diagnostic");
    Serial.println("ESP32-S3 application has booted!");
    Serial.println("====================================");
}

void loop()
{
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);

    Serial.print("Heartbeat ");
    Serial.print(heartbeat++);
    Serial.print("   GPIO6=");
    Serial.println(ledState ? "HIGH" : "LOW");

    delay(1000);
}
