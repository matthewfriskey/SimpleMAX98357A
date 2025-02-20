#ifndef SIMPLE_MAX98357A_H
#define SIMPLE_MAX98357A_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <mp3dec.h>

#define BUFFER_SIZE 512
#define MAX_TRACKS 99

class SimpleMAX98357A {
public:
    enum error_t { NO_ERROR, INIT_FAILED, FILE_ERROR, UNSUPPORTED_FORMAT, STORAGE_FAILED };

    SimpleMAX98357A();

    bool begin(FS &fs, int bclkPin, int lrclkPin, int dinPin);

    void setVolume(uint8_t volume);
    void volumeUp();
    void volumeDown();
    uint8_t getVolume();

    void play(const char* filename);
    void play(int index);
    void next();
    void previous();
    void loop(bool enable);
    void repeat(bool enable);
    void pause();
    void resume();
    void stop();

    bool isPlaying();
    bool isStorageConnected();
    error_t getLastError();

    void loop();

private:
    FS* _fs;
    File _file;
    bool _playing, _paused, _repeat, _loop;
    uint8_t _volume;
    int _currentIndex;
    error_t _lastError;
    uint8_t _buffer[BUFFER_SIZE];
    struct WavHeader { uint32_t sample_rate; uint16_t bits_per_sample; uint16_t num_channels; } _wavHeader;
    HMP3Decoder _mp3Decoder;
    uint8_t _mp3Buffer[MAINBUF_SIZE];
    int _mp3Offset;

    bool parseWavHeader();
    void scaleBuffer(uint8_t* buffer, size_t size);
    void initMP3();
    void processMP3();
    void processWAV();
};

#endif