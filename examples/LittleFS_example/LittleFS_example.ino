#include <Arduino.h>
#include <LittleFS.h>
#include "SimpleMAX98357A.h"

SimpleMAX98357A player;
const int reedSwitchPin = 4;

void setup() {
    Serial.begin(115200);
    pinMode(reedSwitchPin, INPUT_PULLUP);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    if (!player.begin(LittleFS, 27, 26, 25)) {
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