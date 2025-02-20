#include <Arduino.h>
#include <SD.h>
#include "SimpleMAX98357A.h"

SimpleMAX98357A player;
const int reedSwitchPin = 4;

void setup() {
    Serial.begin(115200);
    pinMode(reedSwitchPin, INPUT_PULLUP);

    SPI.begin(18, 19, 23, 5);  // SCK, MISO, MOSI, CS
    if (!SD.begin(5)) {
        Serial.println("SD SPI Mount Failed");
        return;
    }

    if (!player.begin(SD, 27, 26, 25)) {
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
}