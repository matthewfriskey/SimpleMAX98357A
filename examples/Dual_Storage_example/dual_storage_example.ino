#include <Arduino.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include "SimpleMAX98357A.h"

SimpleMAX98357A player;
const int reedSwitchPin = 4;
const int switchStoragePin = 15;  // Example pin to toggle storage

void setup() {
    Serial.begin(115200);
    pinMode(reedSwitchPin, INPUT_PULLUP);
    pinMode(switchStoragePin, INPUT_PULLUP);

    SD_MMC.setPins(13, 12, 21, 10, 11);
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD_MMC Mount Failed");
        return;
    }

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed");
        // Continue with SD_MMC
    }

    if (!player.begin(SD_MMC, 27, 26, 25)) {
        Serial.println("Player init failed");
        return;
    }

    player.setVolume(50);
}

void loop() {
    player.loop();

    if (digitalRead(reedSwitchPin) == LOW) {
        player.play(3);
        delay(200);
    }

    if (digitalRead(switchStoragePin) == LOW) {
        player.stop();
        player.begin(LittleFS, 27, 26, 25);
        Serial.println("Switched to LittleFS");
        delay(200);
    }
}