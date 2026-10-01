#include <Arduino.h>
#include "esp_system.h"

constexpr int LED_PIN = 6;

bool ledState = false;
unsigned long heartbeat = 0;

const char* resetReasonName(esp_reset_reason_t reason)
{
    switch (reason)
    {
        case ESP_RST_POWERON:   return "Power-on reset";
        case ESP_RST_EXT:       return "External reset";
        case ESP_RST_SW:        return "Software reset";
        case ESP_RST_PANIC:     return "Exception / panic";
        case ESP_RST_INT_WDT:   return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:  return "Task watchdog";
        case ESP_RST_WDT:       return "Other watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep-sleep wake";
        case ESP_RST_BROWNOUT:  return "Brownout";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return "Unknown";
    }
}

void testPsram()
{
    Serial.println();
    Serial.println("----- PSRAM TEST -----");

    if (!psramFound())
    {
        Serial.println("PSRAM: NOT FOUND");
        return;
    }

    Serial.println("PSRAM: FOUND");

    Serial.printf(
        "PSRAM size:       %u bytes\n",
        ESP.getPsramSize()
    );

    Serial.printf(
        "Free PSRAM:       %u bytes\n",
        ESP.getFreePsram()
    );

    Serial.printf(
        "Largest PSRAM block: %u bytes\n",
        ESP.getMaxAllocPsram()
    );

    const size_t testSize = 1024 * 1024;

    Serial.println();
    Serial.println("Allocating 1 MiB PSRAM test buffer...");

    uint8_t* buffer = (uint8_t*)ps_malloc(testSize);

    if (buffer == nullptr)
    {
        Serial.println("PSRAM allocation: FAIL");
        return;
    }

    Serial.println("PSRAM allocation: PASS");

    Serial.println("Writing test pattern...");

    for (size_t i = 0; i < testSize; i++)
    {
        buffer[i] = (uint8_t)((i * 37 + 13) & 0xFF);
    }

    Serial.println("Verifying test pattern...");

    bool passed = true;

    for (size_t i = 0; i < testSize; i++)
    {
        uint8_t expected = (uint8_t)((i * 37 + 13) & 0xFF);

        if (buffer[i] != expected)
        {
            Serial.printf(
                "PSRAM ERROR at address offset 0x%X\n",
                (unsigned)i
            );

            Serial.printf(
                "Expected: 0x%02X   Read: 0x%02X\n",
                expected,
                buffer[i]
            );

            passed = false;
            break;
        }
    }

    if (passed)
    {
        Serial.println("1 MiB PSRAM write/read test: PASS");
    }
    else
    {
        Serial.println("1 MiB PSRAM write/read test: FAIL");
    }

    free(buffer);

    Serial.printf(
        "Free PSRAM after test: %u bytes\n",
        ESP.getFreePsram()
    );
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" BYU-I eBadge v4.1.1 Diagnostic #2");
    Serial.println(" ESP32-S3 / Flash / PSRAM / Stability");
    Serial.println("========================================");

    Serial.printf(
        "Chip model:        %s\n",
        ESP.getChipModel()
    );

    Serial.printf(
        "CPU frequency:     %u MHz\n",
        ESP.getCpuFreqMHz()
    );

    Serial.printf(
        "Flash size:        %u bytes\n",
        ESP.getFlashChipSize()
    );

    Serial.printf(
        "Flash speed:       %u Hz\n",
        ESP.getFlashChipSpeed()
    );

    Serial.printf(
        "Internal heap:     %u bytes free\n",
        ESP.getFreeHeap()
    );

    esp_reset_reason_t reason = esp_reset_reason();

    Serial.printf(
        "Last reset reason: %s (%d)\n",
                  resetReasonName(reason),
                  reason
    );

    testPsram();

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("Entering continuous stability test.");
    Serial.println("GPIO6 will toggle once per second.");
    Serial.println("----------------------------------------");
}

void loop()
{
    ledState = !ledState;

    digitalWrite(
        LED_PIN,
        ledState ? HIGH : LOW
    );

    Serial.printf(
        "Heartbeat %-6lu GPIO6=%s | Heap=%u | PSRAM=%u\n",
        heartbeat++,
        ledState ? "HIGH" : "LOW",
        ESP.getFreeHeap(),
                  ESP.getFreePsram()
    );

    delay(1000);
}
