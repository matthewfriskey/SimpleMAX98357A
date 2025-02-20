#include <Arduino.h>
#include <SD_MMC.h>
#include "SimpleMAX98357A.h"

SimpleMAX98357A player;
const int reedSwitchPin = 4;

void setup() {
    Serial.begin(115200);
    pinMode(reedSwitchPin, INPUT_PULLUP);

    // SD_MMC setup
    SD_MMC.setPins(13, 12, 21, 10, 11);  // CLK, CMD, DATA1, DATA2, DATA3
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD_MMC Mount Failed");
        return;
    }

    if (!player.begin(SD_MMC, 27, 26, 25)) {  // BCLK, LRCLK, DIN
        Serial.println("Player init failed");
        return;
    }

    player.setVolume(50);
}

void loop() {
    player.loop();

    if (digitalRead(reedSwitchPin) == LOW) {
        player.play(3);  // Plays /04.mp3 or /04.wav
        delay(200);
    }
}